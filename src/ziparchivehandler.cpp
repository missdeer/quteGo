#include <QVector>
#include "ziparchivehandler.h"

ZipArchiveHandler::ZipArchiveHandler(const QString &archive)
	: m_zipReader(archive)
{
	auto fil = m_zipReader.fileInfoList();
	for (auto & fi : fil)
	{
		if (fi.isFile && fi.filePath.endsWith(".sgf", Qt::CaseInsensitive))
			m_fileList.append(fi.filePath);
	}
}

ZipArchiveHandler::~ZipArchiveHandler()
{
	if (m_zipReader.isReadable())
		m_zipReader.close();
	if (m_buffer.isOpen())
		m_buffer.close();
}

const QStringList &ZipArchiveHandler::getSGFFileList()
{
	return m_fileList;
}

QIODevice *ZipArchiveHandler::getSGFContent(const QString &fileName)
{
	if (m_buffer.isOpen())
		m_buffer.close();
	auto data = m_zipReader.fileData(fileName);
	m_buffer.setBuffer(&data);
	m_buffer.open(QIODevice::ReadOnly);
	return &m_buffer;
}
