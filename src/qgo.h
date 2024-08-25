/*
 * qgo.h
 */

#ifndef QGO_H
#define QGO_H

#include <QObject>
#include <QSoundEffect>

class HelpViewer;
class QUrl;

class qGo : public QObject
{
    Q_OBJECT
public:
    qGo();
    ~qGo();
    void openManual(const QUrl &);
    int  checkModified();
    void updateAllBoardSettings();
    void playClick();
    void playAutoPlayClick();
    void playTalkSound();
    void playMatchSound();
    void playGameEndSound();
    void playPassSound();
    void playTimeSound();
    void playSaySound();
    void playEnterSound();
    void playStoneSound();
    void playLeaveSound();
    void playDisConnectSound();
    void playConnectSound();

public slots:
    void unused_quit();

private:
    QSoundEffect soundEffect;
    HelpViewer *helpViewer {};
};

class QApplication;

extern qGo          *g_quteGo;
extern QApplication *g_qGoApp;

#endif
