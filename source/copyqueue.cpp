#include "copyqueue.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

#include <QDateTime>


CopyQueue::CopyQueue(QObject *parent) :
    QObject(parent)
{
    queue = new QList<CopyItem>();
    current_id = 0;
}

void CopyQueue::add(QString command, int fileSize, QString filePath, QString origName) {
    CopyItem item;
    item.command = command;
    item.fileSize = fileSize;
    item.filePath = filePath;
    item.origName = origName;

    queue->append(item);
}

void CopyQueue::startCopy(){
    copyNext();
}

void CopyQueue::copyNext() {
    if(current_id == queue->length()){
      // qDebug() << "all-finished";
      return;
    }
    
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
    
    copyProgress = new CopyProgress();
    connect(copyProgress, SIGNAL(progressValue(int, int)), this, SLOT(setCopyProgressValue(int, int)));
    connect(copyProgress, SIGNAL(finished()), this, SLOT(finishedProgress()));

    currentItem = &queue->at(current_id);
    
    QString command = currentItem->command;
    int fileSize = currentItem->fileSize;
    QString filePath = currentItem->filePath;
    // qDebug() << "next:" << command;

    proc = new QProcess(this);
    connect(proc, SIGNAL(finished(int)), this, SLOT(copyProcFinished(int)));
    proc->start(command);
    
    copyProgress->maxSize = fileSize;
    copyProgress->filePath = filePath;
    copyProgress->start();
    
    timer->start(1000);
}


void CopyQueue::copyProcFinished(int status){
    // qDebug() << "finished-next";
//    renameCopiedFile(currentItem->filePath, currentItem->origName);

    current_id++;
    
    emit finishedNextCopy();
    timer->stop();
    copyNext();
}


void CopyQueue::renameCopiedFile(QString filePath, QString origName){
  QString newFilePath, dirPath;
  
  QFileInfo info(filePath);
  dirPath = info.absolutePath();
  newFilePath = dirPath + "/" + origName;
  
  QFile copiedFile(filePath);
  copiedFile.rename(newFilePath);
}

void CopyQueue::finishedProgress(){
    // qDebug() << "finish-progress";
    renameCopiedFile(currentItem->filePath, currentItem->origName);
}


void CopyQueue::setCopyProgressValue(int val, int max){
    emit progressValue(val, max);
}

void CopyQueue::timerUpdate(){
    emit timerTimeout();
}
