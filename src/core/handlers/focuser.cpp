#include "focuser.h"

Focuser::Focuser(QWidget *w) {
  this->widget = w;
}

void Focuser::show() {
  this->widget->show();
  this->focus();
}

void Focuser::focus() {
  this->widget->activateWindow();
  this->widget->raise();
}
