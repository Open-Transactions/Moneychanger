#ifndef DLGNEWCONTACT_HPP
#define DLGNEWCONTACT_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <QDialog>
#include <QString>
#include <QGridLayout>


class MTIdentifierWidget;

namespace Ui {
class MTDlgNewContact;
}

class MTDlgNewContact : public QDialog
{
    Q_OBJECT

private:
    bool m_bFirstRun;

    QGridLayout         * m_pIdLayout;
    MTIdentifierWidget  * m_pIdWidget;

    QString m_qstrNymID;

protected:
    virtual void showEvent(QShowEvent * event);
    bool eventFilter(QObject *obj, QEvent *event);

public:
    explicit MTDlgNewContact(QWidget *parent = 0);
    ~MTDlgNewContact();

    QString GetId() const { return m_qstrNymID; }

private slots:
    void on_buttonBox_accepted();

private:
    Ui::MTDlgNewContact *ui;
};

#endif // DLGNEWCONTACT_HPP
