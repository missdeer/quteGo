#ifndef QEDGETTS_H
#define QEDGETTS_H

#include <QNetworkAccessManager>
#include <QObject>
#include <QWebSocket>

struct VoiceIdentity
{
    QString name;
    QString shortName;
    QString locale;
    QString gender;
    QString contentCategory;
    QString voicePersonalities;
    QString friendlyName;
    QString status;
    QString suggestedCodec;
};

class QEdgeTTS : public QObject
{
    Q_OBJECT
public:
    explicit QEdgeTTS(QObject *parent = nullptr);

    void getVoiceList();
    void getTextToSpeech(const QString &text, const QString &voiceName, int volume = 0, int rate = 0, int pitch = 0);
signals:
    void voiceListReceived(const QList<VoiceIdentity> &voiceList);
    void errorOccurred(QString);
    void voiceReceived(QByteArray);
    void finished();
private slots:
    void onVoiceListReceived();
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void onBinaryMessageReceived(const QByteArray &message);
    void onError(QAbstractSocket::SocketError error);

private:
    QNetworkAccessManager m_nam;
    QWebSocket            m_webSocket;
    QByteArray            m_audioData;
    QString               m_outputFilePath;
    QString               m_textToSpeech;
    QString               m_voiceName;
    int                   m_volume;
    int                   m_rate;
    int                   m_pitch;

    void setupConnection();
    void teardownConnection();
    void connectToServer(QNetworkRequest &request);
};

#endif // QEDGETTS_H
