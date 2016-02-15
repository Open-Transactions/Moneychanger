#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/ui/dlgexportedcash.hpp>
#include <ui_dlgexportedcash.h>

#include <QMessageBox>
#include <QClipboard>


DlgExportedCash::DlgExportedCash(QWidget *parent, QString qstrHisCopy, QString qstrMyCopy,
                                 QString   qstrLabelHeader1/*=QString("")*/,
                                 QString   qstrLabelHeader2/*=QString("")*/,
                                 QString   qstrLabelHeader3/*=QString("")*/,
                                 QString   qstrLabelHeader4/*=QString("")*/,
                                 bool      bShowWarning/*=true*/) :
    QDialog(parent),
    m_bShowWarning(bShowWarning),
    ui(new Ui::DlgExportedCash)
{
    ui->setupUi(this);

    this->installEventFilter(this);

    ui->plainTextEdit    ->setPlainText(qstrHisCopy);
    ui->plainTextEditCopy->setPlainText(qstrMyCopy);

    if (!qstrLabelHeader1.isEmpty())
        ui->labelHeader1->setText(qstrLabelHeader1);
    if (!qstrLabelHeader2.isEmpty())
        ui->labelHeader2->setText(qstrLabelHeader2);
    if (!qstrLabelHeader3.isEmpty())
        ui->labelHeader3->setText(qstrLabelHeader3);
    if (!qstrLabelHeader4.isEmpty())
        ui->labelHeader4->setText(qstrLabelHeader4);
}

DlgExportedCash::~DlgExportedCash()
{
    delete ui;
}

void DlgExportedCash::on_buttonBox_accepted()
{
    if (m_bShowWarning)
    {
        QMessageBox::StandardButton reply;

        reply = QMessageBox::question(this, "", QString("%1<br/><br/>%2").arg(tr("Are you sure you want to close this dialog?")).
                                      arg(tr("WARNING: You are strongly advised to first SAVE BOTH COPIES of the exported cash!")),
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes)
            accept();
    }
    else
        accept();
}

void DlgExportedCash::on_pushButton_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();

    if (NULL != clipboard)
    {
        clipboard->setText(ui->plainTextEdit->toPlainText());

        QMessageBox::information(this, tr("Moneychanger"), tr("Copied"));
    }
}

void DlgExportedCash::on_pushButtonCopy_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();

    if (NULL != clipboard)
    {
        clipboard->setText(ui->plainTextEditCopy->toPlainText());

        QMessageBox::information(this, tr("Moneychanger"), tr("Copied"));
    }
}

bool DlgExportedCash::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Escape)
        {
            close(); // This is caught by this same filter.
            return true;
        }
    }
    // standard event processing
    return QDialog::eventFilter(obj, event);
}

