#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/ui/dlgpairnode.hpp>
#include <ui_dlgpairnode.h>

//#include <core/handlers/serialportreader.hpp>

#include <QSerialPort>
#include <QTextStream>
#include <QMessageBox>
#include <QKeyEvent>
#include <QDebug>


//void DlgPairNode::on_pushButtonListen_clicked()
//{
//    QSerialPort serialPort;
//    QString serialPortName = ui->lineEditPort->text();
//    serialPort.setPortName(serialPortName);

//    QString qstrBaudRate = ui->lineEditBaud->text();
//    // Notice something missing here?
//    int serialPortBaudRate = QSerialPort::Baud38400;
//    serialPort.setBaudRate(serialPortBaudRate);

//    if (!serialPort.open(QIODevice::ReadOnly)) {
//        qDebug() << QObject::tr("Failed to open port %1, error: %2").arg(serialPortName).arg(serialPort.errorString()) << endl;
//    }
//    else
//        SerialPortReader serialPortReader(&serialPort);
//}

void DlgPairNode::on_pushButtonListen_clicked()
{
      QSerialPort serialPort;
      QString serialPortName = ui->lineEditPort->text();
      serialPort.setPortName(serialPortName);

      int serialPortBaudRate = QSerialPort::Baud38400;
      serialPort.setBaudRate(serialPortBaudRate);

      if (!serialPort.open(QIODevice::ReadOnly)) {
          QMessageBox::information(this, tr("Failure"), tr("Failed to open port %1, error: %2").arg(serialPortName).arg(serialPort.error()));
      }
      else
      {
          QByteArray readData = serialPort.readAll();
          if (serialPort.waitForReadyRead(5000))
              readData.append(serialPort.readAll());

          if (serialPort.error() == QSerialPort::ReadError) {
              QMessageBox::information(this, tr("Failure"), tr("Failed to read from port %1, error: %2").arg(serialPortName).arg(serialPort.errorString()));
          } else if (serialPort.error() == QSerialPort::TimeoutError && readData.isEmpty()) {
              QMessageBox::information(this, tr("Failure"), tr("No data was currently available for reading from port %1").arg(serialPortName));
          }
          else {
              QMessageBox::information(this, tr("Success"), tr("Data successfully received from port %1").arg(serialPortName));
              qDebug() << readData << endl;

              QString qstrData{readData};

              if (!qstrData.isEmpty())
                  ProcessSerialData(qstrData);
          }
      }
}


void DlgPairNode::ProcessSerialData(QString & serial_data)
{
    QRegularExpression re("(\\w+)");
    QRegularExpressionMatchIterator i = re.globalMatch("the quick fox");

    QStringList words;
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString word = match.captured(1);
        words << word;
    }
    // words contains "the", "quick", "fox"
}


DlgPairNode::DlgPairNode(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgPairNode)
{
    ui->setupUi(this);

    this->installEventFilter(this);
}

DlgPairNode::~DlgPairNode()
{
    delete ui;
}

void DlgPairNode::on_buttonBox_accepted()
{
    // Blah blah
    // ----------------------------------
    accept();
}


bool DlgPairNode::eventFilter(QObject *obj, QEvent *event)
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

void DlgPairNode::on_buttonBox_rejected()
{
    reject();
}

