#ifndef PAGESMART_PARTYACCT_HPP
#define PAGESMART_PARTYACCT_HPP

#include <QWizardPage>

namespace Ui {
class PageSmart_PartyAcct;
}

class PageSmart_PartyAcct : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageSmart_PartyAcct(QWidget *parent = 0);
    ~PageSmart_PartyAcct();

    bool isComplete() const;

protected:
    void initializePage(); //virtual

    void SetFieldsBlank();

private slots:
    void on_pushButtonSelect_clicked();

    void on_plainTextEdit_textChanged();

    void on_lineEditPartyName_textChanged(const QString &arg1);

private:
    QString m_qstrClickText;
    Ui::PageSmart_PartyAcct *ui;
};

#endif // PAGESMART_PARTYACCT_HPP
