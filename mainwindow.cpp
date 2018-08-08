#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtDebug>
#include <QtAlgorithms>

#include <QStringListModel>
#include <QShortcut>
#include <QFileInfo>

#include "fun.h"


MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  init();
  addActions();
  addShortcuts();
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::init() {
  tableView()->setEditTriggers(QAbstractItemView::NoEditTriggers);

  ui->lProgress->setText("");
  ui->lStatus->setText("");
  setCopyProgressValue(0, 0);

  curDirPath = "";
  copyActive = true;

  dirsList = new QList<FileItem>();
  filesList = new QList<FileItem>();
  tableModel = new QStandardItemModel();
}

void MainWindow::addActions() {
  // connect( ui->bTest, SIGNAL(clicked()), this, SLOT(bTestClick()) );
  
  connect( ui->bGoRoot, SIGNAL(clicked()), this, SLOT(goRoot()) );
  connect( ui->bGoUp, SIGNAL(clicked()), this, SLOT(goUp()) );
  connect( ui->bCopy, SIGNAL(clicked()), this, SLOT(bCopyClick()) );
  connect( ui->bGo, SIGNAL(clicked()), this, SLOT(bGoClick()) );
  
  connect( ui->lePath, SIGNAL(returnPressed()), this, SLOT(pathEnter()) );
  connect( ui->tvList, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(listDoubelClick(const QModelIndex &)) );
}

void MainWindow::addShortcuts() {
  QShortcut *bGoRoot_Key = new QShortcut(QKeySequence("F1"), this);
  connect( bGoRoot_Key, SIGNAL(activated()), ui->bGoRoot, SLOT(click()) );

  QShortcut *bGo_Key = new QShortcut(QKeySequence("F4"), this);
  connect( bGo_Key, SIGNAL(activated()), ui->bGo, SLOT(click()) );

  QShortcut *quit_Key = new QShortcut(QKeySequence("Esc"), this);
  connect( quit_Key, SIGNAL(activated()), this, SLOT(close()) );

  QShortcut *bCopy_Key = new QShortcut(QKeySequence("F5"), this);
  connect( bCopy_Key, SIGNAL(activated()), ui->bCopy, SLOT(click()) );

  // QShortcut *bCopy_Key = new QShortcut(QKeySequence("F5"), tableView(), 0, 0, Qt::WidgetShortcut);
  // connect( bCopy_Key, SIGNAL(activated()), ui->bCopy, SLOT(click()) );


  // -- file table
  QShortcut *bGoUp_Key = new QShortcut(QKeySequence("Backspace"), tableView(), 0, 0, Qt::WidgetShortcut);
  connect( bGoUp_Key, SIGNAL(activated()), ui->bGoUp, SLOT(click()) );

  QShortcut *prevDir_Key = new QShortcut(QKeySequence("Left"), tableView(), 0, 0, Qt::WidgetShortcut);
  connect( prevDir_Key, SIGNAL(activated()), this, SLOT(prevDir()) );

  QAction *action = new QAction(this);
  QList<QKeySequence> shortcuts;
  shortcuts << QKeySequence("Right") << QKeySequence("Return") << QKeySequence("Enter");
  action->setShortcuts(shortcuts);
  action->setShortcutContext(Qt::WidgetShortcut);
  connect( action, SIGNAL(triggered()), this, SLOT(enterDir()) );
  tableView()->addAction(action);

  QShortcut *homeList_Key = new QShortcut(QKeySequence("Home"), tableView(), 0, 0, Qt::WidgetShortcut);
  connect( homeList_Key, SIGNAL(activated()), this, SLOT(homeList()) );

  QShortcut *endList_Key = new QShortcut(QKeySequence("End"), tableView(), 0, 0, Qt::WidgetShortcut);
  connect( endList_Key, SIGNAL(activated()), this, SLOT(endList()) );
}

bool compareInsensitive(const QString &s1, const QString &s2) {
  return s1.toLower() < s2.toLower();
}
bool compareInsensitiveItems(const FileItem &item1, const FileItem &item2) {
  return item1.fileName.toLower() < item2.fileName.toLower();
}


// ------------------------------------------------ actions ------------------------------------------------

void MainWindow::bGoClick() {
  pathEnter();
}

void MainWindow::bTestClick() {
}

void MainWindow::bCopyClick() {
  if (!tableView() || !tableView()->model() || !tableView()->selectionModel()) {
    return;
  }

  setCopyProgressValue(0, 0);
  copyActive = true;
  totalCopyNum = 0;
  curCopyNum = 0;
  totalCopySize = 0;
  fileSize = 0;
  prevFileSize = 0;
  speedText = "";

  CopyQueue *cq = new CopyQueue(this);
  connect( cq, SIGNAL(progressValue(int, int)), this, SLOT(setCopyProgressValue(int, int)) );
  connect( cq, SIGNAL(timerTimeout()), this, SLOT(timerUpdate()) );
  connect( cq, SIGNAL(finishedNextCopy()), this, SLOT(finishedNextCopyAction()) );
  connect( cq, SIGNAL(copyQueueFinished()), this, SLOT(copyQueueFinished()) );

  QModelIndexList list = tableView()->selectionModel()->selectedIndexes();
  totalCopyNum = list.length();
  if (totalCopyNum == 0) {
    qDebug("No files selected to copy");
    return;
  }

  QString toPath = ui->leToPath->text();
  toPath = formatPath(toPath);
  Fun::createDirs(toPath);

  foreach(const QModelIndex &index, list) {
    int row = index.row();
    int column = index.column();

    QModelIndex nameIndex = index.sibling(row, 0);
    QModelIndex attrsIndex = index.sibling(row, 2);

    if (!nameIndex.isValid()) {
      qFatal("Invalid Sibling Index (bCopyClick)");
      return;
    }

    QString name = nameIndex.data().toString();
    QString attrs = attrsIndex.data().toString();

    if (attrs.at(0) == 'd') continue;

    QString origName = name;
    QString convertedName = fixPullEncoding(origName);

    QString fromPath = curDirPath + convertedName;
    QString filePath = toPath + "/" + convertedName;

    QString command = "adb pull \"" + fromPath + "\" \"" + toPath + "/.\"";
    qDebug() << QString("Copy command: %1").arg(command);

    int itemIndex = index.row() - dirsList->length();
    int fileSize = filesList->at(itemIndex).fileSize;
    totalCopySize += fileSize;

    cq->add(command, fileSize, filePath, origName);
  }

  if (cq->count() > 0) {
    cq->startCopy();
  }

  tableView()->setFocus();
}

void MainWindow::listDoubelClick(const QModelIndex &index) {
  enterDir();
}

void MainWindow::pathEnter() {
  QString path = ui->lePath->text();
  // qDebug() << path;
  curDirPath = "";
  path.replace(QRegExp("/$"), "");
  goToDir(path);
  tableView()->setFocus();
}


// ------------------------------------------------ navigation ------------------------------------------------

void MainWindow::goRoot() {
  curDirPath = "";
  goToDir();
  tableView()->setFocus();
}

void MainWindow::goUp() {
  QString prevDir = getPrevDir(curDirPath);
  QString currentDirName = getCurrentDir(curDirPath);

  // qDebug() << "prevDir:" << prevDir;
  // qDebug() << "currentDirName:" << currentDirName;

  curDirPath = "";
  goToDir(prevDir);

  int row = 0;

  FileItem temp;
  temp.fileName = currentDirName;
  int res = dirsList->indexOf(temp);
  if (res != -1)
    row = res;

  // qDebug() << "row:" << row;

  QModelIndex newIndex = tableView()->model()->index(row, 0);
  tableView()->setCurrentIndex(newIndex);
  tableView()->scrollTo(newIndex);

  tableView()->setFocus();
}

void MainWindow::goToDir(QString dir) {
  if (dir.length() == 0)
    curDirPath = "";
  ui->teOutput->clear();

  proc = new QProcess(this);

  curDirPath = curDirPath + dir + "/";
  QString command = "adb shell \"ls -l \'" + curDirPath + "\'\"";

  ui->lePath->setText(curDirPath);
  proc->start(command);
  if (!proc->waitForFinished())
    return;

  QByteArray resBA = proc->readAllStandardOutput();
  QString res = QString::fromUtf8(resBA);
  proc->close();

  res = res.replace("\r\r", "\r");
  res = res.trimmed();

  ui->teOutput->append(res);

  QStringList list, dirlist, filelist;
  list = res.split("\r\n", QString::SkipEmptyParts);
  // qDebug() << "listLen:" << list.length();

  if (list.length() > 0) {
    // qDebug() << "list";
    dirsList->clear();
    filesList->clear();

    foreach(QString item, list) {
      if (item.at(0) == 'd') {
        QStringList attrs = item.split(QRegExp("\\s+"));
        QString name = "";
        for (int i = 5; i<attrs.length(); i++) {
          name += attrs[i] + " ";
        }
        name.remove(name.length()-1, 1);

        QString attributes = attrs[0];

        FileItem dirItem;
        dirItem.fileName = name;
        dirItem.attributes = attributes;
        dirsList->append(dirItem);

        dirlist.append(name);
      }
      else if (item.at(0) == '-') {
        QStringList attrs = item.split(QRegExp("\\s+"));
        QString name = "";
        for (int i = 6; i<attrs.length(); i++) {
          name += attrs[i] + " ";
        }
        name.remove(name.length()-1, 1);

        int size = attrs[3].toInt();
        QString attributes = attrs[0];

        FileItem fileItem;
        fileItem.fileName = name;
        fileItem.fileSize = size;
        fileItem.attributes = attributes;
        filesList->append(fileItem);

        filelist.append(name);
      }
    }
    ui->teOutput->moveCursor(QTextCursor::Start);

    qSort(dirsList->begin(), dirsList->end(), compareInsensitiveItems);
    qSort(filesList->begin(), filesList->end(), compareInsensitiveItems);

    // QStandardItemModel *tableModel = new QStandardItemModel();
    tableModel->clear();
    tableModel->setColumnCount(3);

    QStringList labels;
    labels.append("Name");
    labels.append("Size");
    labels.append("Attributes");
    tableModel->setHorizontalHeaderLabels(labels);

    for (int i = 0; i<dirsList->length(); i++) {
      FileItem item = dirsList->at(i);
      QList<QStandardItem*> list;

      QStandardItem *nameCol = new QStandardItem(item.fileName);
      nameCol->setData(QIcon("icons/dir.png"), Qt::DecorationRole);

      QStandardItem *sizeCol = new QStandardItem("");
      QStandardItem *attrsCol = new QStandardItem(item.attributes);

      list.append(nameCol);
      list.append(sizeCol);
      list.append(attrsCol);

      tableModel->appendRow(list);
    }

    for (int i = 0; i<filesList->length(); i++) {
      FileItem item = filesList->at(i);
      QList<QStandardItem*> list;

      QStandardItem *nameCol = new QStandardItem(item.fileName);
      nameCol->setData(QIcon("icons/file.png"), Qt::DecorationRole);

      QString fileSize = formatSize(item.fileSize);
      QStandardItem *sizeCol = new QStandardItem(fileSize);
      QStandardItem *attrsCol = new QStandardItem(item.attributes);

      list.append(nameCol);
      list.append(sizeCol);
      list.append(attrsCol);

      tableModel->appendRow(list);
    }

    tableView()->setModel(tableModel);


    QHeaderView *hHeader = tableView()->horizontalHeader();
    hHeader->setResizeMode(0, QHeaderView::Stretch);
    hHeader->resizeSection(1, 100);
    hHeader->setStyleSheet(hHeaderStyle);

    QHeaderView *vHeader = tableView()->verticalHeader();
    vHeader->setStyleSheet(vHeaderStyle);

    updateStatus(dirlist.length(), filelist.length());

    QModelIndex index = tableView()->model()->index(0, 0);
    tableView()->setCurrentIndex(index);
  }
  else {
    tableView()->setModel(NULL);
    updateStatus(0, 0);
  }
}


// ------------------------------------------------ custom slots ------------------------------------------------

void MainWindow::homeList() {
  QModelIndex index = tableView()->model()->index(0, 0);
  tableView()->setCurrentIndex(index);
}

void MainWindow::endList() {
  int rows = tableView()->model()->rowCount();
  QModelIndex index = tableView()->model()->index(rows-1, 0);
  tableView()->setCurrentIndex(index);
}

void MainWindow::prevDir() {
  bool focus = tableView()->hasFocus();
  // qDebug() << "table-focus-prev:" << focus;
  if (!focus) return;

  ui->bGoUp->click();
}

void MainWindow::enterDir() {
  bool focus = tableView()->hasFocus();
  // qDebug() << "table-focus-enter:" << focus;
  if (!focus) return;

  QModelIndexList list = tableView()->selectionModel()->selectedIndexes();
  if (list.length() == 0) {
    // qDebug() << "no-selection";
    return;
  }
  QModelIndex index = list.at(0);
  if (!index.isValid()) {
    // qDebug() << "Invalid Index (enterDir)";
    // ui->teOutput->append("Invalid Index (enterDir)");
    return;
  }

  int row = index.row();
  int column = index.column();

  QModelIndex nameIndex = index.sibling(row, 0);
  QModelIndex attrsIndex = index.sibling(row, 2);

  if (!nameIndex.isValid()) {
    // ui->teOutput->append("Invalid Sibling Index (enterDir)");
    return;
  }

  QString name = nameIndex.data().toString();
  QString attrs = attrsIndex.data().toString();

  QString type = "file";
  if (attrs.length() != 0) {
    QChar typeChar = attrs.at(0);
    if (typeChar == 'd') {
      type = "dir";
    }
  }

  if (type == "dir") {
    goToDir(name);
  }
  else {
    // qDebug() << "file (enterDir)";
  }
}

void MainWindow::copyProcFinished(int status) {
  // qDebug() << "copyProcFinished";
}

void MainWindow::finishedNextCopyAction() {
  fileSize = 0;
  prevFileSize = 0;
  curCopyNum++;
}

void MainWindow::timerUpdate() {
  int speedi = fileSize - prevFileSize;
  speedText = formatSize(speedi);
  prevFileSize = fileSize;
}

void MainWindow::setCopyProgressValue(int val, int max) {
  if (val == 0) {
    ui->pbCopy->setValue(0);
    ui->lProgress->setText("");
    return;
  }
  if (!copyActive) return;

  fileSize = val;
  int percent = 0;
  if (val != 0 && max != 0) {
    percent = (double) val / max * 100;
  }
  ui->pbCopy->setValue(percent);

  int curNum = curCopyNum + 1;
  curNum = qMin(curNum, totalCopyNum);

  QString copiedCur = formatSize(val);
  QString copiedTotal = formatSize(max);
  QString copySpeed = speedText.length() != 0 ? ", " + speedText + "/s": "";

  QString progressText = QString("Copy [%1/%2]: %3 of %4%5").arg(curNum).arg(totalCopyNum).arg(copiedCur).arg(copiedTotal).arg(copySpeed);

  ui->lProgress->setText(progressText);
}

void MainWindow::copyQueueFinished() {
  copyActive = false;
  ui->lProgress->setText("Copy Queue Finished");
  ui->pbCopy->setValue(0);
}


// ------------------------------------------------ ui service ------------------------------------------------

void MainWindow::updateStatus(int dirlen, int filelen) {
  ui->lStatus->setText("Dirs: " + QString::number(dirlen) + "; Files: " + QString::number(filelen));
}

QTableView* MainWindow::tableView() {
  return ui->tvList;
}


// ------------------------------------------------ service ------------------------------------------------

QString MainWindow::fixPullEncoding(QString item) {
  QByteArray b = item.toUtf8();
  QString res;

  QByteArray codes;
  codes.append(0x81);
  codes.append(0x8d);
  codes.append(0x8f);
  codes.append(0x90);
  codes.append(0x9d);

  QTextCodec *codec = QTextCodec::codecForName("Windows-1252");

  for (int i = 0; i<b.length(); i++) {
    QString s1;
    char ch = b.at(i);

    if (codes.contains(ch)) {
      unsigned char code = (unsigned char) ch;
      // qDebug() << "fix-code:" << code;

      res += ch;
      continue;
    }

    QByteArray btemp;
    btemp.append(ch);

    s1 = codec->toUnicode(btemp);
    res += s1;
  }

  return res;
}

QString MainWindow::formatPath(QString path) {
  if (path.length() == 0) {
    path = "Downloads";
  }

  QString res = "";
  // QFileInfo info(path);
  // res = info.absoluteFilePath();

  QDir d(path);
  res = d.absolutePath();
  res.replace(QRegExp("/$"), "");

  qDebug() << res;

  return res;
}

QString MainWindow::formatSize(int size) {
  double res = (double) size, tmp;
  QString sizeRes = "";

  QStringList unitsList;
  unitsList << "B" << "KB" << "MB" << "GB";
  QString units = unitsList.at(0);
  int uidx = 0;


  tmp = res / 1000;
  while(tmp > 1) {
    res = tmp;
    uidx++;
    units = unitsList.at(uidx);
    tmp = tmp / 1000;
  }

  sizeRes = QString::number(res, 'f', 2) + " " + units;
  return sizeRes;
}

QString MainWindow::getPrevDir(QString dir) {
  QString res = dir;

  res.replace(QRegExp("/$"), "");
  QRegExp rx("^(.+)/");
  int pos = rx.indexIn(res);
  if (pos == -1) return "";
  res = rx.cap(1);

  return res;
}

QString MainWindow::getCurrentDir(QString dir) {
  QString res = dir;

  res.replace(QRegExp("/$"), "");
  QRegExp rx("[^/]+$");
  int pos = rx.indexIn(res);
  if (pos == -1) return "";
  res = rx.cap(0);

  return res;
}


// ----------------------------------------- add -----------------------------------------

QString MainWindow::getCodesFromString(QString str) {
  QString res = "";
  QString sep = " ";

  for (int i = 0; i<str.length(); i++) {
    if (i == str.length()-1)
      sep = "";
    res += QString::number(str.at(i).unicode()) + sep;
  }

  return res;
}

QString MainWindow::getCodesFromBytes(QByteArray bytes) {
  QString res = "";
  QString sep = " ";

  for (int i = 0; i<bytes.length(); i++) {
    if (i == bytes.length()-1) sep = "";
    res += QString::number((int) bytes.at(i)) + sep;
  }

  return res;
}

QString MainWindow::getCodesFromBytesHex(QByteArray bytes) {
  QString res = "";
  QString sep = " ";

  bytes = bytes.toHex();

  for (int i = 0; i<bytes.length(); i++) {
    if (i == bytes.length()-1)
      sep = "";
    res += bytes[i];
    if (i%2 != 0)
      res += sep;
  }

  return res;
}
