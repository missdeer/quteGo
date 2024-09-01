#ifndef ARCHIVEHANDLER_H
#define ARCHIVEHANDLER_H

#include <QIODevice>
#include <QSharedPointer>
#include <QStringList>

#include "archiveitemlistwidget.h"

class ArchiveHandler : public QObject
{
    Q_OBJECT
public:
    explicit ArchiveHandler(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~ArchiveHandler()                                             = default;
    virtual QIODevice             *getSGFContent(const QString &fileName) = 0;
    virtual QIODevice             *getSGFContent(int index)               = 0;
    virtual QIODevice             *getCurrentSGFContent()                 = 0;
    virtual ArchiveItemListWidget *getArchiveItemListWidget()             = 0;
    virtual bool                   hasSGF()                               = 0;
signals:
    void currentItemChanged();
    void itemActivated();
};

using ArchiveHandlerPtr = QSharedPointer<ArchiveHandler>;

#endif // ARCHIVEHANDLER_H
