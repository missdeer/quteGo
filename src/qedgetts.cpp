#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QtEndian>

#include "qedgetts.h"
#include "networkreplyhelper.h"

namespace
{
    // inspired by https://github.com/wujunwei928/edge-tts-go & https://github.com/rany2/edge-tts
#define TRUSTED_CLIENT_TOKEN "6A5AA1D4EAFF4E9FB37E23D68491D6F4"
#define WSS_URL "wss://speech.platform.bing.com/consumer/speech/synthesize/readaloud/edge/v1?TrustedClientToken=" TRUSTED_CLIENT_TOKEN
#define VOICE_LIST_URL "https://speech.platform.bing.com/consumer/speech/synthesize/readaloud/voices/list?trustedclienttoken=" TRUSTED_CLIENT_TOKEN

    QString connectID()
    {
        // generate a uuid
        QUuid uuid = QUuid::createUuid();
        return uuid.toString(QUuid::WithoutBraces);
    }

    QString dateToString()
    {
        // Get the current UTC time
        QDateTime utcTime = QDateTime::currentDateTimeUtc();

        // Format the date and time as specified, closely matching Go's formatting
        QString timeString = utcTime.toString("ddd MMM dd yyyy HH:mm:ss 'GMT'0000 '(Coordinated Universal Time)'");

        return timeString;
    }
    QString ssmlHeadersPlusData(const QString &requestID, const QString &timestamp, const QString &ssml)
    {
        return QStringLiteral("X-RequestId:%1\r\n"
                              "Content-Type:application/ssml+xml\r\n"
                              "X-Timestamp:%2Z\r\n"
                              "Path:ssml\r\n\r\n"
                              "%3")
            .arg(requestID, timestamp, ssml);
    }

    QMap<QString, QByteArray> getHeadersAndData(const QByteArray &data, QByteArray &remainingData)
    {
        int headerEnd = data.indexOf("\r\n\r\n");
        if (headerEnd == -1)
        {
            return {}; // Equivalent to returning an error in Go
        }

        QByteArray        headerPart = data.left(headerEnd);
        QList<QByteArray> lines      = headerPart.split('\r\n');

        QMap<QString, QByteArray> headers;
        for (const QByteArray &line : lines)
        {
            int colonIndex = line.indexOf(':');
            if (colonIndex == -1)
            {
                return {}; // Invalid header format
            }

            QString    key   = QString::fromUtf8(line.left(colonIndex).trimmed());
            QByteArray value = line.mid(colonIndex + 1).trimmed();
            headers[key]     = value;
        }

        remainingData = data.mid(headerEnd + 4); // Skip over the "\r\n\r\n"
        return headers;
    }

    QString generateSSML(const QString &text, const QString &voice, int rate, int volume, int pitch)
    {
        // Generate SSML content by QtXML
        QDomDocument doc;
        QDomElement  root = doc.createElement("speak");
        root.setAttribute("version", "1.0");
        root.setAttribute("xmlns", "http://www.w3.org/2001/10/synthesis");
        root.setAttribute("xml:lang", "en-US");
        auto voiceNode    = root.appendChild(doc.createElement("voice"));
        auto voiceElement = voiceNode.toElement();
        voiceElement.setAttribute("name", voice);
        auto prosodyNode    = voiceElement.appendChild(doc.createElement("prosody"));
        auto prosodyElement = prosodyNode.toElement();
        prosodyElement.setAttribute("pitch", (pitch >= 0 ? "+" : "") + QString::number(pitch) + "Hz");
        prosodyElement.setAttribute("rate", (rate >= 0 ? "+" : "") + QString::number(rate) + "%");
        prosodyElement.setAttribute("volume", (volume >= 0 ? "+" : "") + QString::number(volume) + "%");
        prosodyElement.appendChild(doc.createTextNode(text));
        doc.appendChild(root);
        return doc.toString();
    }

    QString generateCommandRequestContent()
    {
        QString content;
        content += "X-Timestamp:" + dateToString() + "\r\n";
        content += "Content-Type:application/json; charset=utf-8\r\n";
        content += "Path:speech.config\r\n\r\n";
        content += R"({"context":{"synthesis":{"audio":{"metadataoptions":{)";
        content += R"("sentenceBoundaryEnabled":false,"wordBoundaryEnabled":true},)";
        content += R"("outputFormat":"audio-24khz-48kbitrate-mono-mp3"}}}})";
        content += "\r\n";

        return content;
    }

    QString generateSSMLRequestContent(const QString &text, const QString &voice, int rate, int volume, int pitch)
    {
        return ssmlHeadersPlusData(connectID(), dateToString(), generateSSML(text, voice, rate, volume, pitch));
    }
} // namespace

QEdgeTTS::QEdgeTTS(QObject *parent) : QObject {parent}
{
    connect(&m_webSocket, &QWebSocket::connected, this, &QEdgeTTS::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &QEdgeTTS::onDisconnected);
    connect(&m_webSocket, &QWebSocket::textMessageReceived, this, &QEdgeTTS::onTextMessageReceived);
    connect(&m_webSocket, &QWebSocket::binaryMessageReceived, this, &QEdgeTTS::onBinaryMessageReceived);
    connect(&m_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &QEdgeTTS::onError);
}

void QEdgeTTS::getVoiceList()
{
    QNetworkRequest request(QUrl(VOICE_LIST_URL));
    request.setRawHeader("Authority", "speech.platform.bing.com");
    request.setRawHeader("Sec-CH-UA",
                         "\" Not;A Brand\";v=\"99\", \"Microsoft "
                         "Edge\";v=\"91\", \"Chromium\";v=\"91\"");
    request.setRawHeader("Sec-CH-UA-Mobile", "?0");
    request.setRawHeader("User-Agent",
                         "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, "
                         "like Gecko) Chrome/91.0.4472.77 Safari/537.36 Edg/91.0.864.41");
    request.setRawHeader("Accept", "*/*");
    request.setRawHeader("Sec-Fetch-Site", "none");
    request.setRawHeader("Sec-Fetch-Mode", "cors");
    request.setRawHeader("Sec-Fetch-Dest", "empty");
    request.setRawHeader("Accept-Encoding", "gzip, deflate, br");
    request.setRawHeader("Accept-Language", "en-US,en;q=0.9");
    auto *reply = new NetworkReplyHelper(m_nam.get(request));
    connect(reply, &NetworkReplyHelper::done, this, &QEdgeTTS::onVoiceListReceived);
}

void QEdgeTTS::onVoiceListReceived()
{
    auto *reply = qobject_cast<NetworkReplyHelper *>(sender());
    if (!reply)
    {
        emit errorOccurred("Invalid reply object");
        return;
    }
    reply->deleteLater();

    auto data = reply->content();

    QJsonDocument        doc    = QJsonDocument::fromJson(data);
    auto                 voices = doc.array();
    QList<VoiceIdentity> voiceList;
    for (auto voice : voices)
    {
        auto          voiceObj = voice.toObject();
        VoiceIdentity vi;
        vi.name                = voiceObj["Name"].toString();
        vi.shortName           = voiceObj["ShortName"].toString();
        vi.locale              = voiceObj["Locale"].toString();
        vi.gender              = voiceObj["Gender"].toString();
        vi.friendlyName        = voiceObj["FriendlyName"].toString();
        vi.status              = voiceObj["Status"].toString();
        vi.suggestedCodec      = voiceObj["SuggestedCodec"].toString();
        auto        categories = voiceObj["VoiceTag"].toObject()["ContentCategories"].toArray();
        QStringList categoriesList;
        for (auto category : categories)
        {
            categoriesList << category.toString();
        }
        vi.contentCategory        = categoriesList.join(",");
        auto        personalities = voiceObj["VoiceTag"].toObject()["VoicePersonalities"].toArray();
        QStringList personalitiesList;
        for (auto personality : personalities)
        {
            personalitiesList << personality.toString();
        }
        vi.voicePersonalities = personalitiesList.join(",");
        voiceList << vi;
    }

    emit voiceListReceived(voiceList);
}

void QEdgeTTS::getTextToSpeech(const QString &text, const QString &voiceName, int volume, int rate, int pitch)
{
    m_textToSpeech = text;
    m_voiceName    = voiceName;
    m_volume       = volume;
    m_rate         = rate;
    m_pitch        = pitch;
    m_audioData.clear();

    QString         wssUrl = WSS_URL "&ConnectionId=" + connectID();
    QNetworkRequest request(wssUrl);
    request.setRawHeader("Pragma", "no-cache");
    request.setRawHeader("Cache-Control", "no-cache");
    request.setRawHeader("Origin", "chrome-extension://jdiccldimpdaibmpdkjnbmckianbfold");
    request.setRawHeader("Accept-Encoding", "gzip, deflate, br");
    request.setRawHeader("Accept-Language", "en-US,en;q=0.9");
    request.setRawHeader(
        "User-Agent",
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.77 Safari/537.36 Edg/91.0.864.41");
    connectToServer(request);
}

void QEdgeTTS::connectToServer(QNetworkRequest &request)
{
    m_webSocket.open(request);
}

void QEdgeTTS::onConnected()
{
    qDebug() << "WebSocket connected";
    m_webSocket.sendTextMessage(generateCommandRequestContent());
    m_webSocket.sendTextMessage(generateSSMLRequestContent(m_textToSpeech, m_voiceName, m_rate, m_volume, m_pitch));
}

void QEdgeTTS::onDisconnected()
{
    qDebug() << "WebSocket disconnected";
    teardownConnection();
}

void QEdgeTTS::onTextMessageReceived(const QString &message)
{
    qDebug() << "Received text message: " << message;
    // Handle message
    QByteArray remainingData;
    auto       headers = getHeadersAndData(message.toUtf8(), remainingData);
    if (headers.empty())
    {
        emit errorOccurred("Invalid headers");
        return;
    }
    if (headers["Path"] == "turn.end")
    {
        // End of the turn
        m_webSocket.close();
        emit voiceReceived(m_audioData);
    }
}

void QEdgeTTS::onBinaryMessageReceived(const QByteArray &message)
{
    qDebug() << "Received binary message";
    // Handle binary data
    if (message.size() < 2)
    {
        emit errorOccurred("we received a binary message, but it is missing the header length");
        return;
    }
    QByteArray data         = message.left(2);
    auto       headerLength = qFromBigEndian<quint16>(reinterpret_cast<const uchar *>(message.constData()));
    if (message.size() < headerLength + 2)
    {
        emit errorOccurred("we received a binary message, but it is missing the audio data");
        return;
    }
    m_audioData.append(message.mid(2 + headerLength));
}

void QEdgeTTS::onError(QAbstractSocket::SocketError error)
{
    emit errorOccurred(m_webSocket.errorString());
}

void QEdgeTTS::setupConnection()
{
    // Setup additional connection parameters
}

void QEdgeTTS::teardownConnection()
{
    m_webSocket.close();
}