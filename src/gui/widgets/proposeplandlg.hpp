#ifndef PROPOSEPLANDLG_HPP
#define PROPOSEPLANDLG_HPP

#include <QWidget>

namespace Ui {
class ProposePlanDlg;
}

class ProposePlanDlg : public QWidget
{
    Q_OBJECT

public:
    explicit ProposePlanDlg(QWidget *parent = 0);
    ~ProposePlanDlg();

private:
    Ui::ProposePlanDlg *ui;
};

#endif // PROPOSEPLANDLG_HPP
