#include "thirdparty/Qt7z/Qt7z/qt7zfileinfo.h"
#include "sevenzarchivehandler.h"

SevenZArchiveHandler::SevenZArchiveHandler(const QString &archive)
	: m_pkg(archive)
{
	if (!m_pkg.open()) {
		return ;
	}

	QStringList fileNameList = m_pkg.fileNameList();
	for (auto & fn : fileNameList)
	{
		if (fn.endsWith(".sgf", Qt::CaseInsensitive))
			m_fileList.append(fn);
	}
}

SevenZArchiveHandler::~SevenZArchiveHandler()
{
	if (m_pkg.isOpen())
		m_pkg.close();
	if (m_buffer.isOpen())
		m_buffer.close();
}

const QStringList &SevenZArchiveHandler::getSGFFileList()
{
	return m_fileList;
}

QIODevice *SevenZArchiveHandler::getSGFContent(const QString &fileName)
{
	if (m_buffer.isOpen())
		m_buffer.close();
	if (m_buffer.open(QBuffer::ReadWrite) && m_pkg.extractFile(fileName, &m_buffer))
	{
		m_buffer.seek(0);
		return &m_buffer;
	}
	return nullptr;
}
