#include "rararchivehandler.h"
#include "thirdparty/QtRAR/src/qtrar.h"
#include "thirdparty/QtRAR/src/qtrarfile.h"
#include "thirdparty/QtRAR/src/qtrarfileinfo.h"

RarArchiveHandler::RarArchiveHandler(const QString &archive) : m_archive(archive)
{
    QtRAR rar(archive);
    if (!rar.open(QtRAR::OpenModeList))
    {
        return;
    }

    // TODO: Support password
    if (rar.isHeadersEncrypted() || rar.isFilesEncrypted())
    {
        return;
    }

    QStringList fileNameList = rar.fileNameList();
    for (auto &fn : fileNameList)
    {
        if (fn.endsWith(".sgf", Qt::CaseInsensitive))
            m_fileList.append(fn);
    }
    rar.close();
}

RarArchiveHandler::~RarArchiveHandler()
{
    if (m_buffer.isOpen())
        m_buffer.close();
}

const QStringList &RarArchiveHandler::getSGFFileList()
{
    return m_fileList;
}

QIODevice *RarArchiveHandler::getSGFContent(const QString &fileName)
{
    QtRARFile file(m_archive, fileName);
    if (file.open(QIODevice::ReadOnly))
    {
        auto data = file.readAll();
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
    }
    return nullptr;
}
