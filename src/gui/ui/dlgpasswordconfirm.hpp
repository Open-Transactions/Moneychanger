#ifndef DLGPASSWORDCONFIRM_HPP
#define DLGPASSWORDCONFIRM_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <QString>
#include <QDialog>
#include <QCloseEvent>

#include <opentxs/core/crypto/OTPassword.hpp>


namespace Ui {
class MTDlgPasswordConfirm;
}

namespace opentxs {
class OTPassword;
}

class MTOverrideCursor;

class MTDlgPasswordConfirm : public QDialog
{
    Q_OBJECT

public:
    explicit MTDlgPasswordConfirm(QWidget *parent, opentxs::OTPassword & thePassword);
    ~MTDlgPasswordConfirm();

    void setDisplay(QString qstrDisplay);

protected:
    void closeEvent(QCloseEvent * e);

private slots:
    void on_MTDlgPasswordConfirm_accepted();

private:
    opentxs::OTPassword * m_pPassword;

    MTOverrideCursor * m_pCursor; // Do not delete. Here for reference only.

    Ui::MTDlgPasswordConfirm *ui;
};

#endif // DLGPASSWORDCONFIRM_HPP
