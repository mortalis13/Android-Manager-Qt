#include "fun.h"

namespace Fun
{

void createDirs(QString path) {
  QDir d;
  d.mkpath(path);
}

}
