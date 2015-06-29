#ifndef PAGESMART_PARTY_HPP
#define PAGESMART_PARTY_HPP

#include <QWizardPage>

namespace Ui {
class PageSmart_Party;
}

class PageSmart_Party : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageSmart_Party(QWidget *parent = 0);
    ~PageSmart_Party();

    bool isComplete() const;

protected:
    void initializePage(); //virtual

    void SetFieldsBlank();

private slots:
    void on_pushButtonSelect_clicked();

private:
    QString m_qstrClickText;
    Ui::PageSmart_Party *ui;
};

#endif // PAGESMART_PARTY_HPP
