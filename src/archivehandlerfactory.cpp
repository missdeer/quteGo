#include <QFileInfo>

#include "archivehandlerfactory.h"

QMap<QString, ArchiveHandlerType> ArchiveHandlerFactory::m_handlers;
QStringList                       ArchiveHandlerFactory::m_nameFilters;
QStringList                       ArchiveHandlerFactory::m_extensions;

bool ArchiveHandlerFactory::registerArchiveHandler(const QString &ext, const QString &nameFilter, ArchiveHandlerType handler)
{
    m_handlers[ext] = handler;
    m_nameFilters.append(nameFilter);
    m_extensions.append(ext);
    return true;
}

bool ArchiveHandlerFactory::registerArchiveHandler(const QStringList &extensions, const QStringList &nameFilters, ArchiveHandlerType handler)
{
    for (const auto &ext : extensions)
    {
        m_handlers[ext] = handler;
    }

    m_nameFilters.append(nameFilters);
    m_extensions.append(extensions);
    return true;
}

ArchiveHandler *ArchiveHandlerFactory::createArchiveHandler(const QString &archive)
{
    QFileInfo fi(archive);
    if (!fi.exists())
        return nullptr;
    auto iter = m_handlers.find(fi.suffix().toLower());
    if (iter != m_handlers.end())
    {
        auto handler = iter.value();
        return handler(archive);
    }
    return nullptr;
}

QStringList ArchiveHandlerFactory::nameFilters()
{
    return m_nameFilters;
}

QStringList ArchiveHandlerFactory::extensions()
{
    return m_extensions;
}
