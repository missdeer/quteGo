#ifndef ARCHIVEHANDLER_H
#define ARCHIVEHANDLER_H

#include <QStringList>
#include <QIODevice>
#include <QSharedPointer>

class ArchiveHandler
{
public:
	ArchiveHandler();
	virtual ~ArchiveHandler();
	virtual const QStringList &getSGFFileList() = 0;
	virtual QIODevice *getSGFContent(const QString& fileName) = 0;
};

using ArchiveHandlerPtr=QSharedPointer<ArchiveHandler>;

#endif // ARCHIVEHANDLER_H
