#ifndef ARCHIVEITEMLISTWIDGET_H
#define ARCHIVEITEMLISTWIDGET_H

#include <QWidget>

class ArchiveItemListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ArchiveItemListWidget(QWidget *parent = nullptr);
    virtual ~ArchiveItemListWidget() = default;

signals:
    void currentItemChanged();
    void itemActivated();
};

#endif // ARCHIVEITEMLISTWIDGET_H
