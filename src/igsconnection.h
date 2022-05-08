/*

 * igsconnection.h

 */

#ifndef IGSCONNECTION_H
#define IGSCONNECTION_H

#include <QObject>
#include <QString>
#include <QTcpSocket>

class QTextCodec;
struct Host;

#define MAX_LINESIZE 512

class IGSConnection : public QObject
{
    Q_OBJECT

    QWidget *m_lv_p, *m_lv_g;

public:
    IGSConnection(QWidget *, QWidget *);
    virtual ~IGSConnection();

    bool isConnected();
    bool openConnection(const Host &);
    bool closeConnection();
    void sendTextToHost(QString txt, bool ignoreCodec = false);

signals:
    // for statistics reason
    void signal_setBytesIn(int);
    void signal_setBytesOut(int);
    void signal_text_to_app(const QString &);

protected:
    virtual bool checkPrompt(const QString &);
    //	void convertBlockToLines();

    void sendTextToApp(const QString &);

private slots:
    void OnHostFound();
    void OnConnected();
    void OnReadyRead();
    void OnConnectionClosed();
    void OnDelayedCloseFinish();

private:
    QTcpSocket *qsocket;
    QTextCodec *textCodec;

    int   len_saved_data;
    char *saved_data;
    // struct USERINFO {
    QString username;
    QString password;
    //}  userInfo;

    enum
    {
        LOGIN,      // parse will search for login prompt
        PASSWORD,   // parse will search for password prompt
        SESSION,    // logged in
        AUTH_FAILED // wrong user/pass
    } authState;
};

#endif
