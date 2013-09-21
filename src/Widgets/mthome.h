#ifndef MTHOME_H
#define MTHOME_H

#include <QWidget>

namespace Ui {
class MTHome;
}

class MTHome : public QWidget
{
    Q_OBJECT
    
public:
    explicit MTHome(QWidget *parent = 0);
    ~MTHome();
    
private slots:
    void on_tableView_clicked(const QModelIndex &index);

private:
    Ui::MTHome *ui;
};

#endif // MTHOME_H
