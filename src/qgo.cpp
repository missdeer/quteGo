/*
 * qgo.cpp
 */

#include <QDebug>
#include <QDir>
#include <QLineEdit>
#include <QMessageBox>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QRandomGenerator>

#include "qgo.h"
#include "audio.h"
#include "board.h"
#include "config.h"
#include "defines.h"
#include "helpviewer.h"
#include "mainwindow.h"
#include "setting.h"

#if defined(Q_OS_MACX)
#    include <CoreFoundation/CFBundle.h>
#    include <CoreFoundation/CFString.h>
#endif // Q_OS_MACX

qGo::qGo()
{
    m_mediaPlayer      = new QMediaPlayer;
    m_mediaAudioOutput = new QAudioOutput;
    m_mediaAudioOutput->setVolume(1.f);
    m_mediaPlayer->setAudioOutput(m_mediaAudioOutput);
}

qGo::~qGo()
{
    delete helpViewer;
    delete m_mediaPlayer;
    delete m_mediaAudioOutput;
}

/* @@@ check if there is a use case for this, and repair or remove as necessary.
 */
void qGo::unused_quit()
{
#if 0
	emit signal_leave_qgo();
	int check = checkModified();
	if (check == 0) {
		QMessageBox::warning(0, PACKAGE,
				     tr("At least one board is modified.\n"
					"If you exit the application now, all changes will be lost!"
					"\nExit anyway?"),
				     tr("Yes"), tr("No"), {},
				     1, 0);
		/* This code never did anything with the message box result.  */
	}
	qDebug() << "Program quits now...";
#endif
}

void qGo::openManual(const QUrl &url)
{
    if (helpViewer == nullptr)
        helpViewer = new HelpViewer(0);
    helpViewer->set_url(url);
    helpViewer->show();
    helpViewer->raise();
}

int qGo::checkModified()
{
    for (auto it : main_window_list)
    {
        if (!it->checkModified(false))
            return 0;
    }
    return 1;
}

void qGo::updateAllBoardSettings()
{
    for (auto it : main_window_list)
        it->update_settings();
    g_setting->engines_changed = false;
}

void qGo::playClick()
{
    playSound(QStringLiteral("/sounds/click.wav"));
}

void qGo::playStoneSound()
{
    int index = QRandomGenerator::global()->bounded(1, 12);
    if (index == 1)
        playSound(QStringLiteral("/sounds/stone.wav"));
    else
        playSound(QStringLiteral("/sounds/stone%1.wav").arg(index));
}

void qGo::playAutoPlayClick()
{
    if (g_setting->readBoolEntry("SOUND_AUTOPLAY"))
    {
        playSound(QStringLiteral("/sounds/click.wav"));
    }
}

void qGo::playTalkSound()
{
    if (g_setting->readBoolEntry("SOUND_TALK"))
    {
        playSound(QStringLiteral("/sounds/talk.wav"));
    }
}

void qGo::playMatchSound()
{
    if (g_setting->readBoolEntry("SOUND_MATCH"))
    {
        playSound(QStringLiteral("/sounds/match.wav"));
    }
}

void qGo::playPassSound()
{
    if (g_setting->readBoolEntry("SOUND_PASS"))
    {
        playSound(QStringLiteral("/sounds/pass.wav"));
    }
}

void qGo::playGameEndSound()
{
    if (g_setting->readBoolEntry("SOUND_GAMEEND"))
    {
        playSound(QStringLiteral("/sounds/gameend.wav"));
    }
}

void qGo::playTimeSound()
{
    if (g_setting->readBoolEntry("SOUND_TIME"))
    {
        playSound(QStringLiteral("/sounds/tictoc.wav"));
    }
}

void qGo::playSaySound()
{
    if (g_setting->readBoolEntry("SOUND_SAY"))
    {
        playSound(QStringLiteral("/sounds/say.wav"));
    }
}

void qGo::playEnterSound()
{
    if (g_setting->readBoolEntry("SOUND_ENTER"))
    {
        playSound(QStringLiteral("/sounds/enter.wav"));
    }
}

void qGo::playLeaveSound()
{
    if (g_setting->readBoolEntry("SOUND_LEAVE"))
    {
        playSound(QStringLiteral("/sounds/leave.wav"));
    }
}

void qGo::playConnectSound()
{
    if (g_setting->readBoolEntry("SOUND_CONNECT"))
    {
        playSound(QStringLiteral("/sounds/connect.wav"));
    }
}

void qGo::playDisConnectSound()
{
    if (g_setting->readBoolEntry("SOUND_DISCONNECT"))
    {
        playSound(QStringLiteral("/sounds/connect.wav"));
    }
}

void qGo::playSound(const QString& sound)
{
#if defined(Q_OS_MACX)
    m_mediaPlayer->setSource(QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + "/../Resources" + sound));
#else
    m_mediaPlayer->setSource(QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + sound));
#endif
    m_mediaPlayer->play();
}

void help_about()
{
    QString txt;
    txt = "<p>Copyright \u00a9 2019-2024\nFan Yang "
           "&lt;me@minidump.info&gt;</p>";
    txt += "<p>Copyright \u00a9 2011-2019\nBernd Schmidt "
          "&lt;bernds_cb1@t-online.de&gt;</p>";
    txt += "<p>Copyright \u00a9 2001-2006\nPeter Strempel "
           "&lt;pstrempel@t-online.de&gt;, Johannes Mesa &lt;frosla@gmx.at&gt;, "
           "Emmanuel B\u00E9ranger &lt;yfh2@hotmail.com&gt;</p>";
    txt +=
        "<p>" + QObject::tr("GTP code originally from Goliath, thanks to: ") + "PALM Thomas, DINTILHAC Florian, HIVERT Anthony, PIOC Sebastien</p>";

    txt += "<hr/><p>Visit <a href=\"https://github.com/missdeer/qutego\">the Github "
           "repository</a> for new versions.</p>";
    QString translation = QString("<hr/><p>") + QObject::tr("Please set your own language and your name! Use your own language!") + QString("</p>");
    txt += translation;

    QMessageBox mb;
    mb.setWindowTitle(PACKAGE " " VERSION);
    mb.setTextFormat(Qt::RichText);
    mb.setText(txt);
    mb.exec();
}

void help_new_version()
{
    QMessageBox mb;
    mb.setWindowTitle(PACKAGE);
    mb.setTextFormat(Qt::RichText);
    mb.setText(NEWVERSIONWARNING);
    mb.exec();
}
