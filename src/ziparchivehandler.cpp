#include <QVector>

#include "ziparchivehandler.h"
#include "archivehandlerfactory.h"


namespace
{
    bool bRes = ArchiveHandlerFactory::registerArchiveHandler(
        "zip", [](const QString &archive) -> ArchiveHandler * { return new ZipArchiveHandler(archive); });
} // namespace

ZipArchiveHandler::ZipArchiveHandler(const QString &archive) : m_zipReader(archive)
{
    auto fil = m_zipReader.fileInfoList();
    for (auto &fi : fil)
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
    auto data = m_zipReader.fileData(fileName);
    if (data.isEmpty())
        return nullptr;
    if (m_buffer.isOpen())
        m_buffer.close();
    m_buffer.setData(data);
    if (m_buffer.open(QIODevice::ReadOnly))
    {
        m_buffer.seek(0);
        return &m_buffer;
    }
    return nullptr;
}
