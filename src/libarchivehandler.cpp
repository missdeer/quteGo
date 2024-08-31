#include <archive.h>
#include <archive_entry.h>

#include <QListWidget>
#include <QTextCodec>
#include <QVBoxLayout>
#include <QVector>

#include "libarchivehandler.h"
#include "archivehandlerfactory.h"


namespace
{
    bool bRes =
        ArchiveHandlerFactory::registerArchiveHandler({"zip", "rar", "7z"},
                                                      {QObject::tr("Archieve files (*.zip *.rar *.7z)")},
                                                      [](const QString &archive) -> ArchiveHandler * { return new LibArchiveHandler(archive); });
} // namespace

LibArchiveHandler::LibArchiveHandler(const QString &archive) : m_itemListWidget(new ArchiveItemListWidget()), m_archivePath(archive)
{
    QVBoxLayout *layout = new QVBoxLayout;
    m_itemListWidget->setLayout(layout);
    QListWidget *pListWidget = new QListWidget(m_itemListWidget);
    pListWidget->connect(pListWidget, &QListWidget::currentTextChanged, this, &LibArchiveHandler::onItemSelected);
    connect(pListWidget, &QListWidget::itemActivated, this, &LibArchiveHandler::onItemActivated);
    layout->addWidget(pListWidget);
    layout->setContentsMargins(0,0,0,0);
    traverseArchive(m_archivePath, [this, pListWidget](struct archive *a, struct archive_entry *entry) {
        QString currentFilePath = getEntryName(entry);
        if (currentFilePath.endsWith(".sgf", Qt::CaseInsensitive))
        {
            m_fileList.append(currentFilePath);
            pListWidget->addItem(currentFilePath);
        }
        archive_read_data_skip(a);
        return false;
    });
}

LibArchiveHandler::~LibArchiveHandler()
{
    if (m_buffer.isOpen())
        m_buffer.close();
    delete m_itemListWidget;
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

QIODevice *LibArchiveHandler::getSGFContent(int index)
{
    if (index < 0 || index >= m_fileList.size())
        return nullptr;
    return getSGFContent(m_fileList[index]);
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
#if defined(Q_OS_WIN)
    if (!m_archivePath.endsWith(".zip", Qt::CaseInsensitive))
    {
        const auto *entryPath = archive_entry_pathname_w(entry);
        return QString::fromWCharArray(entryPath);
    }
#endif
    const auto *entryPath = archive_entry_pathname(entry);
    // TODO: zip archive may have different encoding with system, should add an explicit option to set encoding
    auto *codec = QTextCodec::codecForName("GBK");
    if (codec)
    {
        return codec->toUnicode(entryPath);
    }
    return QString::fromLocal8Bit(entryPath);
}

ArchiveItemListWidget *LibArchiveHandler::getArchiveItemListWidget()
{
    return m_itemListWidget;
}

void LibArchiveHandler::onItemSelected(const QString &item)
{
    getSGFContent(item);
    emit currentItemChanged();
}

void LibArchiveHandler::onItemActivated(QListWidgetItem *item)
{
    getSGFContent(item->text());
    emit itemActivated();
}

QIODevice *LibArchiveHandler::getCurrentSGFContent()
{
    return &m_buffer;
}

QStringList LibArchiveHandler::getNameFilters()
{
    return {
        tr("Archieve files (*.zip *.rar *.7z)"),
    };
}

bool LibArchiveHandler::hasSGF()
{
    return !m_fileList.empty();
}
