/*
 * helpviewer.h
 */

#ifndef HELPVIEWER_H
#define HELPVIEWER_H

#include <QAction>
#include <QMainWindow>
#include <QTextBrowser>
#include <QToolBar>

class HelpViewer : public QMainWindow
{
    Q_OBJECT

public:
    HelpViewer(QWidget *parent = 0);
    ~HelpViewer();
    void set_url(const QUrl &url);

protected:
    void initToolBar();

private:
    QTextBrowser *browser;
    QToolBar     *toolBar;
    QAction      *buttonHome, *buttonClose, *buttonBack, *buttonForward;
};

#endif
