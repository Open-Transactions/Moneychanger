#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/ui/dlginbailment.hpp>
#include <ui_dlginbailment.h>

#include <QMessageBox>
#include <QClipboard>
#include <QKeyEvent>

//const auto& id = request.id();
//const auto& server = request.server();
//const auto& sender = request.initiator();
//const auto& recipient = request.recipient();
//const auto& pendingBailment = request.pendingbailment();
//const auto& unit = pendingBailment.unitid();
//const auto& bailmentServer = pendingBailment.serverid();
//const auto& txid = pendingBailment.txid();


DlgInbailment::DlgInbailment(QWidget *parent, QString notaryId, QString issuerNymId, QString myNymId, QString unitTypeId, QString blockchainAddress) :
    QDialog(parent),
    ui(new Ui::DlgInbailment),
    notaryId_(notaryId),
    issuerNymId_(issuerNymId),
    myNymId_(myNymId),
    unitTypeId_(unitTypeId),
    blockchainAddress_(blockchainAddress)
{
    ui->setupUi(this);

    this->installEventFilter(this);

    ui->lineEditAddress->setStyleSheet("QLineEdit { background-color: lightgray }");

    ui->lineEditAddress->setText(blockchainAddress);
    ui->toolButtonQrCode->setString(blockchainAddress);
}

DlgInbailment::~DlgInbailment()
{
    delete ui;
    ui = nullptr;
}

void DlgInbailment::on_toolButtonCopyAddress_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();

    if (nullptr != clipboard)
    {
        clipboard->setText(ui->lineEditAddress->text());

        QMessageBox::information(this, tr("Moneychanger"), QString("%1:<br/>%2").
                                 arg(tr("Copied blockchain deposit address to the clipboard")).
                                 arg(ui->lineEditAddress->text()));
    }
}

void DlgInbailment::on_toolButtonQrCode_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();

    if (nullptr != clipboard)
    {
        QImage image;
        ui->toolButtonQrCode->asImage(image, 200);
        QPixmap pixmapQR = QPixmap::fromImage(image);

        clipboard->setPixmap(pixmapQR);

        QMessageBox::information(this, tr("Moneychanger"), QString("%1:<br/>%2").
                                 arg(tr("Copied QR CODE IMAGE to the clipboard, of blockchain deposit address")).
                                 arg(ui->lineEditAddress->text()));
    }

}

bool DlgInbailment::eventFilter(QObject *obj, QEvent *event)
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
