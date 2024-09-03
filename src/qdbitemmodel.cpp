#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTextCodec>
#include <QUuid>

#include "qdbitemmodel.h"
#include "encodingutils.h"

QDBItemModel::QDBItemModel(QObject *parent) : QAbstractTableModel {parent} {}

QDBItemModel::~QDBItemModel()
{
    auto db = QSqlDatabase::database(m_connectionName);
    if (db.isOpen())
    {
        db.close();
    }
    QSqlDatabase::removeDatabase(m_connectionName);
}

void QDBItemModel::loadQDB(const QString &filename)
{
    m_connectionName = QUuid::createUuid().toString();
    auto db          = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    db.setDatabaseName(filename);
    if (!db.open())
    {
        return;
    }
    QString   sql = QStringLiteral("SELECT COUNT(*) FROM go_games;");
    QSqlQuery query(sql, db);
    if (query.next())
    {
        m_rowCount = query.value(0).toInt();
        beginInsertRows(QModelIndex(), 0, m_rowCount - 1);
        endInsertRows();
    }
}

int QDBItemModel::rowCount(const QModelIndex &parent) const
{
    return m_rowCount;
}

int QDBItemModel::columnCount(const QModelIndex &parent) const
{
    return 11;
}

QModelIndex QDBItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
    {
        return {};
    }

    return createIndex(row, column);
}

QVariant QDBItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    if (index.row() < 0 || index.row() >= m_rowCount)
    {
        return {};
    }
    if (index.column() < 0)
    {
        return {};
    }
    if (role == Qt::DisplayRole)
    {
        auto      db  = QSqlDatabase::database(m_connectionName);
        QString   sql = QStringLiteral("SELECT * FROM go_games WHERE ID=%1;").arg(index.row() + 1);
        QSqlQuery query(sql, db);
        if (!query.next())
        {
            return {};
        }
        switch (index.column())
        {
        case 0:
            return query.value(query.record().indexOf("ID")).toInt();
        case 1:
            return query.value(query.record().indexOf("EVENTNAME")).toString();
        case 2:
            return query.value(query.record().indexOf("GAMENAME")).toString();
        case 3:
            return query.value(query.record().indexOf("BLACKNAME")).toString();
        case 4:
            return query.value(query.record().indexOf("BLACKRANK")).toString();
        case 5:
            return query.value(query.record().indexOf("WHITENAME")).toString();
        case 6:
            return query.value(query.record().indexOf("WHITERANK")).toString();
        case 7:
            return query.value(query.record().indexOf("GAMERESULT")).toString();
        case 8:
            return query.value(query.record().indexOf("KOMI")).toString();
        case 9:
            return query.value(query.record().indexOf("MOVECOUNT")).toInt();
        case 10:
            return query.value(query.record().indexOf("COMMENTED")).toBool() ? tr("Yes") : tr("No");
        }
    }
    return {};
}

Qt::ItemFlags QDBItemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }

    return QAbstractItemModel::flags(index);
}

QVariant QDBItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {
            switch (section)
            {
            case 0:
                return tr("ID");
            case 1:
                return tr("Event Name");
            case 2:
                return tr("Game Name");
            case 3:
                return tr("Black Player");
            case 4:
                return tr("Black Rank");
            case 5:
                return tr("White Player");
            case 6:
                return tr("White Rank");
            case 7:
                return tr("Result");
            case 8:
                return tr("Komi");
            case 9:
                return tr("Move Count");
            case 10:
                return tr("Commented");
            }
        }
    }
    return {};
}

QByteArray QDBItemModel::getSGFContent(int index)
{
    auto      db  = QSqlDatabase::database(m_connectionName);
    QString   sql = QStringLiteral("SELECT GAMEDATA FROM go_games WHERE ID=%1;").arg(index + 1);
    QSqlQuery query(sql, db);
    if (query.next())
    {
        auto content = query.value(query.record().indexOf("GAMEDATA")).toByteArray();
        // convert from GBK to UTF-8, SGF from sdb is encoded in GBK
        auto [fromConverter, toConverter] = EncodingUtils::prepareConverters(QStringLiteral("GBK"), QStringLiteral("UTF-8"));
        auto [res, _]                     = EncodingUtils::convertDataEncoding(content, fromConverter, toConverter);
        return res;
    }
    return {};
}

QString QDBItemModel::getSGFName(int index)
{
    auto      db  = QSqlDatabase::database(m_connectionName);
    QString   sql = QStringLiteral("SELECT * FROM go_games WHERE ID=%1;").arg(index + 1);
    QSqlQuery query(sql, db);
    if (query.next())
    {
        auto eventName = query.value(query.record().indexOf("EVENTNAME")).toString();
        auto gameName  = query.value(query.record().indexOf("GAMENAME")).toString();
        if (!eventName.isEmpty() || !gameName.isEmpty())
            return QStringLiteral("%1 - %2").arg(eventName, gameName);
        auto blackName = query.value(query.record().indexOf("BLACKNAME")).toString();
        auto whiteName = query.value(query.record().indexOf("WHITENAME")).toString();
        if (!blackName.isEmpty() || !whiteName.isEmpty())
            return QStringLiteral("%1 vs %2").arg(blackName, whiteName);
    }

    return {};
}
