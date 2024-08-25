#include <QFileInfo>

#include "archivehandlerfactory.h"

QMap<QString, ArchiveHandlerType> ArchiveHandlerFactory::m_handlers;

bool ArchiveHandlerFactory::registerArchiveHandler(const QString &ext, ArchiveHandlerType handler)
{
    m_handlers[ext] = handler;
    return true;
}

bool ArchiveHandlerFactory::registerArchiveHandler(const QStringList &extensions, ArchiveHandlerType handler)
{
    for (const auto &ext : extensions)
    {
        m_handlers[ext] = handler;
    }
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
