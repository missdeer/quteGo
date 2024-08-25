#ifndef LIBARCHIVEHANDLER_H
#define LIBARCHIVEHANDLER_H

#include <QBuffer>

#include "archivehandler.h"

struct archive;
struct archive_entry;

using LibArchiveTraversalCallback = std::function<bool(struct archive *, struct archive_entry *entry)>;

class LibArchiveHandler : public ArchiveHandler
{
public:
    explicit LibArchiveHandler(const QString &archive);
    ~LibArchiveHandler();
    const QStringList &getSGFFileList();
    QIODevice         *getSGFContent(const QString &fileName);

private:
    QString     m_archivePath;
    QStringList m_fileList;
    QBuffer     m_buffer;

    bool traverseArchive(const QString &archive, LibArchiveTraversalCallback callback);
    QString getEntryName(struct archive_entry *entry);
};

#endif // LIBARCHIVEHANDLER_H
