#include <QDebug>
#include <QMessageBox>

#include "compose.h"
#include "ui_compose.h"

#include "dlgchooser.h"



void MTCompose::on_toButton_clicked()
{
    // Select from Nyms in address book.
}

void MTCompose::on_fromButton_clicked()
{
    // Select from Nyms in local wallet.

    // -----------------------------------------------
    DlgChooser * pChooser = new DlgChooser(this);
    pChooser->setAttribute(Qt::WA_DeleteOnClose);
    // -----------------------------------------------
    mapIDName & the_map = pChooser->m_map;





    the_map.insert(QString("ID1"), QString("NAME1"));
    the_map.insert(QString("ID2"), QString("NAME2"));
    the_map.insert(QString("ID3"), QString("NAME3"));
    the_map.insert(QString("ID4"), QString("NAME4"));
    the_map.insert(QString("ID5"), QString("NAME5"));
    the_map.insert(QString("ID6"), QString("NAME6"));
    // -----------------------------------------------
    pChooser->SetPreSelected("ID4");
    // -----------------------------------------------
    pChooser->setWindowTitle("You must choose wisely.");
    // -----------------------------------------------
    if (pChooser->exec() == QDialog::Accepted) {
        qDebug() << QString("SELECT was clicked for ID: %1").arg(pChooser->m_qstrCurrentID);
    } else {
      qDebug() << "CANCEL was clicked";
    }
    // -----------------------------------------------

}

void MTCompose::on_serverButton_clicked()
{
    // Select from servers in local wallet filtered by both Nyms.
}

void MTCompose::on_sendButton_clicked()
{
    // Send message and then close dialog. Use progress bar.
}

void MTCompose::dialog()
{
/** Compose Dialog **/

    if (!already_init)
    {
        this->setWindowTitle("Compose: (no subject)");

        QString style_sheet = "QPushButton{border: none; border-style: outset; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa);}"
                "QPushButton:pressed {border: 1px solid black; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa); }"
                "QPushButton:hover {border: 1px solid black; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa); }";

        ui->fromButton->setStyleSheet(style_sheet);
        ui->toButton->setStyleSheet(style_sheet);
        ui->serverButton->setStyleSheet(style_sheet);

        // Here if there is pre-set data for the subject, contents, to, from, server, etc
        // then we set it here.
        //


        // Todo.


        // -------------------------------------------

        /** Flag Already Init **/
        already_init = true;
    }

}

void MTCompose::closeEvent(QCloseEvent *event)
{
    // Pop up a Yes/No dialog to confirm the cancellation of this message.
    // (ONLY if the subject or contents fields contain text.)
    //
    if (!ui->contentsEdit->toPlainText().isEmpty() || !ui->subjectEdit->text().isEmpty())
    {
        QMessageBox::StandardButton reply;

        reply = QMessageBox::question(this, "", "Are you sure you want to cancel this message?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply != QMessageBox::Yes) {
          event->ignore();
          return;
        }
    }
    // -------------------------------------------
    QWidget::closeEvent(event);
}

MTCompose::MTCompose(QWidget *parent) :
    QWidget(parent, Qt::Window),
    already_init(false),
    ui(new Ui::MTCompose)
{
    ui->setupUi(this);

    this->installEventFilter(this);
}

MTCompose::~MTCompose()
{
    delete ui;
}

bool MTCompose::eventFilter(QObject *obj, QEvent *event){

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

void MTCompose::on_subjectEdit_textChanged(const QString &arg1)
{
    if (arg1.isEmpty())
        this->setWindowTitle(QString("Compose: (no subject)"));
    else
        this->setWindowTitle(QString("Compose: %1").arg(arg1));
}
