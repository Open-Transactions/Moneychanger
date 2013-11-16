#include <QApplication>
#include <QEvent>

#include "overridecursor.h"

MTOverrideCursor::MTOverrideCursor(QObject *parent) :
    QObject(parent)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    QCoreApplication * pCore = QCoreApplication::instance();

    pCore->installEventFilter(this);
}



MTOverrideCursor::~MTOverrideCursor()
{
    QApplication::restoreOverrideCursor();

    QCoreApplication * pCore = QCoreApplication::instance();

    pCore->removeEventFilter(this);

}

bool MTOverrideCursor::eventFilter(QObject *obj, QEvent *event)
{
    switch ( event->type())
    {
    //list event you want to prevent here ...
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick:
    //...
    return true;
    default: break;
    }
    return QObject::eventFilter( obj, event );
}

// -----------------------------------------------------
