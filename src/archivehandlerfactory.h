#ifndef ARCHIVEHANDLERFACTORY_H
#define ARCHIVEHANDLERFACTORY_H

#include "archivehandler.h"

class ArchiveHandlerFactory
{
public:
    ArchiveHandlerFactory();
    static ArchiveHandler *createArchiveHandler(const QString &archive);
};

#endif // ARCHIVEHANDLERFACTORY_H
