#ifndef FOCUSER_H
#define FOCUSER_H

#include <QWidget>

class Focuser {
  QWidget *widget;
public:
  Focuser(QWidget *);
  void show();
  void focus();
};

#endif // FOCUSER_H
