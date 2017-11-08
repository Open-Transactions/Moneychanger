#ifndef DLGINBAILMENT_HPP
#define DLGINBAILMENT_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <QDialog>

namespace Ui {
class DlgInbailment;
}

class DlgInbailment : public QDialog
{
    Q_OBJECT

public:
    //const auto& server = request.server();
    //const auto& sender = request.initiator();
    //const auto& recipient = request.recipient();
    //const auto& unit = pendingBailment.unitid();

    explicit DlgInbailment(QWidget *parent, QString notaryId, QString issuerNymId, QString myNymId, QString unitTypeId, QString blockchainAddress);
    ~DlgInbailment();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_toolButtonCopyAddress_clicked();
    void on_toolButtonQrCode_clicked();

private:
    Ui::DlgInbailment *ui{nullptr};

    QString notaryId_;
    QString issuerNymId_;
    QString myNymId_;
    QString unitTypeId_;
    QString blockchainAddress_;
};

#endif // DLGINBAILMENT_HPP
