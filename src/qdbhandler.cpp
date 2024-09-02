#include <QCoreApplication>
#include <QDir>
#include <QProcess>
#include <QTableView>
#include <QVBoxLayout>

#include "qdbhandler.h"
#include "archivehandlerfactory.h"
#include "qdbitemmodel.h"

namespace
{
    bool bRes2 = ArchiveHandlerFactory::registerArchiveHandler(
        "qdb", QObject::tr("quteGo database files (*.qdb)"), [](const QString &archive) -> ArchiveHandler * { return new QDBHandler(archive); });
} // namespace

QDBHandler::QDBHandler(const QString &archive)
    : m_itemListWidget(new ArchiveItemListWidget), m_model(new QDBItemModel(m_itemListWidget)), m_archivePath(archive)
{
    setupUi();
    readDB(m_archivePath);
}

QDBHandler::QDBHandler() : m_itemListWidget(new ArchiveItemListWidget), m_model(new QDBItemModel(m_itemListWidget))
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
    QVBoxLayout *layout = new QVBoxLayout;
    m_itemListWidget->setLayout(layout);
    QTableView *pTableView = new QTableView(m_itemListWidget);
    pTableView->setModel(m_model);
    pTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    pTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    layout->addWidget(pTableView);
    layout->setContentsMargins(0, 0, 0, 0);
}

void QDBHandler::readDB(const QString &archive)
{
    m_archivePath = archive;
    Q_ASSERT(m_model);
    m_model->loadQDB(archive);
}

QIODevice *QDBHandler::getSGFContent(int index)
{
    if (index < 0 || index >= m_model->rowCount())
        return nullptr;
    QByteArray sgf = m_model->getSGFContent(index);
    if (m_buffer.isOpen())
        m_buffer.close();
    m_buffer.setData(sgf);
    if (m_buffer.open(QIODevice::ReadOnly))
        m_buffer.seek(0);
    return &m_buffer;
}

ArchiveItemListWidget *QDBHandler::getArchiveItemListWidget()
{
    return m_itemListWidget;
}

QIODevice *QDBHandler::getCurrentSGFContent()
{
    return &m_buffer;
}

bool QDBHandler::hasSGF()
{
    return m_model->rowCount() != 0;
}
