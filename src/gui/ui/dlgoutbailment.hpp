#ifndef DLGOUTBAILMENT_HPP
#define DLGOUTBAILMENT_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <QDialog>

namespace Ui {
class DlgOutbailment;
}

class DlgOutbailment : public QDialog
{
    Q_OBJECT

public:
    explicit DlgOutbailment(QWidget *parent, std::int64_t & AMOUNT, std::string & blockchain_address,
                            QString asset_type, QString issuer_nym, QString notary);
    ~DlgOutbailment();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_pushButtonWithdraw_clicked();
    void on_lineEditAmount_editingFinished();
    void on_lineEditBlockchain_editingFinished();

private:
    Ui::DlgOutbailment *ui{nullptr};

    std::int64_t & AMOUNT_;
    std::string  & blockchain_address_;

    QString asset_type_;
    QString issuer_nym_;
    QString notary_;

    bool withdrawalPerformed_{false};
};

#endif // DLGOUTBAILMENT_HPP
