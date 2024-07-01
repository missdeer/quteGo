/*
 * qgo.cpp
 */

#include <QDebug>
#include <QDir>
#include <QLineEdit>
#include <QMessageBox>
#include <QSoundEffect>

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

qGo::qGo() {}

qGo::~qGo()
{
    delete helpViewer;
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
    QSoundEffect se;
    se.setSource(QUrl(":/sounds/click.wav"));
    se.play();
}

void qGo::playStoneSound()
{
    static int idx = 0;

    QSoundEffect se;
    switch (idx % 11)
    {
    default:
        se.setSource(QUrl(":/sounds/stone.wav"));
        break;
    case 0:
        se.setSource(QUrl(":/sounds/stone2.wav"));
        break;
    case 1:
        se.setSource(QUrl(":/sounds/stone3.wav"));
        break;
    case 2:
        se.setSource(QUrl(":/sounds/stone4.wav"));
        break;
    case 3:
        se.setSource(QUrl(":/sounds/stone5.wav"));
        break;
    case 4:
        se.setSource(QUrl(":/sounds/stone6.wav"));
        break;
    case 5:
        se.setSource(QUrl(":/sounds/stone7.wav"));
        break;
    case 6:
        se.setSource(QUrl(":/sounds/stone8.wav"));
        break;
    case 7:
        se.setSource(QUrl(":/sounds/stone9.wav"));
        break;
    case 8:
        se.setSource(QUrl(":/sounds/stone10.wav"));
        break;
    case 9:
        se.setSource(QUrl(":/sounds/stone11.wav"));
        break;
    }
    se.play();

    idx += 1 + rand() % 4;
}

void qGo::playAutoPlayClick()
{
    if (g_setting->readBoolEntry("SOUND_AUTOPLAY"))
    {
        QSoundEffect se;
        se.setSource(QUrl(":/sounds/click.wav"));
        se.play();
    }
}

void qGo::playTalkSound()
{
    if (g_setting->readBoolEntry("SOUND_TALK"))
    {
        QSoundEffect se;
        se.setSource(QUrl(":/sounds/talk.wav"));
        se.play();
    }
}

void qGo::playMatchSound()
{
    if (g_setting->readBoolEntry("SOUND_MATCH"))
    {
        QSoundEffect se;
        se.setSource(QUrl(":/sounds/match.wav"));
        se.play();
    }
}

void qGo::playPassSound()
{
    if (g_setting->readBoolEntry("SOUND_PASS"))
    {
        QSoundEffect se;
        se.setSource(QUrl(":/sounds/pass.wav"));
        se.play();
    }
}

void qGo::playGameEndSound()
{
    if (g_setting->readBoolEntry("SOUND_GAMEEND"))
    {
        QSoundEffect se;
        se.setSource(QUrl(":/sounds/gameend.wav"));
        se.play();
    }
}

void qGo::playTimeSound()
{
    if (g_setting->readBoolEntry("SOUND_TIME"))
    {
        QSoundEffect se;
        se.setSource(QUrl(":/sounds/tictoc.wav"));
        se.play();
    }
}

void qGo::playSaySound()
{
    if (g_setting->readBoolEntry("SOUND_SAY"))
    {
        QSoundEffect se;
        se.setSource(QUrl(":/sounds/say.wav"));
        se.play();
    }
}

void qGo::playEnterSound()
{
    if (g_setting->readBoolEntry("SOUND_ENTER"))
    {
        QSoundEffect se;
        se.setSource(QUrl(":/sounds/enter.wav"));
        se.play();
    }
}

void qGo::playLeaveSound()
{
    if (g_setting->readBoolEntry("SOUND_LEAVE"))
    {
        QSoundEffect se;
        se.setSource(QUrl(":/sounds/leave.wav"));
        se.play();
    }
}

void qGo::playConnectSound()
{
    if (g_setting->readBoolEntry("SOUND_CONNECT"))
    {
        QSoundEffect se;
        se.setSource(QUrl(":/sounds/connect.wav"));
        se.play();
    }
}

void qGo::playDisConnectSound()
{
    if (g_setting->readBoolEntry("SOUND_DISCONNECT"))
    {
        QSoundEffect se;
        se.setSource(QUrl(":/sounds/connect.wav"));
        se.play();
    }
}

void help_about()
{
    QString txt;
    txt = "<p>Copyright \u00a9 2011-2019\nBernd Schmidt "
          "&lt;bernds_cb1@t-online.de&gt;</p>";
    txt += "<p>Copyright \u00a9 2001-2006\nPeter Strempel "
           "&lt;pstrempel@t-online.de&gt;, Johannes Mesa &lt;frosla@gmx.at&gt;, "
           "Emmanuel B\u00E9ranger &lt;yfh2@hotmail.com&gt;</p>";
    txt +=
        "<p>" + QObject::tr("GTP code originally from Goliath, thanks to: ") + "PALM Thomas, DINTILHAC Florian, HIVERT Anthony, PIOC Sebastien</p>";

    txt += "<hr/><p>Visit <a href=\"https://github.com/bernds/qutego\">the Github "
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
