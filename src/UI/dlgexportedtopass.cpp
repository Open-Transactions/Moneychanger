#include <QApplication>
#include <QMessageBox>
#include <QClipboard>
#include <QKeyEvent>
#include <QDebug>

#include "dlgexportedtopass.h"
#include "ui_dlgexportedtopass.h"

DlgExportedToPass::DlgExportedToPass(QWidget *parent, QString qstrTheCash) :
    QDialog(parent),
    ui(new Ui::DlgExportedToPass)
{
    ui->setupUi(this);

    this->installEventFilter(this);

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
                                  arg(tr("WARNING: You are strongly advised to first SAVE A COPY of the exported cash!")),
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

        QMessageBox::information(this, tr("Cash Copied"), tr("Copied exported cash to the clipboard"));
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
        return QObject::eventFilter(obj, event);
    }
}
