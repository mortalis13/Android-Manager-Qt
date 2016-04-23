#include "copyprogress.h"
#include <QDebug>

CopyProgress::CopyProgress(QObject *parent) :
    QThread(parent)
{
}

void CopyProgress::run()
{
    fileSize = 0;
    prevFileSize = 0;
    QFile receiveFile;

    while (maxSize > fileSize)
    {
        receiveFile.setFileName(this->filePath);
        fileSize = receiveFile.size();
        emit progressValue(fileSize, maxSize);
        msleep(100);
    }
}
