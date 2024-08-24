#ifndef ARCHIVEHANDLERFACTORY_H
#define ARCHIVEHANDLERFACTORY_H

#include "archivehandler.h"

using ArchiveHandlerType = ArchiveHandler *(*)(const QString &);

class ArchiveHandlerFactory
{
public:
    static ArchiveHandler *createArchiveHandler(const QString &archive);
    static bool            registerArchiveHandler(const QString &ext, ArchiveHandlerType handler);

private:
    static QMap<QString, ArchiveHandlerType> m_handlers;
};

#endif // ARCHIVEHANDLERFACTORY_H
