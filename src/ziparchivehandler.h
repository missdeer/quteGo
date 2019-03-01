#ifndef ZIPARCHIVEHANDLER_H
#define ZIPARCHIVEHANDLER_H

#include <private/qzipreader_p.h>
#include <QBuffer>
#include "archivehandler.h"

class ZipArchiveHandler : public ArchiveHandler
{
public:
	explicit ZipArchiveHandler(const QString &archive);
	~ZipArchiveHandler();
	const QStringList &getSGFFileList();
	QIODevice *getSGFContent(const QString &fileName);
private:
	QZipReader m_zipReader;
	QStringList m_fileList;
	QBuffer m_buffer;
};

#endif // ZIPARCHIVEHANDLER_H
