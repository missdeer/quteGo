#include <QCoreApplication>
#include <QDir>
#include <QListWidget>
#include <QProcess>
#include <QVBoxLayout>

#include "sdbhandler.h"
#include "archivehandlerfactory.h"

namespace
{
    bool bRes1 = ArchiveHandlerFactory::registerArchiveHandler(QStringLiteral("sdb"),
                                                               QObject::tr("Stonebase database files (*.sdb)"),
                                                               [](const QString &archive) -> ArchiveHandler * { return new SDBHandler(archive); });
} // namespace

SDBHandler::SDBHandler(const QString &archive) : QDBHandler()
{
    // convert sdb to qdb
    QString sdb = QDir::toNativeSeparators(archive);
    QString qdb = sdb;
    qdb.replace(sdb.length() - 4, 4, QStringLiteral(".qdb"));
    QString sdb2qdb = QCoreApplication::applicationDirPath() + QStringLiteral("/sdb2qdb.exe");
    QProcess::execute(sdb2qdb, {QStringLiteral("--convert=") + qdb, sdb});
    // read qdb
    readDB(qdb);
}
