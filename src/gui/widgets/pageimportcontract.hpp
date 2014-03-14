#ifndef PAGEIMPORTCONTRACT_HPP
#define PAGEIMPORTCONTRACT_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <QWizardPage>

namespace Ui {
class MTPageImportContract;
}

class MTPageImportContract : public QWizardPage
{
    Q_OBJECT

    QString m_qstrContents;

public:
    explicit MTPageImportContract(QWidget *parent = 0);
    ~MTPageImportContract();

    bool isComplete() const;

    //virtual
    int nextId() const;

    //virtual
    bool validatePage();

    QString getContents() const { return m_qstrContents; }

private slots:
    void on_radioButton_1_toggled(bool checked);

    void on_radioButton_2_toggled(bool checked);

    void on_radioButton_3_toggled(bool checked);

    void on_pushButton_clicked();

    void on_plainTextEditPaste_textChanged();

private:
    Ui::MTPageImportContract *ui;
};

#endif // PAGEIMPORTCONTRACT_HPP
