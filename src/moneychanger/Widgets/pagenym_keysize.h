#ifndef PAGENYM_KEYSIZE_H
#define PAGENYM_KEYSIZE_H

#include <QWizardPage>

namespace Ui {
class MTPageNym_Keysize;
}

class MTPageNym_Keysize : public QWizardPage
{
    Q_OBJECT

public:
    explicit MTPageNym_Keysize(QWidget *parent = 0);
    ~MTPageNym_Keysize();

private:
    Ui::MTPageNym_Keysize *ui;
};

#endif // PAGENYM_KEYSIZE_H
