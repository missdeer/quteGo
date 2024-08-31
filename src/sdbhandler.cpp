#include <QCoreApplication>
#include <QDir>
#include <QListWidget>
#include <QProcess>
#include <QVBoxLayout>

#include "sdbhandler.h"
#include "archivehandlerfactory.h"

namespace
{
    bool bRes1 = ArchiveHandlerFactory::registerArchiveHandler(
        "sdb", QObject::tr("Stonebase database files (*.sdb)"), [](const QString &archive) -> ArchiveHandler * { return new SDBHandler(archive); });
    bool bRes2 = ArchiveHandlerFactory::registerArchiveHandler(
        "qdb", QObject::tr("quteGo database files (*.qdb)"), [](const QString &archive) -> ArchiveHandler * { return new QDBHandler(archive); });
} // namespace

QDBHandler::QDBHandler(const QString &archive) : m_archivePath(archive)
{
    setupUi();
    readDB(m_archivePath);
}

QDBHandler::QDBHandler()
{
    setupUi();
}

QDBHandler::~QDBHandler()
{
    if (m_buffer.isOpen())
        m_buffer.close();
    delete m_itemListWidget;
}

void QDBHandler::setupUi()
{
    m_itemListWidget    = new ArchiveItemListWidget();
    QVBoxLayout *layout = new QVBoxLayout;
    m_itemListWidget->setLayout(layout);
    QListWidget *pListWidget = new QListWidget(m_itemListWidget);
    pListWidget->connect(pListWidget, &QListWidget::currentTextChanged, this, &QDBHandler::onItemSelected);
    connect(pListWidget, &QListWidget::itemActivated, this, &QDBHandler::onItemActivated);
    layout->addWidget(pListWidget);
    layout->setContentsMargins(0, 0, 0, 0);
}

void QDBHandler::readDB(const QString &archive)
{
    m_archivePath = archive;
}

const QStringList &QDBHandler::getSGFFileList()
{
    return m_fileList;
}

QIODevice *QDBHandler::getSGFContent(const QString &fileName)
{
    return nullptr;
}

QIODevice *QDBHandler::getSGFContent(int index)
{
    if (index < 0 || index >= m_fileList.size())
        return nullptr;
    return getSGFContent(m_fileList[index]);
}

ArchiveItemListWidget *QDBHandler::getArchiveItemListWidget()
{
    return m_itemListWidget;
}

void QDBHandler::onItemSelected(const QString &item)
{
    getSGFContent(item);
    emit currentItemChanged();
}

void QDBHandler::onItemActivated(QListWidgetItem *item)
{
    getSGFContent(item->text());
    emit itemActivated();
}

QIODevice *QDBHandler::getCurrentSGFContent()
{
    return &m_buffer;
}

QStringList QDBHandler::getNameFilters()
{
    return {
        tr("Archieve files (*.zip *.rar *.7z)"),
    };
}

bool QDBHandler::hasSGF()
{
    return !m_fileList.empty();
}

SDBHandler::SDBHandler(const QString &archive) : QDBHandler()
{
    // convert sdb to qdb
    QString sdb = QDir::toNativeSeparators(archive);
    QString qdb = sdb;
    qdb.replace(sdb.length() - 4, 4, ".qdb");
    QString sdb2qdb = QCoreApplication::applicationDirPath() + "/sdb2qdb.exe";
    QProcess::execute(sdb2qdb, {QStringLiteral("--convert=") + qdb, sdb});
    // read qdb
    readDB(qdb);
}
