#ifndef RARARCHIVEHANDLER_H
#define RARARCHIVEHANDLER_H

#include <QBuffer>

#include "archivehandler.h"

class RarArchiveHandler : public ArchiveHandler
{
public:
    explicit RarArchiveHandler(const QString &archive);
    ~RarArchiveHandler();
    const QStringList &getSGFFileList();
    QIODevice         *getSGFContent(const QString &fileName);

private:
    QString     m_archive;
    QStringList m_fileList;
    QBuffer     m_buffer;
};

#endif // RARARCHIVEHANDLER_H
