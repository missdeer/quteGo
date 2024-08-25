#include <archive.h>
#include <archive_entry.h>

#include <QTextCodec>
#include <QVector>

#include "libarchivehandler.h"
#include "archivehandlerfactory.h"

namespace
{
    bool bRes = ArchiveHandlerFactory::registerArchiveHandler(
        {"zip", "rar", "7z"}, [](const QString &archive) -> ArchiveHandler * { return new LibArchiveHandler(archive); });
} // namespace

LibArchiveHandler::LibArchiveHandler(const QString &archive) : m_archivePath(archive)
{
    traverseArchive(m_archivePath, [this](struct archive *a, struct archive_entry *entry) {
        QString currentFilePath = getEntryName(entry);
        if (currentFilePath.endsWith(".sgf", Qt::CaseInsensitive))
        {
            m_fileList.append(currentFilePath);
        }
        archive_read_data_skip(a);
        return false;
    });
}

LibArchiveHandler::~LibArchiveHandler()
{
    if (m_buffer.isOpen())
        m_buffer.close();
}

const QStringList &LibArchiveHandler::getSGFFileList()
{
    return m_fileList;
}

QIODevice *LibArchiveHandler::getSGFContent(const QString &fileName)
{
    traverseArchive(m_archivePath, [this, fileName](struct archive *a, struct archive_entry *entry) {
        QString currentFilePath = getEntryName(entry);
        if (fileName == currentFilePath)
        {
            const void *buff;
            size_t      size;
            la_int64_t  offset;

            QByteArray content;
            while (archive_read_data_block(a, &buff, &size, &offset) == ARCHIVE_OK)
            {
                content.append(static_cast<const char *>(buff), int(size));
            }

            if (m_buffer.isOpen())
                m_buffer.close();
            m_buffer.setData(content);
            if (m_buffer.open(QIODevice::ReadOnly))
            {
                m_buffer.seek(0);
                return true;
            }
        }
        archive_read_data_skip(a);
        return false;
    });
    return &m_buffer;
}

bool LibArchiveHandler::traverseArchive(const QString &archive, LibArchiveTraversalCallback callback)
{
    auto *a = archive_read_new();
    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);
#if defined(Q_OS_WIN)
    auto r = archive_read_open_filename_w(a, m_archivePath.toStdWString().c_str(), 10240);
#else
    auto r = archive_read_open_filename(a, m_archivePath.toStdString().c_str(), 10240);
#endif
    if (r != ARCHIVE_OK)
        return false;
    struct archive_entry *entry;
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK)
    {
        if (callback(a, entry))
        {
            break;
        }
    }
    r = archive_read_free(a);
    return true;
}

QString LibArchiveHandler::getEntryName(struct archive_entry *entry)
{
    const auto *entryPath = archive_entry_pathname(entry);
    auto       *codec     = QTextCodec::codecForName("GBK");
    if (codec)
    {
        return codec->toUnicode(entryPath);
    }
    return QString::fromLatin1(entryPath);
}