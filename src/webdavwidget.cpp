#include <utility>

#include <QFileDialog>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QTableWidget>
#include <QToolBar>
#include <QUrl>
#include <QVBoxLayout>
#include <qwebdavitem.h>

#include "webdavwidget.h"
#include "networkreplyhelper.h"
#include "setting.h"

const QString SETTINGS_WEBDAV_SERVERRUL = "WEBDAV_SERVER_URL";
const QString SETTINGS_WEBDAV_USERNAME  = "WEBDAV_USERNAME";
const QString SETTINGS_WEBDAV_PASSWORD  = "WEBDAV_PASSWORD";

const QString SETTINGS_WEBDAV_AUTOCONNECT          = "WEBDAV_AUTO_CONNECT";
const QString SETTINGS_WEBDAV_REMEMBERLASTUSEDPATH = "WEBDAV_REMEMBER_LAST_USED_PATH";
const QString SETTINGS_WEBDAV_LASTPATH             = "WEBDAV_LAST_USED_PATH";

WebDavWidget::WebDavWidget(QWidget *parent) : QWidget(parent), m_tableWidget(new QTableWidget(0, 2, this))
{
    auto *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto *toolbar = new QToolBar(this);
    toolbar->setIconSize(QSize(22, 22));

    m_connectAction = toolbar->addAction(QIcon(QStringLiteral(":/rc/icons/connect.png")), tr("Connect"));
    connect(m_connectAction, &QAction::triggered, this, &WebDavWidget::onConnectActionTriggered);

    m_newDocumentAction = toolbar->addAction(QIcon(QStringLiteral(":/rc/icons/file-new.png")), tr("New Document"));
    m_newDocumentAction->setEnabled(false);
    connect(m_newDocumentAction, &QAction::triggered, this, &WebDavWidget::onNewDocumentActionTriggered);

    m_newDirectoryAction = toolbar->addAction(QIcon(QStringLiteral(":/rc/icons/new-dir.png")), tr("New Directory"));
    m_newDirectoryAction->setEnabled(false);
    connect(m_newDirectoryAction, &QAction::triggered, this, &WebDavWidget::onNewDirectoryActionTriggered);

    m_renameAction = toolbar->addAction(QIcon(QStringLiteral(":/rc/icons/rename.png")), tr("Rename"));
    m_renameAction->setEnabled(false);
    connect(m_renameAction, &QAction::triggered, this, &WebDavWidget::onRenameActionTriggered);

    m_removeAction = toolbar->addAction(QIcon(QStringLiteral(":/rc/icons/remove.png")), tr("Remove"));
    m_removeAction->setEnabled(false);
    connect(m_removeAction, &QAction::triggered, this, &WebDavWidget::onRemoveActionTriggered);

    m_uploadAction = toolbar->addAction(QIcon(QStringLiteral(":/rc/icons/upload.png")), tr("Upload Document"));
    m_uploadAction->setEnabled(false);
    connect(m_uploadAction, &QAction::triggered, this, &WebDavWidget::onUploadActionTriggered);

    m_refreshAction = toolbar->addAction(QIcon(QStringLiteral(":/rc/icons/refresh.png")), tr("Refresh"));
    connect(m_refreshAction, &QAction::triggered, this, &WebDavWidget::onRefreshActionTriggered);
    m_refreshAction->setEnabled(false);

    m_upLevelAction = toolbar->addAction(QIcon(QStringLiteral(":/rc/icons/up.png")), tr("Goto Parent Directory"));
    connect(m_upLevelAction, &QAction::triggered, this, &WebDavWidget::onUpLevelActionTriggered);
    m_upLevelAction->setEnabled(false);

    m_disconnectAction = toolbar->addAction(QIcon(QStringLiteral(":/rc/icons/disconnect.png")), tr("Disconnect"));
    connect(m_disconnectAction, &QAction::triggered, this, &WebDavWidget::onDisconnectActionTriggered);
    m_disconnectAction->setEnabled(false);

    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_tableWidget->setColumnWidth(0, 300);
    m_tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Name")));
    m_tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Size")));
    m_tableWidget->verticalHeader()->setVisible(false);
    m_tableWidget->setShowGrid(false);

    mainLayout->addWidget(toolbar);
    mainLayout->addWidget(m_tableWidget);

    setLayout(mainLayout);

    connect(m_tableWidget, &QTableWidget::itemSelectionChanged, this, &WebDavWidget::onItemSelectionChanged);
    connect(m_tableWidget, &QTableWidget::itemActivated, this, &WebDavWidget::onTableWidgetItemActivited);
    connect(&m_webdavDirParser, &QWebdavDirParser::finished, this, &WebDavWidget::onWebDavDirParserFinished);
    connect(&m_webdavDirParser, &QWebdavDirParser::errorChanged, this, &WebDavWidget::onError);
    connect(&m_webdav, &QWebdav::errorChanged, this, &WebDavWidget::onError);
}

bool WebDavWidget::onStart()
{
    bool bAutoConnect = g_setting->readBoolEntry(SETTINGS_WEBDAV_AUTOCONNECT);
    m_serverUrl       = g_setting->readEntry(SETTINGS_WEBDAV_SERVERRUL);

    if (bAutoConnect)
    {
        doConnect();
        return true;
    }
    return false;
}

void WebDavWidget::onConnectActionTriggered()
{
    m_serverUrl = g_setting->readEntry(SETTINGS_WEBDAV_SERVERRUL);

    if (m_serverUrl.isEmpty())
    {
        QMessageBox::warning(this, tr("Warning"), tr("No valid WebDav server information, please set it before connecting."));
        return;
    }

    doConnect();
}

void WebDavWidget::doConnect()
{
    m_serverUrl        = g_setting->readEntry(SETTINGS_WEBDAV_SERVERRUL);
    auto username      = g_setting->readEntry(SETTINGS_WEBDAV_USERNAME);
    auto password      = g_setting->readEntry(SETTINGS_WEBDAV_PASSWORD);
    bool bRememberPath = g_setting->readBoolEntry(SETTINGS_WEBDAV_REMEMBERLASTUSEDPATH);

    if (m_serverUrl.isEmpty())
    {
        QMessageBox::warning(this, tr("Warning"), tr("No valid WebDav server information, please set it before connecting."));
        return;
    }

    QUrl url(m_serverUrl);
    m_webdav.setConnectionSettings(
        url.scheme() == "https" ? QWebdav::HTTPS : QWebdav::HTTP, url.host(), QStringLiteral("/"), username, password, url.port());
    if (!bRememberPath || m_currentPath.isEmpty())
    {
        m_currentPath = url.path();
        if (!m_currentPath.endsWith('/'))
        {
            m_currentPath.append('/');
        }
    }
    qDebug() << "list directory:" << m_currentPath;
    m_webdavDirParser.listDirectory(&m_webdav, m_currentPath);
}

void WebDavWidget::saveLastPathToSettings()
{
    if (!m_currentPath.isEmpty())
    {
        g_setting->writeEntry(SETTINGS_WEBDAV_LASTPATH, m_currentPath);
    }
}

void WebDavWidget::onDisconnectActionTriggered()
{
    Q_ASSERT(m_tableWidget);
    m_tableWidget->clearContents();
    m_tableWidget->setRowCount(0);
    m_connectAction->setEnabled(true);
    m_refreshAction->setEnabled(false);
    m_upLevelAction->setEnabled(false);
    m_disconnectAction->setEnabled(false);
    m_newDocumentAction->setEnabled(false);
    m_newDirectoryAction->setEnabled(false);
    m_uploadAction->setEnabled(false);
    m_renameAction->setEnabled(false);
    m_removeAction->setEnabled(false);
    m_isConnected = false;
}

void WebDavWidget::onRefreshActionTriggered()
{
    qDebug() << "list directory:" << m_currentPath;
    m_webdavDirParser.listDirectory(&m_webdav, m_currentPath);
}

void WebDavWidget::onUpLevelActionTriggered()
{
    if (m_currentPath == QLatin1String("/"))
    {
        return;
    }
    if (!m_currentPath.endsWith('/'))
    {
        return;
    }
    m_currentPath.remove(m_currentPath.length() - 1, 1);
    int index = m_currentPath.lastIndexOf('/');
    m_currentPath.remove(index + 1, m_currentPath.length() - index);
    qDebug() << "list directory:" << m_currentPath;
    m_webdavDirParser.listDirectory(&m_webdav, m_currentPath);
    saveLastPathToSettings();
}

void WebDavWidget::onWebDavDirParserFinished()
{
    auto *parser = qobject_cast<QWebdavDirParser *>(sender());
    Q_ASSERT(parser);
    auto list = parser->getList();

    Q_ASSERT(m_tableWidget);
    m_tableWidget->clearContents();
    m_tableWidget->setRowCount(0);
    for (auto &item : list)
    {
        if (!item.isDir())
        {
            // only list .sgf/.zip/.rar/.7z files and directories
            if (!item.name().endsWith(".sgf", Qt::CaseInsensitive) && !item.name().endsWith(".zip", Qt::CaseInsensitive) &&
                !item.name().endsWith(".rar", Qt::CaseInsensitive) && !item.name().endsWith(".7z", Qt::CaseInsensitive))
            {
                continue;
            }
        }
        int row = m_tableWidget->rowCount();
        m_tableWidget->insertRow(row);

        auto *nameItem = new QTableWidgetItem(item.name());
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        nameItem->setIcon(QIcon(item.isDir() ? ":/rc/icons/file-open.png" : ":/rc/icons/file-new.png"));
        m_tableWidget->setItem(row, 0, nameItem);
        if (!item.isDir())
        {
            auto *sizeItem = new QTableWidgetItem(QString::number(item.size()));
            sizeItem->setFlags(sizeItem->flags() & ~Qt::ItemIsEditable);
            sizeItem->setTextAlignment(Qt::AlignRight);
            m_tableWidget->setItem(row, 1, sizeItem);
        }
    }
    m_connectAction->setEnabled(false);
    m_refreshAction->setEnabled(true);
    m_upLevelAction->setEnabled(m_currentPath != QLatin1String("/"));
    m_disconnectAction->setEnabled(true);
    m_newDocumentAction->setEnabled(true);
    m_newDirectoryAction->setEnabled(true);
    m_uploadAction->setEnabled(true);
    m_renameAction->setEnabled(!m_tableWidget->selectedItems().empty());
    m_removeAction->setEnabled(!m_tableWidget->selectedItems().empty());
    m_isConnected = true;
}

void WebDavWidget::onError(const QString &err)
{
    qDebug() << "WebDav error message: " << err;
    emit error(err);
    m_isConnected = false;
}

void WebDavWidget::onTableWidgetItemActivited(QTableWidgetItem *item)
{
    Q_ASSERT(item);
    Q_ASSERT(m_tableWidget);
    auto *nameItem = m_tableWidget->item(item->row(), 0);
    Q_ASSERT(nameItem);
    auto name = nameItem->text();
    if (isDir(item))
    {
        // is Dir, list this directory
        m_currentPath.append(name);
        m_currentPath.append('/');
        qDebug() << "list directory:" << m_currentPath;
        m_webdavDirParser.listDirectory(&m_webdav, m_currentPath);
        saveLastPathToSettings();
    }
    else
    {
        //  is file, get this file
        auto  filePath = QStringLiteral("%1%2").arg(m_currentPath, name);
        auto *reply    = new NetworkReplyHelper(m_webdav.get(filePath));

        connect(reply, &NetworkReplyHelper::done, this, &WebDavWidget::onRequestWebDavFileFinished);
        connect(reply, &NetworkReplyHelper::errorMessage, this, &WebDavWidget::onNetworkError);
    }
}

void WebDavWidget::onRequestWebDavFileFinished()
{
    auto *reply = qobject_cast<NetworkReplyHelper *>(sender());
    Q_ASSERT(reply);
    reply->deleteLater();

    Q_ASSERT(m_tableWidget);
    auto *item = m_tableWidget->currentItem();
    Q_ASSERT(item);
    auto *nameItem = m_tableWidget->item(item->row(), 0);
    Q_ASSERT(nameItem);
    auto name     = nameItem->text();
    auto filePath = QStringLiteral("%1%2%3").arg(m_serverUrl, m_currentPath, name);
    auto content  = reply->content();
    emit retrievedWebDavFile(filePath, content);
}

void WebDavWidget::onSaveDocument(const QString &fullUrl, const QByteArray &content)
{
    QUrl url(fullUrl);

    auto *reply = new NetworkReplyHelper(m_webdav.put(url.path(), content));
    connect(reply, &NetworkReplyHelper::done, this, &WebDavWidget::onPutWebDavFileFinished);
    connect(reply, &NetworkReplyHelper::errorMessage, this, &WebDavWidget::onNetworkError);
}

void WebDavWidget::onPutWebDavFileFinished()
{
    auto *reply = qobject_cast<NetworkReplyHelper *>(sender());
    Q_ASSERT(reply);
    reply->deleteLater();

    emit storedWebDavFile(reply->reply()->request().url().toString());

    QTimer::singleShot(0, this, &WebDavWidget::onRefreshActionTriggered);
}

void WebDavWidget::onNetworkError(QNetworkReply::NetworkError err, QString errMsg)
{
    qDebug() << "WebDav error message: " << err;
    emit error(std::move(errMsg));
}

void WebDavWidget::onItemSelectionChanged()
{
    m_renameAction->setEnabled(hasSelected());
    m_removeAction->setEnabled(hasSelected());
}

void WebDavWidget::onNewDocumentActionTriggered()
{
    auto fileName = QInputDialog::getText(this, tr("New document"), tr("Please input new document name"));
    if (fileName.isEmpty())
    {
        return;
    }
    auto *reply = new NetworkReplyHelper(m_webdav.put(m_currentPath + fileName, ""));
    connect(reply, &NetworkReplyHelper::done, this, &WebDavWidget::onNewWebDavFileFinished);
    connect(reply, &NetworkReplyHelper::errorMessage, this, &WebDavWidget::onNetworkError);
}

void WebDavWidget::onNewDirectoryActionTriggered()
{
    auto dirName = QInputDialog::getText(this, tr("New directory"), tr("Please input new directory name"));
    if (dirName.isEmpty())
    {
        return;
    }
    auto *reply = new NetworkReplyHelper(m_webdav.mkdir(m_currentPath + dirName));
    connect(reply, &NetworkReplyHelper::done, this, &WebDavWidget::onNewWebDavDirectoryFinished);
    connect(reply, &NetworkReplyHelper::errorMessage, this, &WebDavWidget::onNetworkError);
}

void WebDavWidget::onRenameActionTriggered()
{
    auto oldName = getSelectedName();
    auto newName = QInputDialog::getText(this, tr("Renaming %1 to...").arg(oldName), tr("Please input new name"), QLineEdit::Normal, oldName);
    if (newName.isEmpty() || oldName == newName)
    {
        return;
    }
    auto *reply = new NetworkReplyHelper(m_webdav.move(m_currentPath + oldName, m_currentPath + newName));
    connect(reply, &NetworkReplyHelper::done, this, &WebDavWidget::onRenameWebDavItemFinished);
    connect(reply, &NetworkReplyHelper::errorMessage, this, &WebDavWidget::onNetworkError);
}

void WebDavWidget::uploadFile(const QString &filePath)
{
    auto *file = new QFile(filePath);
    if (!file->open(QIODevice::ReadOnly))
    {
        delete file;
        return;
    }
    QFileInfo fileInfo(filePath);
    auto     *reply = new NetworkReplyHelper(m_webdav.put(m_currentPath + fileInfo.fileName(), file));
    reply->setData((void *)file);
    connect(reply, &NetworkReplyHelper::done, this, &WebDavWidget::onUploadWebDavFileFinished);
    connect(reply, &NetworkReplyHelper::errorMessage, this, &WebDavWidget::onNetworkError);
}

void WebDavWidget::uploadData(const QString &fileName, const QByteArray &content)
{
    onSaveDocument(m_currentPath + fileName, content);
}

void WebDavWidget::onUploadActionTriggered()
{
    auto fileNames = QFileDialog::getOpenFileNames(this, tr("Select files to upload"));
    for (const auto &fileName : fileNames)
    {
        uploadFile(fileName);
    }
}

void WebDavWidget::onRemoveActionTriggered()
{
    int res = QMessageBox::question(
        this, tr("Confirm"), tr("Do you want to remove file %1 ? It can't be recovered.").arg(getSelectedName()), QMessageBox::Yes | QMessageBox::No);
    if (res == QMessageBox::Yes)
    {
        auto *reply = new NetworkReplyHelper(m_webdav.remove(getSelectedPath()));
        connect(reply, &NetworkReplyHelper::done, this, &WebDavWidget::onRemoveWebDavFileFinished);
        connect(reply, &NetworkReplyHelper::errorMessage, this, &WebDavWidget::onNetworkError);
    }
}

bool WebDavWidget::hasSelected() const
{
    Q_ASSERT(m_tableWidget);
    return !m_tableWidget->selectedItems().empty();
}

bool WebDavWidget::isSelectedDir() const
{
    Q_ASSERT(m_tableWidget);
    auto *item = m_tableWidget->currentItem();
    Q_ASSERT(item);
    return isDir(item);
}

QString WebDavWidget::getSelectedPath() const
{
    return m_currentPath + getSelectedName();
}

QString WebDavWidget::getSelectedName() const
{
    Q_ASSERT(m_tableWidget);
    auto *item = m_tableWidget->currentItem();
    Q_ASSERT(item);
    auto *nameItem = m_tableWidget->item(item->row(), 0);
    Q_ASSERT(nameItem);
    return nameItem->text();
}

bool WebDavWidget::isDir(QTableWidgetItem *item) const
{
    Q_ASSERT(m_tableWidget);
    Q_ASSERT(item);
    return !m_tableWidget->item(item->row(), 1);
}

void WebDavWidget::onRemoveWebDavFileFinished()
{
    auto *reply = qobject_cast<NetworkReplyHelper *>(sender());
    Q_ASSERT(reply);
    reply->deleteLater();

    QMessageBox::information(
        this, tr("Notice"), tr("File %1 has been removed.").arg(QFileInfo(reply->reply()->request().url().path()).fileName()), QMessageBox::Ok);

    QTimer::singleShot(0, this, &WebDavWidget::onRefreshActionTriggered);
}

void WebDavWidget::onNewWebDavDirectoryFinished()
{
    auto *reply = qobject_cast<NetworkReplyHelper *>(sender());
    Q_ASSERT(reply);
    reply->deleteLater();

    QTimer::singleShot(0, this, &WebDavWidget::onRefreshActionTriggered);
}

void WebDavWidget::onRenameWebDavItemFinished()
{
    auto *reply = qobject_cast<NetworkReplyHelper *>(sender());
    Q_ASSERT(reply);
    reply->deleteLater();

    QTimer::singleShot(0, this, &WebDavWidget::onRefreshActionTriggered);
}

void WebDavWidget::onUploadWebDavFileFinished()
{
    auto *reply = qobject_cast<NetworkReplyHelper *>(sender());
    Q_ASSERT(reply);
    reply->deleteLater();

    auto *file = static_cast<QFile *>(reply->rawDataPointer());
    file->close();
    file->deleteLater();

    QTimer::singleShot(0, this, &WebDavWidget::onRefreshActionTriggered);
}

void WebDavWidget::onNewWebDavFileFinished()
{
    auto *reply = qobject_cast<NetworkReplyHelper *>(sender());
    Q_ASSERT(reply);
    reply->deleteLater();

    QTimer::singleShot(0, this, &WebDavWidget::onRefreshActionTriggered);
}

bool WebDavWidget::isConnected() const
{
    return m_isConnected;
}
const QString &WebDavWidget::currentPath() const
{
    return m_currentPath;
}
