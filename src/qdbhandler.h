#pragma once
#include <QBuffer>

#include "archivehandler.h"

class QListWidgetItem;

class QDBHandler : public ArchiveHandler
{
    Q_OBJECT
public:
    explicit QDBHandler(const QString &archive);
    QDBHandler();
    virtual ~QDBHandler();
    const QStringList     &getSGFFileList() override;
    QIODevice             *getSGFContent(const QString &fileName) override;
    QIODevice             *getSGFContent(int index) override;
    QIODevice             *getCurrentSGFContent() override;
    ArchiveItemListWidget *getArchiveItemListWidget() override;
    QStringList            getNameFilters() override;
    bool                   hasSGF() override;
private slots:
    void onItemSelected(const QString &item);
    void onItemActivated(QListWidgetItem *item);

private:
    ArchiveItemListWidget *m_itemListWidget;
    QString                m_archivePath;
    QStringList            m_fileList;
    QBuffer                m_buffer;

    void setupUi();
protected:
    void readDB(const QString &archive);
};
