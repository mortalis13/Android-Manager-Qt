#ifndef COPYPROGRESS_H
#define COPYPROGRESS_H

#include <QThread>
#include <QFile>


class CopyProgress : public QThread
{
  Q_OBJECT
public:
  explicit CopyProgress(QObject *parent = 0);

  void run();
  QString filePath;
  int maxSize;
  int fileSize;
  int prevFileSize;


signals:
  void progressValue(int fileSize, int maxSize);

public slots:


};

#endif // COPYPROGRESS_H
