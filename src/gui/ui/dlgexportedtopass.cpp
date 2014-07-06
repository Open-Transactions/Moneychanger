#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/ui/dlgexportedtopass.hpp>
#include <ui_dlgexportedtopass.h>

#include <QMessageBox>
#include <QClipboard>


DlgExportedToPass::DlgExportedToPass(QWidget *parent, QString qstrTheCash,
                                     QString qstrLabelHeader1/*=QString("")*/,
                                     QString qstrLabelHeader2/*=QString("")*/) :
    QDialog(parent),
//    m_qstrLabelHeader1(qstrLabelHeader1),
//    m_qstrLabelHeader2(qstrLabelHeader2),
    ui(new Ui::DlgExportedToPass)
{
    ui->setupUi(this);

    this->installEventFilter(this);

    if (!qstrLabelHeader1.isEmpty())
        ui->labelHeader1->setText(qstrLabelHeader1);
    if (!qstrLabelHeader2.isEmpty())
        ui->labelHeader2->setText(qstrLabelHeader2);

    ui->plainTextEdit->setPlainText(qstrTheCash);
}

DlgExportedToPass::~DlgExportedToPass()
{
    delete ui;
}

void DlgExportedToPass::on_buttonBox_accepted()
{
    QMessageBox::StandardButton reply;

    reply = QMessageBox::question(this, "", QString("%1<br/><br/>%2").arg(tr("Are you sure you want to close this dialog?")).
                                  arg(tr("WARNING: You are strongly advised to first SAVE A COPY!")),
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
        accept();
}

void DlgExportedToPass::on_pushButton_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();

    if (NULL != clipboard)
    {
        clipboard->setText(ui->plainTextEdit->toPlainText());

        QMessageBox::information(this, tr("Copied"), tr("Copied to the clipboard"));
    }
}

bool DlgExportedToPass::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Escape){
            close(); // This is caught by this same filter.
            return true;
        }
        return true;
    }else {
        // standard event processing
        return QDialog::eventFilter(obj, event);
    }
}
