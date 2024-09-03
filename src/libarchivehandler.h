#ifndef LIBARCHIVEHANDLER_H
#define LIBARCHIVEHANDLER_H

#include <QBuffer>

#include "archivehandler.h"

struct archive;
struct archive_entry;
class QListWidgetItem;
using LibArchiveTraversalCallback = std::function<bool(struct archive *, struct archive_entry *entry)>;

class LibArchiveHandler : public ArchiveHandler
{
    Q_OBJECT
public:
    explicit LibArchiveHandler(const QString &archive);
    ~LibArchiveHandler() override;
    QIODevice             *getSGFContent(int index) override;
    QIODevice             *getCurrentSGFContent() override;
    QString                getSGFName(int index) override;
    QString                getCurrentSGFName() override;
    ArchiveItemListWidget *getArchiveItemListWidget() override;
    bool                   hasSGF() override;
private slots:
    void onItemSelected(const QString &item);
    void onItemActivated(QListWidgetItem *item);

private:
    ArchiveItemListWidget *m_itemListWidget;
    QString                m_archivePath;
    QStringList            m_fileList;
    QBuffer                m_buffer;
    QString                m_currentSGFName;

    bool       traverseArchive(const QString &archive, LibArchiveTraversalCallback callback);
    QString    getEntryName(struct archive_entry *entry);
    QIODevice *getSGFContent(const QString &fileName);
};

#endif // LIBARCHIVEHANDLER_H
