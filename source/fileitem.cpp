#include "fileitem.h"

FileItem::FileItem()
{
}

bool FileItem::operator==(const FileItem &other)
{
    return this->fileName == other.fileName;
}
