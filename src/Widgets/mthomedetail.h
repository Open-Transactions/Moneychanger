#ifndef MTHOMEDETAIL_H
#define MTHOMEDETAIL_H

#include <QWidget>

namespace Ui {
class MTHomeDetail;
}

class MTHomeDetail : public QWidget
{
    Q_OBJECT
    
public:
    explicit MTHomeDetail(QWidget *parent = 0);
    ~MTHomeDetail();
    
private:
    Ui::MTHomeDetail *ui;
};

#endif // MTHOMEDETAIL_H
