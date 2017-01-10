#ifndef DLGPAIRNODE_HPP
#define DLGPAIRNODE_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <QDialog>



namespace Ui {
class DlgPairNode;
}

class DlgPairNode : public QDialog
{
    Q_OBJECT
public:
    explicit DlgPairNode(QWidget *parent = 0);
    ~DlgPairNode();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

signals:

public slots:

private slots:
    void on_pushButtonListen_clicked();
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    void ProcessSerialData(QString & serial_data);

    Ui::DlgPairNode *ui;
};

#endif // DLGPAIRNODE_HPP


