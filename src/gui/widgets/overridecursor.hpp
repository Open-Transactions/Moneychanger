#ifndef OVERRIDECURSOR_HPP
#define OVERRIDECURSOR_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <QObject>


// Use:  OTCleanup<MTOverrideCursor> theCursorAngel(MTOverrideCursor::It());
//
// Actually even better, use:  MTSpinner theSpinner;
//
// (See CPP file for notes on why.)


class MTOverrideCursor : public QObject
{
    Q_OBJECT
protected:
    static   MTOverrideCursor * s_pCursor;
    explicit MTOverrideCursor(QObject *parent = 0); // parent is always ignored in this class.
public:
    virtual ~MTOverrideCursor();
    static   MTOverrideCursor * It();

    // This version is used by the password dialog, which temporarily
    // deactivates the override cursor until it's finished collecting
    // the password. (Only IF the override cursor is set...)
    //
    static   MTOverrideCursor * Exists();
    // -----------------------------------
    // NOTE: these two methods ONLY used by DlgPassword and DlgPasswordConfirm.
    // Might even want to make these private and then make those friend classes.
    //
    void Pause();
    void Unpause();

signals:

public slots:

protected:
    bool m_bActive;
    bool m_bPaused;

    void SpinnerActivate();
    void SpinnerDeactivate();

    bool eventFilter( QObject *obj, QEvent *event );
};

// -----------------------------------

class MTSpinner
{
    MTOverrideCursor * m_pCursor;
public:
    MTSpinner();
    ~MTSpinner();
};

// -----------------------------------

#endif // OVERRIDECURSOR_HPP


