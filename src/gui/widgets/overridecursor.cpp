#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <QApplication>
#include <QEvent>

#include <gui/widgets/overridecursor.hpp>


// -----------------------------------
MTSpinner::MTSpinner()
    : m_pCursor(MTOverrideCursor::It())
{   // Note: MTOverrideCursor::It() returns NULL if one is already instantiated.
    // Therefore, m_pCursor MIGHT contain a pointer, or MIGHT contain NULL.
    // This way, only the outermost-spinner ever actually does something.
    // The outermost one instantiates, and sets the pointer into MTSpinner.
    // Then when the MTSpinner destructs, the pointer is set, so it deletes it,
    // and the MTOverrideCursor being deleted sets the static pointer back to NULL
    // in its destructor.
    // (This way, the next run it will be instantiated again and work properly.)
    // Whereas any INNER MTSpinners will do nothing, since a NULL pointer will be
    // set here, and thus the inner MTSpinner's destructor will do nothing, and the
    // static MTOverrideCursor pointer will remain valid until the outermost one destroys it.
    // Easy, right?  :-)
}

// Note:
MTSpinner::~MTSpinner()
{
    if (NULL != m_pCursor)
        delete m_pCursor;
    m_pCursor = NULL;
}
// --------------------------------------------
//static
MTOverrideCursor * MTOverrideCursor::Exists()
{
    // This version is used by the password dialog, which temporarily
    // deactivates the override cursor until it's finished collecting
    // the password. (Only IF the override cursor is set...)
    //
    return MTOverrideCursor::s_pCursor;
}
// --------------------------------------------
//static
MTOverrideCursor * MTOverrideCursor::s_pCursor = NULL;
// --------------------------------------------
//static
MTOverrideCursor * MTOverrideCursor::It()
{
    // Use:  opentxs::OTCleanup<MTOverrideCursor> theCursorAngel(MTOverrideCursor::It());
    //
    // Oooo even better, use:  MTSpinner theSpinner;
    // ----------------------------
    // How does this work?
    //
    // MTOverrideCursor::s_pCursor is instantiated, if NULL, and a pointer returned.
    //
    // If it's returned, it should be stored in an opentxs::OTCleanup instance, which will destroy
    // it when it goes out of scope.
    //
    // Meanwhile our destructor, ~MTOverrideCursor, sets MTOverrideCursor::s_pCursor back
    // to NULL again, so that future calls will correctly instantiate it again, instead of
    // returning a bad pointer.
    //
    // One more thing: if MTOverrideCursor::s_pCursor is NOT NULL when calling It(), then
    // we return NULL. This is because It() must have been called twice. For example, if you
    // call function A, which sets the override spinner, and then it calls function B, which
    // calls function C, which ALSO sets the override spinner, then a few things are clear:
    //
    // 1. Function C might legitimately activate the spinner, but in the case where it's called by
    //    function A, we do NOT want to activate the spinner, since it's ALREADY ACTIVE (from A.)
    //
    // 2. Function A in this case is ALREADY going to clean up the spinner at the right time, and thus
    //    there's no need to Function C to clean it up as well.
    //
    // 3. Even if Function C was allowed to clean it up, we'd have to make sure it was a different one
    //    than the one Function A is going to clean up, to prevent a double delete. By returning a NULL
    //    pointer to function C, we know it will clean up nothing, which is proper. (And in cases where a
    //    pointer IS returned to function C, we know it will clean up something, as is also proper.)
    //
    // 4. Even if functions A and C were cleaning up their own copies of the object, which they are not since
    //    we're now using a singleton, we would STILL have to deal with the fact that both of them are setting
    //    and removing the override cursor on the QApplication. But our use of a singleton saves us from that problem.
    //
    // By returning NULL in cases where the pointer is already set, we insure that only the OUTERMOST USE of
    // the override cursor will actually do anything, and we also insure that a pointer to that use is available
    // for strange cases like if we are in the password dialog and we need to temporarily suspend the override
    // cursor until the password dialog is finished.
    //
    if (NULL != MTOverrideCursor::s_pCursor)
        return NULL;
    // ------------------------------------------------
    // Else s_pCursor is definitely NULL, so we can go ahead and instantiate it.
    //
    MTOverrideCursor::s_pCursor = new MTOverrideCursor;

    return MTOverrideCursor::s_pCursor;
}
// ------------------------------------------------
void MTOverrideCursor::Pause()
{
    if (!m_bPaused)
    {
        m_bPaused = true;

        this->SpinnerDeactivate();
    }
}
// ------------------------------------------------
void MTOverrideCursor::Unpause()
{
    if (m_bPaused)
    {
        m_bPaused = false;

        this->SpinnerActivate();
    }
}
// ------------------------------------------------
void MTOverrideCursor::SpinnerActivate()
{
    if (!m_bActive)
    {
        m_bActive = true;

        QApplication::setOverrideCursor(Qt::WaitCursor);

        QCoreApplication * pCore = QCoreApplication::instance();

        pCore->installEventFilter(this);
    }
}
// ------------------------------------------------
void MTOverrideCursor::SpinnerDeactivate()
{
    if (m_bActive)
    {
        m_bActive = false;

        QApplication::restoreOverrideCursor();

        QCoreApplication * pCore = QCoreApplication::instance();

        pCore->removeEventFilter(this);
    }
}
// ------------------------------------------------
MTOverrideCursor::MTOverrideCursor(QObject *parent) :
    QObject(NULL), // notice we pass NULL here instead of parent.
    m_bActive(false),
    m_bPaused(false)
{
    this->SpinnerActivate();
}
// ------------------------------------------------
MTOverrideCursor::~MTOverrideCursor()
{
    MTOverrideCursor::s_pCursor = NULL;
    // ----------------------------------------
    this->SpinnerDeactivate();
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
