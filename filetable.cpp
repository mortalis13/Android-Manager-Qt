#include "filetable.h"

FileTable::FileTable(QWidget *parent) :
  QTableView(parent)
{
}


void FileTable::mouseReleaseEvent(QMouseEvent* e) {
  QTableView::mouseReleaseEvent(e);
  
  if (!model() || model()->rowCount() == 0) return;

  int y = e->y();
  int row = rowAt(y);
  
  if (row < 0) {
    int lastRow = model()->rowCount() - 1;
    int activeCol = 0;

    auto index = selectionModel()->currentIndex();
    if (index.isValid()) activeCol = index.column();

    setCurrentIndex(model()->index(lastRow, activeCol));
  }
}
