#pragma once
#include <QBuffer>

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
    QIODevice             *getSGFContent(const QString &fileName) override;
    QIODevice             *getSGFContent(int index) override;
    QIODevice             *getCurrentSGFContent() override;
    ArchiveItemListWidget *getArchiveItemListWidget() override;
    bool                   hasSGF() override;
private slots:
    void onItemSelected(const QString &item);
    void onItemActivated(QListWidgetItem *item);

private:
    ArchiveItemListWidget *m_itemListWidget;
    QDBItemModel          *m_pModel;
    QString                m_archivePath;
    QBuffer                m_buffer;

    void setupUi();

protected:
    void readDB(const QString &archive);
};
