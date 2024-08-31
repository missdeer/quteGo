#ifndef ARCHIVEHANDLERFACTORY_H
#define ARCHIVEHANDLERFACTORY_H

#include "archivehandler.h"

using ArchiveHandlerType = ArchiveHandler *(*)(const QString &);

class ArchiveHandlerFactory
{
public:
    static ArchiveHandler *createArchiveHandler(const QString &archive);
    static bool            registerArchiveHandler(const QString &ext, const QString &nameFilter, ArchiveHandlerType handler);
    static bool            registerArchiveHandler(const QStringList &extensions, const QStringList &nameFilters, ArchiveHandlerType handler);
    static QStringList     nameFilters();
    static QStringList     extensions();

private:
    static QMap<QString, ArchiveHandlerType> m_handlers;
    static QStringList                       m_nameFilters;
    static QStringList                       m_extensions;
};

#endif // ARCHIVEHANDLERFACTORY_H
