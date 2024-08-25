#ifndef ARCHIVEHANDLER_H
#define ARCHIVEHANDLER_H

#include <QIODevice>
#include <QSharedPointer>
#include <QStringList>

class ArchiveHandler
{
public:
    virtual ~ArchiveHandler() = default;
    virtual const QStringList &getSGFFileList()                       = 0;
    virtual QIODevice         *getSGFContent(const QString &fileName) = 0;
};

using ArchiveHandlerPtr = QSharedPointer<ArchiveHandler>;

#endif // ARCHIVEHANDLER_H
