#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QModelIndex>
#include <QProcess>
#include <QTextCodec>
#include <QStandardItemModel>
#include <QTableView>

#include "fileitem.h"
#include "copyqueue.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT


  const QString hHeaderStyle = "QHeaderView::section, QHeaderView::section:checked {background-color: #f1f1f1; border: 1px solid #ddd; padding: 4px;}";
  const QString vHeaderStyle = "QHeaderView::section, QHeaderView::section:checked {background-color: #f1f1f1; border: 1px solid #ddd; padding: 4px; min-width: 20px;}";


public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

  QString getPrevDir(QString dir);
  QString getCurrentDir(QString dir);

  QString formatSize(int speedi);
  QString formatPath(QString path);
  QString fixPullEncoding(QString item);

  QString getCodesFromBytes(QByteArray str);
  QString getCodesFromBytesHex(QByteArray bytes);
  QString getCodesFromString(QString str);


private:
  void init();
  void addActions();
  void addShortcuts();

  void updateStatus(int dirlen, int filelen);
  void goToDir(QString dir = "");

  QTableView* tableView();


private:
  Ui::MainWindow *ui;

  QProcess *proc;

  QString curDirPath;
  QString speedText;

  QList<FileItem> *dirsList;
  QList<FileItem> *filesList;

  QStandardItemModel *tableModel;

  int fileSize;
  int prevFileSize;
  int totalCopySize;
  int curCopyNum;
  int totalCopyNum;

  bool copyActive;


private slots:
  void bTestClick();

  void bGoClick();
  void bCopyClick();
  void listDoubelClick(const QModelIndex &index);
  void pathEnter();

  void goRoot();
  void goUp();
  void prevDir();
  void enterDir();

  void homeList();
  void endList();

  void copyProcFinished(int status);
  void finishedNextCopyAction();
  void copyQueueFinished();

  void setCopyProgressValue(int val, int max);
  void timerUpdate();

};

#endif // MAINWINDOW_H
