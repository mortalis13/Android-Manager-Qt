#ifndef FILETABLE_H
#define FILETABLE_H

#include <QTableView>
#include <QDebug>
#include <QMouseEvent>

class FileTable : public QTableView
{
  Q_OBJECT
public:
  explicit FileTable(QWidget *parent = 0);


protected:
  void mouseReleaseEvent(QMouseEvent* e);

signals:

public slots:

};

#endif // FILETABLE_H
