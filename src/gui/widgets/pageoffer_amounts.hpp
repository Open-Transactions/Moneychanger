#ifndef PAGEOFFER_AMOUNTS_HPP
#define PAGEOFFER_AMOUNTS_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <QWizardPage>

namespace Ui {
class PageOffer_Amounts;
}

class PageOffer_Amounts : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageOffer_Amounts(QWidget *parent = 0);
    ~PageOffer_Amounts();

protected:
    void initializePage(); //virtual
    void RadioChanged();

    bool isComplete() const;//virtual

private slots:
    void on_radioButton_clicked();

    void on_radioButton_2_clicked();

    void on_comboBox_currentIndexChanged(const QString &arg1);

    void on_lineEditQuantity_textChanged(const QString &arg1);

    void on_lineEditPrice_textChanged(const QString &arg1);

private:
    Ui::PageOffer_Amounts *ui;
};

#endif // PAGEOFFER_AMOUNTS_HPP
