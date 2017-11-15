#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/pageacct_asset.hpp>
#include <ui_pageacct_asset.h>

#include <gui/widgets/dlgchooser.hpp>
#include <gui/widgets/detailedit.hpp>

#include <core/moneychanger.hpp>

#include <opentxs/api/Api.hpp>
#include <opentxs/api/Native.hpp>
#include <opentxs/OT.hpp>
#include <opentxs/client/OTAPI_Exec.hpp>


MTPageAcct_Asset::MTPageAcct_Asset(QWidget *parent) :
    QWizardPage(parent),
    m_bFirstRun(true),
    ui(new Ui::MTPageAcct_Asset)
{
    ui->setupUi(this);

    QString style_sheet = "QPushButton{border: none; border-style: outset; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa);}"
            "QPushButton:pressed {border: 1px solid black; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa); }"
            "QPushButton:hover {border: 1px solid black; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa); }";

    ui->pushButtonSelect->setStyleSheet(style_sheet);

    ui->lineEditID->setStyleSheet("QLineEdit { background-color: lightgray }");

    this->registerField("InstrumentDefinitionID*",  ui->lineEditID);
    this->registerField("AssetName", ui->pushButtonSelect, "text");
    // -----------------------------------------------
    connect(this, SIGNAL(SetDefaultAsset(QString, QString)), Moneychanger::It(), SLOT(setDefaultAsset(QString,QString)));
}


void MTPageAcct_Asset::on_pushButtonSelect_clicked()
{
    // --------------------------------------------------
    QString     qstr_default_id = Moneychanger::It()->get_default_asset_id();
    // -------------------------------------------
    QString     qstr_current_id = field("InstrumentDefinitionID").toString();
    // -------------------------------------------
    if (qstr_current_id.isEmpty())
        qstr_current_id = qstr_default_id;
    // -------------------------------------------
    if (qstr_current_id.isEmpty() && (opentxs::OT::App().API().Exec().GetAssetTypeCount() > 0))
        qstr_current_id = QString::fromStdString(opentxs::OT::App().API().Exec().GetAssetType_ID(0));
    // -------------------------------------------
    // Select from Asset Types in local wallet.
    //
    DlgChooser theChooser(this);
    // -----------------------------------------------
    mapIDName & the_map = theChooser.m_map;

    bool bFoundDefault = false;
    // -----------------------------------------------
    const int32_t the_count = opentxs::OT::App().API().Exec().GetAssetTypeCount();
    // -----------------------------------------------
    for (int32_t ii = 0; ii < the_count; ++ii)
    {
        QString OT_id = QString::fromStdString(opentxs::OT::App().API().Exec().GetAssetType_ID(ii));
        QString OT_name("");
        // -----------------------------------------------
        if (!OT_id.isEmpty())
        {
            if (!qstr_current_id.isEmpty() && (0 == qstr_current_id.compare(OT_id)))
                bFoundDefault = true;
            // -----------------------------------------------
            OT_name = QString::fromStdString(opentxs::OT::App().API().Exec().GetAssetType_Name(OT_id.toStdString()));
            // -----------------------------------------------
            the_map.insert(OT_id, OT_name);
        }
     }
    // -----------------------------------------------
    if (bFoundDefault)
        theChooser.SetPreSelected(qstr_current_id);
    // -----------------------------------------------
    theChooser.setWindowTitle(tr("Select the Asset Type"));
    // -----------------------------------------------
    if (theChooser.exec() == QDialog::Accepted)
    {
        if (!theChooser.m_qstrCurrentID  .isEmpty() &&
            !theChooser.m_qstrCurrentName.isEmpty())
        {
            setField("InstrumentDefinitionID",   theChooser.m_qstrCurrentID);
            setField("AssetName", theChooser.m_qstrCurrentName);
            // -----------------------------------------
            ui->lineEditID->home(false);
            // -----------------------------------------
            if (qstr_default_id.isEmpty())
                emit SetDefaultAsset(theChooser.m_qstrCurrentID, theChooser.m_qstrCurrentName);
            // -----------------------------------------
            return;
        }
    }
    // -----------------------------------------------
}


//virtual
void MTPageAcct_Asset::initializePage()
{
    if (!Moneychanger::It()->expertMode())
    {
        ui->pushButtonManage->setVisible(false);
    }
    // -------------------------------------------
    std::string str_name;
    QString     qstr_id;
    // -------------------------------------------
    QString     qstr_default_id = Moneychanger::It()->get_default_asset_id();
    // -------------------------------------------
    QString qstr_current_id = field("InstrumentDefinitionID").toString();
    // -------------------------------------------
    qstr_id = qstr_current_id.isEmpty() ? qstr_default_id : qstr_current_id;
    // -------------------------------------------
    if (qstr_id.isEmpty() && (opentxs::OT::App().API().Exec().GetAssetTypeCount() > 0))
        qstr_id = QString::fromStdString(opentxs::OT::App().API().Exec().GetAssetType_ID(0));
    // -------------------------------------------
    if (!qstr_id.isEmpty())
        str_name = opentxs::OT::App().API().Exec().GetAssetType_Name(qstr_id.toStdString());
    // -------------------------------------------
    if (str_name.empty() || qstr_id.isEmpty())
        SetFieldsBlank();
    else
    {
        QString qstrName = QString::fromStdString(str_name);
        // ---------------------------
        setField("InstrumentDefinitionID",   qstr_id);
        setField("AssetName", qstrName);
        // ---------------------------
        ui->lineEditID->home(false);
        // ---------------------------
        if (qstr_default_id.isEmpty())
            emit SetDefaultAsset(qstr_id, qstrName);
    }
}


void MTPageAcct_Asset::on_pushButtonManage_clicked()
{
    MTDetailEdit * pWindow = new MTDetailEdit(this);

    pWindow->setAttribute(Qt::WA_DeleteOnClose);
    // -------------------------------------
    mapIDName & the_map = pWindow->m_map;
    // -------------------------------------
    the_map.clear();
    // -------------------------------------
    QString qstrPreSelected   = field("InstrumentDefinitionID").toString();
    bool    bFoundPreselected = false;
    // -------------------------------------
    int32_t the_count = opentxs::OT::App().API().Exec().GetAssetTypeCount();
    bool    bStartingWithNone = (the_count < 1);

    for (int32_t ii = 0; ii < the_count; ii++)
    {
        QString OT_id   = QString::fromStdString(opentxs::OT::App().API().Exec().GetAssetType_ID(ii));
        QString OT_name = QString::fromStdString(opentxs::OT::App().API().Exec().GetAssetType_Name(OT_id.toStdString()));

        the_map.insert(OT_id, OT_name);

        if (!qstrPreSelected.isEmpty() && (0 == qstrPreSelected.compare(OT_id)))
            bFoundPreselected = true;
    } // for
    // -------------------------------------
    pWindow->setWindowTitle(tr("Manage Asset Types"));
    // -------------------------------------
    if (bFoundPreselected)
        pWindow->SetPreSelected(qstrPreSelected);
    // -------------------------------------
    pWindow->dialog(MTDetailEdit::DetailEditTypeAsset, true);
    // -------------------------------------
    if (bStartingWithNone && (opentxs::OT::App().API().Exec().GetAssetTypeCount() > 0))
    {
        std::string str_id = opentxs::OT::App().API().Exec().GetAssetType_ID(0);

        if (!str_id.empty())
        {
            std::string str_name = opentxs::OT::App().API().Exec().GetAssetType_Name(str_id);

            if (str_name.empty())
                str_name = str_id;
            // --------------------------------
            setField("InstrumentDefinitionID",   QString::fromStdString(str_id));
            setField("AssetName", QString::fromStdString(str_name));
            // --------------------------------
            ui->lineEditID->home(false);
        }
    }
    // -------------------------------------
    else if (opentxs::OT::App().API().Exec().GetAssetTypeCount() < 1)
        SetFieldsBlank();
    // -------------------------------------------
}

void MTPageAcct_Asset::SetFieldsBlank()
{
    setField("InstrumentDefinitionID",   "");
    setField("AssetName", QString("<%1>").arg(tr("Click to choose Asset Type")));
}

MTPageAcct_Asset::~MTPageAcct_Asset()
{
    delete ui;
}

