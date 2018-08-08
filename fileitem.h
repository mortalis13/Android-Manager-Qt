#ifndef FILEITEM_H
#define FILEITEM_H

#include <QObject>
#include <QIcon>

class FileItem{

public:
  FileItem();

  QIcon fileIcon;
  QString fileName;
  QString attributes;
  int fileSize;
  QString fileDate;
  QString filePath;
  QString filePermissions;
  QString fileOwner;
  QString fileType;
  QColor fileColor;


public:
  bool operator==(const FileItem &other);

};

#endif // FILEITEM_H
