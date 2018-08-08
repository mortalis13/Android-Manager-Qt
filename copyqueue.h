#ifndef COPYQUEUE_H
#define COPYQUEUE_H

#include <QObject>

#include <QProcess>
#include <QDebug>
#include <QTimer>

#include "copyitem.h"
#include "copyprogress.h"


class CopyQueue : public QObject
{
  Q_OBJECT
public:
  explicit CopyQueue(QObject *parent = 0);

  QList<CopyItem> *queue;
  CopyProgress *copyProgress;
  QTimer *timer;
  QProcess *proc;

  const CopyItem *currentItem;

  int current_id;

  void add(QString command, int fileSize, QString filePath, QString origName);
  void copyNext();
  void startCopy();
  void renameCopiedFile(QString filePath, QString origName);

  int count() {
    return queue->count();
  }


signals:
  void progressValue(int val, int max);
  void timerTimeout();
  void finishedNextCopy();
  void copyQueueFinished();

private slots:
  void copyProcFinished(int status);
  void finishedProgress();
  void timerUpdate();
  void setCopyProgressValue(int val, int max);

};

#endif // COPYQUEUE_H
