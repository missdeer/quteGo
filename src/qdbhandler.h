#pragma once
#include <QBuffer>
#include <QTableView>

#include "archivehandler.h"


class QListWidgetItem;
class QDBItemModel;
class QDBHandler : public ArchiveHandler
{
    Q_OBJECT
public:
    explicit QDBHandler(const QString &archive);
    QDBHandler();
    virtual ~QDBHandler();
    QIODevice             *getSGFContent(int index) override;
    QIODevice             *getCurrentSGFContent() override;
    ArchiveItemListWidget *getArchiveItemListWidget() override;
    bool                   hasSGF() override;
private slots:
    void onItemActivated(const QModelIndex &index);
    void onCurrentChanged(const QModelIndex &current, const QModelIndex &previous);
private:
    ArchiveItemListWidget *m_itemListWidget;
    QDBItemModel          *m_model;
    QString                m_archivePath;
    QBuffer                m_buffer;

    void setupUi();

protected:
    void readDB(const QString &archive);
};

class ArchiveItemTableView : public QTableView
{
    Q_OBJECT
public:
    using QTableView::QTableView;

    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;
signals:
    void archiveItemTableViewCurrentChanged(const QModelIndex &, const QModelIndex &);
};