#ifndef SEVENZARCHIVEHANDLER_H
#define SEVENZARCHIVEHANDLER_H

#include "archivehandler.h"
#include "thirdparty/Qt7z/Qt7z/qt7zpackage.h"

class SevenZArchiveHandler : public ArchiveHandler
{
public:
    explicit SevenZArchiveHandler(const QString &archive);
    ~SevenZArchiveHandler();
    const QStringList &getSGFFileList();
    QIODevice         *getSGFContent(const QString &fileName);

private:
    Qt7zPackage m_pkg;
    QStringList m_fileList;
    QBuffer     m_buffer;
};

#endif // SEVENZARCHIVEHANDLER_H
