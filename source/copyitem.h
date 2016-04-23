#ifndef COPYITEM_H
#define COPYITEM_H

#include <QString>

class CopyItem
{
public:
    CopyItem();

    QString command;
    QString filePath;
    QString origName;
    int fileSize;
};

#endif // COPYITEM_H
