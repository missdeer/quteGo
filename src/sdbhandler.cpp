#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>

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
    QString   sdb = QDir::toNativeSeparators(archive);
    QFileInfo fi(sdb);
    QString   dirPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/quteGo";
    QDir      dir(dirPath);
    if (!dir.exists())
        dir.mkpath(dirPath);
    QString qdb     = dir.absoluteFilePath(fi.baseName() + ".qdb");
    QString sdb2qdb = QCoreApplication::applicationDirPath() + QStringLiteral("/sdb2qdb.exe");
    QProcess::execute(sdb2qdb, {QStringLiteral("--convert=") + qdb, sdb});
    // read qdb
    readDB(qdb);
}
