#include <QCoreApplication>
#include <QDir>
#include <QHelpEvent>
#include <QProcess>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QToolTip>
#include <QVBoxLayout>

#include "qdbhandler.h"
#include "archivehandlerfactory.h"
#include "qdbitemmodel.h"

namespace
{
    bool bRes2 = ArchiveHandlerFactory::registerArchiveHandler(QStringLiteral("qdb"),
                                                               QObject::tr("quteGo database files (*.qdb)"),
                                                               [](const QString &archive) -> ArchiveHandler * { return new QDBHandler(archive); });

    class TooltipDelegate : public QStyledItemDelegate
    {
    public:
        TooltipDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

        bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index) override
        {
            if (event->type() == QEvent::ToolTip)
            {
                if (!view->model()->data(index).toString().isEmpty())
                {
                    QRect cellRect = view->visualRect(index);
                    if (!cellRect.contains(event->pos()))
                    {
                        QToolTip::hideText();
                        return true;
                    }
                    QToolTip::showText(event->globalPos(), view->model()->data(index).toString(), view);
                    return true;
                }
            }
            return QStyledItemDelegate::helpEvent(event, view, option, index);
        }
    };
} // namespace

void ArchiveItemTableView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QTableView::currentChanged(current, previous);
    emit archiveItemTableViewCurrentChanged(current, previous);
}

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
    ArchiveItemTableView *pTableView = new ArchiveItemTableView(m_itemListWidget);
    pTableView->setModel(m_model);
    pTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    pTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    pTableView->setItemDelegate(new TooltipDelegate(pTableView));
    connect(pTableView, &QTableView::activated, this, &QDBHandler::onItemActivated);
    connect(pTableView, &ArchiveItemTableView::archiveItemTableViewCurrentChanged, this, &QDBHandler::onCurrentChanged);
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

QString QDBHandler::getSGFName(int index)
{
    return m_model->getSGFName(index);
}

QString QDBHandler::getCurrentSGFName()
{
    return m_model->getSGFName(m_currentIndex);
}

bool QDBHandler::hasSGF()
{
    return m_model->rowCount() != 0;
}

void QDBHandler::onItemActivated(const QModelIndex &index)
{
    getSGFContent(index.row());
    emit itemActivated();
}

void QDBHandler::onCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    m_currentIndex = current.row();
    getSGFContent(current.row());
    emit currentItemChanged();
}