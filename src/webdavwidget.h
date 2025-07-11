#pragma once

#include <QWidget>
#include <qwebdav.h>
#include <qwebdavdirparser.h>

QT_BEGIN_NAMESPACE
class QTableWidget;
class QTableWidgetItem;
QT_END_NAMESPACE

class WebDavWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WebDavWidget(QWidget *parent = nullptr);

    void                         uploadFile(const QString &filePath);
    void                         uploadData(const QString &fileName, const QByteArray &content);
    bool                         onStart();
    [[nodiscard]] bool           isConnected() const;
    [[nodiscard]] const QString &currentPath() const;
public slots:
    void onSaveDocument(const QString &fullUrl, const QByteArray &content);

signals:
    void retrievedWebDavFile(QString path, QByteArray content);
    void storedWebDavFile(QString path);
    void error(QString);

private slots:
    void onConnectActionTriggered();
    void onDisconnectActionTriggered();
    void onRefreshActionTriggered();
    void onUpLevelActionTriggered();
    void onNewDocumentActionTriggered();
    void onNewDirectoryActionTriggered();
    void onRenameActionTriggered();
    void onRemoveActionTriggered();
    void onUploadActionTriggered();

    void onWebDavDirParserFinished();
    void onError(const QString &err);
    void onNetworkError(QNetworkReply::NetworkError err, QString errMsg);
    void onTableWidgetItemActivited(QTableWidgetItem *item);
    void onRequestWebDavFileFinished();
    void onPutWebDavFileFinished();
    void onRemoveWebDavFileFinished();
    void onNewWebDavDirectoryFinished();
    void onRenameWebDavItemFinished();
    void onUploadWebDavFileFinished();
    void onNewWebDavFileFinished();

    void onItemSelectionChanged();

private:
    QTableWidget    *m_tableWidget;
    QAction         *m_connectAction;
    QAction         *m_disconnectAction;
    QAction         *m_refreshAction;
    QAction         *m_upLevelAction;
    QAction         *m_newDocumentAction;
    QAction         *m_newDirectoryAction;
    QAction         *m_uploadAction;
    QAction         *m_renameAction;
    QAction         *m_removeAction;
    QWebdav          m_webdav;
    QWebdavDirParser m_webdavDirParser;
    QString          m_serverUrl;
    QString          m_currentPath;
    bool             m_isConnected {false};

    [[nodiscard]] bool    hasSelected() const;
    [[nodiscard]] bool    isSelectedDir() const;
    [[nodiscard]] bool    isDir(QTableWidgetItem *item) const;
    [[nodiscard]] QString getSelectedPath() const;
    [[nodiscard]] QString getSelectedName() const;
    void                  doConnect();
    void                  saveLastPathToSettings();
};
