/*
 * helpviewer.cpp
 */

#include <QApplication>
#include <QPixmap>
#include <QStringList>

#include "helpviewer.h"
#include "config.h"
#include "qgo.h"
#include "setting.h"
#ifdef HAVE_CONFIG_H
#    include <config.h>
#endif

HelpViewer::HelpViewer(QWidget *parent) : QMainWindow(parent)
{
    resize(600, 480);

    setWindowTitle(PACKAGE " " VERSION " Manual");
    setWindowIcon(QIcon(":/ClientWindow/images/clientwindow/qgo.png"));

    browser = new QTextBrowser(this);

    setCentralWidget(browser);

    initToolBar();
}

void HelpViewer::set_url(const QUrl &url)
{
    QStringList strList;

#if defined(Q_OS_WIN)
    strList << QApplication::applicationDirPath() + "/html";
#else
    strList << QApplication::applicationDirPath() + "/html"
            << "/usr/share/doc/" PACKAGE "/html"
            << "/usr/doc/" PACKAGE "/html"
            << "/usr/local/share/doc/" PACKAGE "/html"
            << "/usr/local/doc/" PACKAGE "/html"
            << "./html/";
#endif

    browser->setSearchPaths(strList);
    browser->setSource(url);
}

HelpViewer::~HelpViewer() {}

void HelpViewer::initToolBar()
{
    toolBar = addToolBar("Navigation Bar");

    buttonClose = new QAction(QIcon(":/images/exit.png"), "Close", this);
    connect(buttonClose, &QAction::triggered, this, [=](bool) { close(); });
    buttonClose->setShortcut(Qt::Key_Escape);
    toolBar->addAction(buttonClose);

    buttonHome = new QAction(QIcon(":/Help/images/help/gohome.png"), "Home", this);
    connect(buttonHome, &QAction::triggered, [=](bool) { browser->home(); });
    toolBar->addAction(buttonHome);

    buttonBack = new QAction(QIcon(":/MainWidgetGui/images/mainwidget/1leftarrow.png"), "Home", this);
    connect(buttonBack, &QAction::triggered, [=](bool) { browser->backward(); });
    toolBar->addAction(buttonBack);

    buttonForward = new QAction(QIcon(":/MainWidgetGui/images/mainwidget/1rightarrow.png"), "Home", this);
    connect(buttonForward, &QAction::triggered, [=](bool) { browser->forward(); });
    toolBar->addAction(buttonForward);

    buttonForward->setEnabled(false);
    buttonBack->setEnabled(false);
    connect(browser, &QTextBrowser::backwardAvailable, [=](bool set) { buttonBack->setEnabled(set); });
    connect(browser, &QTextBrowser::forwardAvailable, [=](bool set) { buttonForward->setEnabled(set); });
}
