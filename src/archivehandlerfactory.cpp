#include <QFileInfo>

#include "archivehandlerfactory.h"
#include "rararchivehandler.h"
#include "sevenzarchivehandler.h"
#include "ziparchivehandler.h"

ArchiveHandlerFactory::ArchiveHandlerFactory() {}

ArchiveHandler *ArchiveHandlerFactory::createArchiveHandler(const QString &archive)
{
    QFileInfo fi(archive);
    if (fi.suffix().compare("zip", Qt::CaseInsensitive) == 0)
        return new ZipArchiveHandler(archive);
    else if (fi.suffix().compare("rar", Qt::CaseInsensitive) == 0)
        return new RarArchiveHandler(archive);
    else if (fi.suffix().compare("7z", Qt::CaseInsensitive) == 0)
        return new SevenZArchiveHandler(archive);

    return nullptr;
}
