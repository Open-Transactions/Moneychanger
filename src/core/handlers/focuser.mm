#include "focuser.h"
#import <Cocoa/Cocoa.h>

Focuser::Focuser(QWidget *w) {
  this->widget = w;
}

void Focuser::show() {
  this->widget->show();
  this->focus();
}

void Focuser::focus() {
  [NSApp activateIgnoringOtherApps:YES];
  this->widget->activateWindow();
  this->widget->raise();
}
