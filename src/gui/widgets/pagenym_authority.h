#ifndef PAGENYM_AUTHORITY_H
#define PAGENYM_AUTHORITY_H

#include <QWizardPage>

namespace Ui {
class MTPageNym_Authority;
}

class MTPageNym_Authority : public QWizardPage
{
    Q_OBJECT

public:
    explicit MTPageNym_Authority(QWidget *parent = 0);
    ~MTPageNym_Authority();

    virtual int nextId() const;

private:
    Ui::MTPageNym_Authority *ui;
};

#endif // PAGENYM_AUTHORITY_H
