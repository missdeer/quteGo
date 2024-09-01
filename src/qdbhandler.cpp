#include <QCoreApplication>
#include <QDir>
#include <QListWidget>
#include <QProcess>
#include <QVBoxLayout>

#include "qdbhandler.h"
#include "archivehandlerfactory.h"
#include "qdbitemmodel.h"

namespace
{
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
    QListView *pListWidget = new QListView(m_itemListWidget);
    m_pModel               = new QDBItemModel(pListWidget);
    pListWidget->setModel(m_pModel);
    layout->addWidget(pListWidget);
    layout->setContentsMargins(0, 0, 0, 0);
}

void QDBHandler::readDB(const QString &archive)
{
    m_archivePath = archive;
    Q_ASSERT(m_pModel);
    m_pModel->loadQDB(archive);
}

QIODevice *QDBHandler::getSGFContent(const QString &fileName)
{
    return nullptr;
}

QIODevice *QDBHandler::getSGFContent(int index)
{
    if (index < 0 || index >= m_pModel->rowCount())
        return nullptr;
    QByteArray sgf = m_pModel->getSGFContent(index);
    m_buffer.setData(sgf);
    m_buffer.seek(0);
    return &m_buffer;
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

bool QDBHandler::hasSGF()
{
    return m_pModel->rowCount() != 0;
}
