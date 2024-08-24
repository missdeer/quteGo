#include <QFileInfo>

#include "archivehandlerfactory.h"
#if defined(Q_OS_WIN)
#include "rararchivehandler.h"
#include "sevenzarchivehandler.h"
#endif
#include "ziparchivehandler.h"

ArchiveHandlerFactory::ArchiveHandlerFactory() {}

ArchiveHandler *ArchiveHandlerFactory::createArchiveHandler(const QString &archive)
{
    QFileInfo fi(archive);
    if (fi.suffix().compare("zip", Qt::CaseInsensitive) == 0)
        return new ZipArchiveHandler(archive);
    #if defined(Q_OS_WIN)
    else if (fi.suffix().compare("rar", Qt::CaseInsensitive) == 0)
        return new RarArchiveHandler(archive);
    else if (fi.suffix().compare("7z", Qt::CaseInsensitive) == 0)
        return new SevenZArchiveHandler(archive);
    #endif

    return nullptr;
}
