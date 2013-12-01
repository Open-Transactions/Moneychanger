#include "pageacct_server.h"
#include "ui_pageacct_server.h"

#include "moneychanger.h"

#include "wizardaddaccount.h"

#include "dlgchooser.h"

#include <opentxs/OTAPI.h>
#include <opentxs/OT_ME.h>

MTPageAcct_Server::MTPageAcct_Server(QWidget *parent) :
    QWizardPage(parent),
    m_bFirstRun(true),
    ui(new Ui::MTPageAcct_Server)
{
    ui->setupUi(this);

    QString style_sheet = "QPushButton{border: none; border-style: outset; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa);}"
            "QPushButton:pressed {border: 1px solid black; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa); }"
            "QPushButton:hover {border: 1px solid black; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa); }";

    ui->pushButtonSelect->setStyleSheet(style_sheet);

    ui->lineEditID->setStyleSheet("QLineEdit { background-color: lightgray }");

    this->registerField("ServerID*", ui->lineEditID);
    // -----------------------------------------------
    connect(this, SIGNAL(SetDefaultServer(QString, QString)), Moneychanger::It(), SLOT(setDefaultServer(QString,QString)));
}



void MTPageAcct_Server::on_pushButtonSelect_clicked()
{
    QString qstr_default_id;
    // -------------------------------------------
    QWizard            * pWizard           = this->wizard();
    MTWizardAddAccount * pWizardAddAccount = (MTWizardAddAccount *)pWizard;
    // -------------------------------------------
    if (NULL != pWizardAddAccount)
    {
        qstr_default_id = Moneychanger::It()->get_default_server_id();
    }
    // -------------------------------------------
    QString qstr_current_id = ui->lineEditID->text();
    // -------------------------------------------
    if (qstr_current_id.isEmpty())
        qstr_current_id = qstr_default_id;
    // -------------------------------------------
    if (qstr_current_id.isEmpty() && (OTAPI_Wrap::GetServerCount() > 0))
        qstr_current_id = QString::fromStdString(OTAPI_Wrap::GetServer_ID(0));
    // -------------------------------------------
    // Select from Servers in local wallet.
    //
    DlgChooser theChooser(this);
    // -----------------------------------------------
    mapIDName & the_map = theChooser.m_map;

    bool bFoundDefault = false;
    // -----------------------------------------------
    const int32_t the_count = OTAPI_Wrap::GetServerCount();
    // -----------------------------------------------
    for (int32_t ii = 0; ii < the_count; ++ii)
    {
        QString OT_id = QString::fromStdString(OTAPI_Wrap::GetServer_ID(ii));
        QString OT_name("");
        // -----------------------------------------------
        if (!OT_id.isEmpty())
        {
            if (!qstr_current_id.isEmpty() && (OT_id == qstr_current_id))
                bFoundDefault = true;
            // -----------------------------------------------
            OT_name = QString::fromStdString(OTAPI_Wrap::GetServer_Name(OT_id.toStdString()));
            // -----------------------------------------------
            the_map.insert(OT_id, OT_name);
        }
     }
    // -----------------------------------------------
    if (bFoundDefault && !qstr_current_id.isEmpty())
        theChooser.SetPreSelected(qstr_current_id);
    // -----------------------------------------------
    theChooser.setWindowTitle(tr("Select the Server"));
    // -----------------------------------------------
    if (theChooser.exec() == QDialog::Accepted)
    {
        if (!theChooser.m_qstrCurrentID  .isEmpty() &&
            !theChooser.m_qstrCurrentName.isEmpty())
        {
            ui->lineEditID->setText(theChooser.m_qstrCurrentID);
            ui->lineEditID->home(false);
            // -----------------------------------------
            ui->pushButtonSelect->setText(theChooser.m_qstrCurrentName);
            // -----------------------------------------
            if (qstr_default_id.isEmpty())
            {
                emit SetDefaultServer(theChooser.m_qstrCurrentID, theChooser.m_qstrCurrentName);
            }
            // -----------------------------------------
            return;
        }
    }
    else
    {
      qDebug() << "CANCEL was clicked";
    }
    // -----------------------------------------------
    ui->pushButtonSelect->setText(QString("<%1>").arg(tr("Click to choose Server")));
    ui->lineEditID->setText("");
    // -------------------------------------------
}


//virtual
void MTPageAcct_Server::showEvent(QShowEvent * event)
{
    // -------------------------------
    // call inherited method
    //
    QWizardPage::showEvent(event);
    // -------------------------------
//    if (m_bFirstRun)
    {
        m_bFirstRun = false;
        // --------------------------------------------------
        std::string str_name;
        QString     qstr_id;
        // -------------------------------------------
        QString qstr_default_id;
        // -------------------------------------------
        QWizard            * pWizard           = this->wizard();
        MTWizardAddAccount * pWizardAddAccount = (MTWizardAddAccount *)pWizard;
        // -------------------------------------------
        if (NULL != pWizardAddAccount)
        {
            qstr_default_id = Moneychanger::It()->get_default_server_id();
        }
        // -------------------------------------------
        QString qstr_current_id = ui->lineEditID->text();
        // -------------------------------------------
        if (qstr_current_id.isEmpty())
            qstr_id = qstr_default_id;
        else
            qstr_id = qstr_current_id;
        // -------------------------------------------
        if (qstr_id.isEmpty() && (OTAPI_Wrap::GetServerCount() > 0))
            qstr_id = QString::fromStdString(OTAPI_Wrap::GetServer_ID(0));
        // -------------------------------------------
        if (!qstr_id.isEmpty())
            str_name = OTAPI_Wrap::GetServer_Name(qstr_id.toStdString());
        // -------------------------------------------
        if (str_name.empty() || qstr_id.isEmpty())
        {
            ui->pushButtonSelect->setText(QString("<%1>").arg(tr("Click to choose Server")));
            ui->lineEditID->setText("");
        }
        else
        {
            QString qstrName = QString::fromStdString(str_name);
            ui->pushButtonSelect->setText(qstrName);
            ui->lineEditID->setText(qstr_id);
            ui->lineEditID->home(false);
            // ---------------------------
            if (qstr_default_id.isEmpty())
            {
                emit SetDefaultServer(qstr_id, qstrName);
            }
        }
        // -------------------------------------------
//        if (OTAPI_Wrap::GetServerCount() < 1)
//            on_pushButtonManage_clicked();
    } // first run.
    // -------------------------------------
}

void MTPageAcct_Server::on_pushButtonManage_clicked()
{
    QWizard            * pWizard           = this->wizard();
    MTWizardAddAccount * pWizardAddAccount = (MTWizardAddAccount *)pWizard;
    // -------------------------------------------
    if (NULL != pWizardAddAccount)
    {
        MTDetailEdit * pWindow = new MTDetailEdit(this);

        pWindow->setAttribute(Qt::WA_DeleteOnClose);
        // -------------------------------------
        mapIDName & the_map = pWindow->m_map;
        // -------------------------------------
        the_map.clear();
        // -------------------------------------
        int32_t the_count = OTAPI_Wrap::GetServerCount();
        bool    bStartingWithNone = (the_count < 1);

        for (int32_t ii = 0; ii < the_count; ii++)
        {
            QString OT_id   = QString::fromStdString(OTAPI_Wrap::GetServer_ID(ii));
            QString OT_name = QString::fromStdString(OTAPI_Wrap::GetServer_Name(OT_id.toStdString()));

            the_map.insert(OT_id, OT_name);
        } // for
        // -------------------------------------
        pWindow->setWindowTitle(tr("Manage Servers"));
        // -------------------------------------
        pWindow->dialog(MTDetailEdit::DetailEditTypeServer, true);
        // -------------------------------------
        if (bStartingWithNone && (OTAPI_Wrap::GetServerCount() > 0))
        {
            std::string str_id = OTAPI_Wrap::GetServer_ID(0);

            if (!str_id.empty())
            {
                std::string str_name = OTAPI_Wrap::GetServer_Name(str_id);

                if (str_name.empty())
                    str_name = str_id;
                // --------------------------------
                ui->pushButtonSelect->setText(QString::fromStdString(str_name));
                ui->lineEditID->setText(QString::fromStdString(str_id));
                ui->lineEditID->home(false);
            }
        }
        // -------------------------------------
        else if (OTAPI_Wrap::GetServerCount() < 1)
        {
            ui->pushButtonSelect->setText(QString("<%1>").arg(tr("Click to choose Server")));
            ui->lineEditID->setText("");
        }
    }
    // -------------------------------------------
}


MTPageAcct_Server::~MTPageAcct_Server()
{
    delete ui;
}
