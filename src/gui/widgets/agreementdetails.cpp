#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/agreementdetails.hpp>
#include <ui_agreementdetails.h>

#include <gui/ui/getstringdialog.hpp>
#include <gui/ui/dlgcreatevariable.hpp>
#include <gui/widgets/dlgchooser.hpp>

#include <gui/widgets/wizardaddcontract.hpp>

#include <gui/widgets/overridecursor.hpp>

#include <core/moneychanger.hpp>
#include <core/handlers/contacthandler.hpp>

#include <opentxs/opentxs.hpp>

#include <QMessageBox>
#include <QClipboard>
#include <QDebug>
#include <QHBoxLayout>
#include <QButtonGroup>

MTAgreementDetails::MTAgreementDetails(QWidget *parent, MTDetailEdit & theOwner) :
    MTEditDetails(parent, theOwner),
    m_pHeaderWidget(NULL),
    ui(new Ui::MTAgreementDetails)
{
    ui->setupUi(this);
    this->setContentsMargins(0, 0, 0, 0);
//  this->installEventFilter(this); // NOTE: Successfully tested theory that the base class has already installed this.

    ui->lineEditID->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditContractID->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditLanguage->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditAccountName->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditAgentName->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditBylawName->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditCallbackClause->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditCallbackName->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditClauseName->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditPartyName->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditVariableName->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditVariableValue->setStyleSheet("QLineEdit { background-color: lightgray }");

    ui->lineEditAssetID->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditAssetName->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditPartyNymID->setStyleSheet("QLineEdit { background-color: lightgray }");
    // ----------------------------------
    // Note: This is a placekeeper, so later on I can just erase
    // the widget at 0 and replace it with the real header widget.
    //
    m_pHeaderWidget  = new QWidget;
    ui->verticalLayout->insertWidget(0, m_pHeaderWidget);
    // ----------------------------------
    ui->tableWidgetBylaws->setSelectionMode    (QAbstractItemView::SingleSelection);
    ui->tableWidgetBylaws->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidgetBylaws->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidgetBylaws->verticalHeader()->hide();
    // ----------------------------------
    ui->tableWidgetParties->setSelectionMode    (QAbstractItemView::SingleSelection);
    ui->tableWidgetParties->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidgetParties->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidgetParties->verticalHeader()->hide();
    // ----------------------------------
    ui->comboBoxVariableAccess->addItem(QString(tr("Constant")),   QVariant(QString("constant")));
    ui->comboBoxVariableAccess->addItem(QString(tr("Persistent")), QVariant(QString("persistent")));
    ui->comboBoxVariableAccess->addItem(QString(tr("Important")),  QVariant(QString("important")));
    // ----------------------------------
    ui->comboBoxVariableValue->addItem(QString(tr("True")),  QVariant(QString("true")));
    ui->comboBoxVariableValue->addItem(QString(tr("False")), QVariant(QString("false")));
    // ----------------------------------
    ui->comboBoxVariableType->addItem(QString(tr("String")),   QVariant(QString("string")));
    ui->comboBoxVariableType->addItem(QString(tr("Long Int")), QVariant(QString("integer")));
    ui->comboBoxVariableType->addItem(QString(tr("Boolean")),  QVariant(QString("bool")));
    // ----------------------------------
    ui->comboBoxVariableValue ->setCurrentIndex(0); // I think this is "true".
    ui->comboBoxVariableAccess->setCurrentIndex(1); // 1 is "persistent"
    ui->comboBoxVariableType  ->setCurrentIndex(0); // 0 is "string" (I think.)
    // ----------------------------------
    ui->labelControllingAgent->setVisible(false);
    ui->comboBoxControllingAgent->setVisible(false);
    // ----------------------------------
    QVBoxLayout * vBox = new QVBoxLayout;
    ui->groupBoxHooks->setLayout(vBox);
    // ----------------------------------
    ui->buttonGroupValidFrom->setId(ui->radioButtonValidFrom1, -2);
    ui->buttonGroupValidFrom->setId(ui->radioButtonValidFrom2, -3);
    // ----------------------------------
    ui->buttonGroupValidTo->setId(ui->radioButtonValidTo1, -2);
    ui->buttonGroupValidTo->setId(ui->radioButtonValidTo2, -3);
    // ----------------------------------
    connect(ui->buttonGroupValidFrom, SIGNAL(buttonClicked(int)), this, SLOT(onGroupButtonValidFromClicked(int)));
    connect(ui->buttonGroupValidTo,   SIGNAL(buttonClicked(int)), this, SLOT(onGroupButtonValidToClicked(int)));
}

// m_pOwner->GetLawyerID();


void MTAgreementDetails::ClearBylaws()
{
    if (NULL != ui)
    {
        ClearBylawMap();
        ClearBylawsGrid();

        // This may be moved soon.
        ui->comboBoxVariableValue ->setCurrentIndex(0); // I think this is "true".
        ui->comboBoxVariableAccess->setCurrentIndex(1); // 1 is "persistent"
        ui->comboBoxVariableType  ->setCurrentIndex(0); // 0 is "string" (I think.)
    }
}

void MTAgreementDetails::ClearParties()
{
    if (NULL != ui)
    {
        ClearPartyMap();
        ClearPartiesGrid();
    }
}

void MTAgreementDetails::ClearBylawMap()
{
//    QMap<QString, QVariant> temp_map = m_mapBylaws;

    m_mapBylaws.clear();
    // ------------------------------------
//    for (QMap<QString, QVariant>::iterator it_map = temp_map.begin();
//         it_map != temp_map.end(); ++it_map)
//    {
        // TODO
//        opentxs::OTDB::OfferDataNym * pData = VPtr<opentxs::OTDB::OfferDataNym>::asPtr(it_map.value());

//        if (NULL != pData) // should never be NULL.
//            delete pData;
//    }
    // --------------------
//    temp_map.clear();
}

void MTAgreementDetails::ClearPartyMap()
{
//    QMap<QString, QVariant> temp_map = m_mapParties;

    m_mapParties.clear();
    // ------------------------------------
//    for (QMap<QString, QVariant>::iterator it_map = temp_map.begin();
//         it_map != temp_map.end(); ++it_map)
//    {
        // TODO
//        opentxs::OTDB::OfferDataNym * pData = VPtr<opentxs::OTDB::OfferDataNym>::asPtr(it_map.value());

//        if (NULL != pData) // should never be NULL.
//            delete pData;
//    }
    // --------------------
//    temp_map.clear();
}


void MTAgreementDetails::ClearBylawsGrid()
{
    ui->tableWidgetBylaws->blockSignals(true);
    // -----------------------------------
    ui->tableWidgetBylaws->clearContents();
    ui->tableWidgetBylaws->setRowCount (0);
    // -----------------------------------
    ui->tableWidgetBylaws->setCurrentCell(-1, 0);
    ui->tableWidgetBylaws->blockSignals(false);

    on_tableWidgetBylaws_currentCellChanged(-1, 0, 0, 0);

    // Sometimes setCurrentCell WOULDN'T trigger currentCellChanged.
    // Like in the case where we're refreshing the smart contract details as we select a different
    // contract. In that case, both smart contracts had 1 party, at index 0, and thus the index for
    // parties didn't actually change. It went from row 0 to row 0. Therefore, when I set the cell
    // here myself by hand, I block the signals, set the cell, and then call currentCellChanged by
    // HAND as well. I made this same fix in various places.
    // In this function the row was going from -1 to -1 so there was no "change."
}

void MTAgreementDetails::ClearPartiesGrid()
{
    ui->tableWidgetParties->blockSignals(true);
    // -----------------------------------
    ui->tableWidgetParties->clearContents();
    ui->tableWidgetParties->setRowCount (0);
    // -----------------------------------
    ui->tableWidgetParties->setCurrentCell(-1, 0);
    ui->tableWidgetParties->blockSignals(false);

    on_tableWidgetParties_currentCellChanged(-1, 0, 0, 0);

    // Sometimes setCurrentCell WOULDN'T trigger currentCellChanged.
    // Like in the case where we're refreshing the smart contract details as we select a different
    // contract. In that case, both smart contracts had 1 party, at index 0, and thus the index for
    // parties didn't actually change. It went from row 0 to row 0. Therefore, when I set the cell
    // here myself by hand, I block the signals, set the cell, and then call currentCellChanged by
    // HAND as well. I made this same fix in various places.
    // In this function the row was going from -1 to -1 so there was no "change."
}

void MTAgreementDetails::ClearContents()
{
    if (NULL != ui)
    {
        m_qstrTemplate = QString("");
        // ------------------------------------------
        ui->lineEditID        ->setText("");
        ui->lineEditName      ->setText("");
        ui->lineEditContractID->setText("");
        // ------------------------------------------
        ui->checkBoxSpecifyAsset->setChecked(true);
        ui->checkBoxSpecifyNym  ->setChecked(true);
        // ------------------------------------------
        if (m_pPlainTextEdit)
            m_pPlainTextEdit->setPlainText("");
        // ------------------------------------------
        time64_t dateFrom = 0;
        time64_t dateTo   = 0;

        m_dateTimeValidFrom = QDateTime::fromTime_t(dateFrom);
        m_dateTimeValidTo   = QDateTime::fromTime_t(dateTo);

        ui->dateTimeEditValidFrom->blockSignals(true);
        ui->dateTimeEditValidFrom->setDateTime(m_dateTimeValidFrom);
        ui->dateTimeEditValidFrom->blockSignals(false);
        ui->dateTimeEditValidFrom->setEnabled(false);

        ui->dateTimeEditValidTo  ->blockSignals(true);
        ui->dateTimeEditValidTo  ->setDateTime(m_dateTimeValidTo);
        ui->dateTimeEditValidTo  ->blockSignals(false);
        ui->dateTimeEditValidTo  ->setEnabled(false);

        ui->radioButtonValidFrom1->blockSignals(true);
        ui->radioButtonValidFrom1->setChecked(true);
        ui->radioButtonValidFrom1->blockSignals(false);
        ui->radioButtonValidTo1->blockSignals(true);
        ui->radioButtonValidTo1->setChecked(true);
        ui->radioButtonValidTo1->blockSignals(false);

        ClearBylaws();
        ClearParties();
    }

//    // ------------------------------------------
//    if (m_pCredentials)
//        m_pCredentials->ClearContents();
//    // ------------------------------------------
//    if (m_pPlainTextEdit)
//        m_pPlainTextEdit->setPlainText("");
//    // ------------------------------------------
//    if (m_pAddresses)
//    {
//        ui->verticalLayout->removeWidget(m_pAddresses);

//        m_pAddresses->setParent(NULL);
//        m_pAddresses->disconnect();
//        m_pAddresses->deleteLater();

//        m_pAddresses = NULL;
//    }
}


void MTAgreementDetails::FavorLeftSideForIDs()
{
    if (NULL != ui)
    {
        ui->lineEditID        ->home(false);
        ui->lineEditName      ->home(false);
        ui->lineEditContractID->home(false);
        ui->lineEditLanguage  ->home(false);
        ui->lineEditAssetID   ->home(false);
        ui->lineEditAssetName ->home(false);
        ui->lineEditPartyNymID->home(false);
    }
}


void MTAgreementDetails::refresh(QString strID, QString strName)
{
    if ((NULL != ui) && !strID.isEmpty())
    {
        QWidget * pHeaderWidget  = MTEditDetails::CreateDetailHeaderWidget(m_Type, strID, strName, "", "", ":/icons/icons/smart_contract_64.png", false);

        pHeaderWidget->setObjectName(QString("DetailHeader")); // So the stylesheet doesn't get applied to all its sub-widgets.

        if (m_pHeaderWidget)
        {
            ui->verticalLayout->removeWidget(m_pHeaderWidget);

            m_pHeaderWidget->setParent(NULL);
            m_pHeaderWidget->disconnect();
            m_pHeaderWidget->deleteLater();

            m_pHeaderWidget = NULL;
        }
        ui->verticalLayout->insertWidget(0, pHeaderWidget);
        m_pHeaderWidget = pHeaderWidget;
        // ----------------------------------
        ui->lineEditID  ->setText(strID);
        ui->lineEditName->setText(strName);
        // --------------------------
        // Look up a copy of the smart contract from the local database.
        //
        int nTemplateID = strID.toInt();
        // ------------------------------------------
        if (nTemplateID > 0)
            m_qstrTemplate = MTContactHandler::getInstance()->GetSmartContract(nTemplateID);
        else
            m_qstrTemplate = QString("");
        // ------------------------------------------
        if (!m_qstrTemplate.isEmpty())
        {
            std::string str_template = m_qstrTemplate.toStdString();
            // ----------------------------------

            // Show stuff, and populate it?

            if (m_pPlainTextEdit)
                m_pPlainTextEdit->setPlainText(m_qstrTemplate);
            // ------------------------------------------------
            std::string contract_id = Moneychanger::It()->OT().Exec().CalculateContractID(str_template);
            ui->lineEditContractID->setText(QString::fromStdString(contract_id));
            // ------------------------------------------------
            time64_t dateFrom = Moneychanger::It()->OT().Exec().Instrmnt_GetValidFrom(str_template);
            time64_t dateTo   = Moneychanger::It()->OT().Exec().Instrmnt_GetValidTo(str_template);

            m_dateTimeValidFrom = QDateTime::fromTime_t(dateFrom);
            m_dateTimeValidTo   = QDateTime::fromTime_t(dateTo);

            ui->dateTimeEditValidFrom->blockSignals(true);
            ui->dateTimeEditValidFrom->setDateTime(m_dateTimeValidFrom);
            ui->dateTimeEditValidFrom->blockSignals(false);

            ui->dateTimeEditValidTo  ->blockSignals(true);
            ui->dateTimeEditValidTo  ->setDateTime(m_dateTimeValidTo);
            ui->dateTimeEditValidTo  ->blockSignals(false);

            bool bSpecifyAsset = Moneychanger::It()->OT().Exec().Smart_AreAssetTypesSpecified(str_template);
            bool bSpecifyNym   = Moneychanger::It()->OT().Exec().Smart_ArePartiesSpecified(str_template);
            // ------------------------------------------------
            if (0 == dateFrom)
            {
                ui->radioButtonValidFrom1->blockSignals(true);
                ui->radioButtonValidFrom1->setChecked(true);
                ui->radioButtonValidFrom1->blockSignals(false);

                ui->dateTimeEditValidFrom->setEnabled(false);
            }
            else
            {
                ui->radioButtonValidFrom2->blockSignals(true);
                ui->radioButtonValidFrom2->setChecked(true);
                ui->radioButtonValidFrom2->blockSignals(false);

                ui->dateTimeEditValidFrom->setEnabled(true);
            }
            // ------------------------------------------------
            if (0 == dateTo)
            {
                ui->radioButtonValidTo1->blockSignals(true);
                ui->radioButtonValidTo1->setChecked(true);
                ui->radioButtonValidTo1->blockSignals(false);

                ui->dateTimeEditValidTo->setEnabled(false);
            }
            else
            {
                ui->radioButtonValidTo2->blockSignals(true);
                ui->radioButtonValidTo2->setChecked(true);
                ui->radioButtonValidTo2->blockSignals(false);

                ui->dateTimeEditValidTo->setEnabled(true);
            }
            // ------------------------------------------------
            ui->checkBoxSpecifyAsset->blockSignals(true);
            ui->checkBoxSpecifyAsset->setChecked(bSpecifyAsset);
            ui->checkBoxSpecifyAsset->blockSignals(false);
            // ------------------------------------------------
            ui->checkBoxSpecifyNym->blockSignals(true);
            ui->checkBoxSpecifyNym->setChecked(bSpecifyNym);
            ui->checkBoxSpecifyNym->blockSignals(false);
            // ------------------------------------------------
            PopulateBylaws();
            PopulateParties();
            // --------------------------
            FavorLeftSideForIDs();
            // --------------------------

        }
        else
        {
           // Otherwise do nothing, since everything was already cleared? Or hide some stuff here?

            ui->lineEditContractID->setText("");

            time64_t dateFrom = 0;
            time64_t dateTo   = 0;

            m_dateTimeValidFrom = QDateTime::fromTime_t(dateFrom);
            m_dateTimeValidTo   = QDateTime::fromTime_t(dateTo);

            ui->dateTimeEditValidFrom->blockSignals(true);
            ui->dateTimeEditValidFrom->setDateTime(m_dateTimeValidFrom);
            ui->dateTimeEditValidFrom->blockSignals(false);
            ui->dateTimeEditValidFrom->setEnabled(false);

            ui->dateTimeEditValidTo  ->blockSignals(true);
            ui->dateTimeEditValidTo  ->setDateTime(m_dateTimeValidTo);
            ui->dateTimeEditValidTo  ->blockSignals(false);
            ui->dateTimeEditValidTo  ->setEnabled(false);

            ui->radioButtonValidFrom1->blockSignals(true);
            ui->radioButtonValidFrom1->setChecked(true);
            ui->radioButtonValidFrom1->blockSignals(false);
            ui->radioButtonValidTo1->blockSignals(true);
            ui->radioButtonValidTo1->setChecked(true);
            ui->radioButtonValidTo1->blockSignals(false);
            // ------------------------------------------------
            ui->checkBoxSpecifyAsset->blockSignals(true);
            ui->checkBoxSpecifyAsset->setChecked(true);
            ui->checkBoxSpecifyAsset->blockSignals(false);
            // ------------------------------------------------
            ui->checkBoxSpecifyNym->blockSignals(true);
            ui->checkBoxSpecifyNym->setChecked(true);
            ui->checkBoxSpecifyNym->blockSignals(false);
            // ------------------------------------------------
            ClearBylaws();
            ClearParties();
        }
        // ------------------------------------------
    }
}



void MTAgreementDetails::PopulateBylaws()
{
    ClearBylawMap();

    if (m_qstrTemplate.isEmpty())
        return;
    // ----------------------------------
    std::string str_template = m_qstrTemplate.toStdString();
    // ----------------------------------
    const int32_t nCount = Moneychanger::It()->OT().Exec().Smart_GetBylawCount(str_template);

    for (int32_t nIndex = 0; nIndex < nCount; ++nIndex)
    {
        std::string str_bylaw = Moneychanger::It()->OT().Exec().Smart_GetBylawByIndex(str_template, nIndex);

        m_mapBylaws.insert(QString::fromStdString(str_bylaw), QString::fromStdString(str_bylaw));
    }
    // ----------------------------------
    PopulateBylawGrid();
}

void MTAgreementDetails::PopulateBylawGrid()
{
    if (m_qstrTemplate.isEmpty())
        return;
    // ----------------------------------
    std::string str_template = m_qstrTemplate.toStdString();
    // ----------------------------------
    ui->tableWidgetBylaws->blockSignals(true);
    // -----------------------------------
    int nGridIndex = 0;
    int nRowCount  = m_mapBylaws.size();
    // -------------------------------------
    ui->tableWidgetBylaws->setRowCount(nRowCount);
    // -----------------------------------
    for (mapIDName::iterator it_top = m_mapBylaws.begin(); it_top != m_mapBylaws.end(); ++it_top)
    {
        QString qstrName           = it_top.key();
        std::string str_name       = qstrName.toStdString();
        std::string str_lang       = Moneychanger::It()->OT().Exec().Bylaw_GetLanguage(str_template, str_name);
        int32_t     nVariableCount = Moneychanger::It()->OT().Exec().Bylaw_GetVariableCount(str_template, str_name);
        int32_t     nClauseCount   = Moneychanger::It()->OT().Exec().Bylaw_GetClauseCount(str_template, str_name);
        int32_t     nHookCount     = Moneychanger::It()->OT().Exec().Bylaw_GetHookCount(str_template, str_name);
        int32_t     nCallbackCount = Moneychanger::It()->OT().Exec().Bylaw_GetCallbackCount(str_template, str_name);
        // -----------------------------------------------------------------------
        QLabel * pLabelName          = new QLabel(QString::fromStdString(str_name));
        QLabel * pLabelLanguage      = new QLabel(QString::fromStdString(str_lang));
        QLabel * pLabelVarCount      = new QLabel(QString("%1").arg(nVariableCount));
        QLabel * pLabelClauseCount   = new QLabel(QString("%1").arg(nClauseCount));
        QLabel * pLabelHookCount     = new QLabel(QString("%1").arg(nHookCount));
        QLabel * pLabelCallbackCount = new QLabel(QString("%1").arg(nCallbackCount));
        // -----------------------------------------------------------------------
        pLabelName         ->setAlignment(Qt::AlignCenter);
        pLabelLanguage     ->setAlignment(Qt::AlignCenter);
        pLabelVarCount     ->setAlignment(Qt::AlignCenter);
        pLabelClauseCount  ->setAlignment(Qt::AlignCenter);
        pLabelHookCount    ->setAlignment(Qt::AlignCenter);
        pLabelCallbackCount->setAlignment(Qt::AlignCenter);
        // -----------------------------------------------------------------------
        ui->tableWidgetBylaws->setCellWidget ( nGridIndex, 0, pLabelName         );
        ui->tableWidgetBylaws->setCellWidget ( nGridIndex, 1, pLabelLanguage     );
        ui->tableWidgetBylaws->setCellWidget ( nGridIndex, 2, pLabelVarCount     );
        ui->tableWidgetBylaws->setCellWidget ( nGridIndex, 3, pLabelClauseCount  );
        ui->tableWidgetBylaws->setCellWidget ( nGridIndex, 4, pLabelHookCount    );
        ui->tableWidgetBylaws->setCellWidget ( nGridIndex, 5, pLabelCallbackCount);
        // -----------------------------------------------------------------------
        ++nGridIndex;
    }
    // -----------------------------------------------------
    if (ui->tableWidgetBylaws->rowCount() > 0)
    {
        ui->tableWidgetBylaws->setCurrentCell(0, 0);
        ui->tableWidgetBylaws->blockSignals(false);
        on_tableWidgetBylaws_currentCellChanged(0, 0, 0, 0); // Note: see comment in following block.

        // Sometimes setCurrentCell WOULDN'T trigger currentCellChanged.
        // Like in the case where we're refreshing the smart contract details as we select a different
        // contract. In that case, both smart contracts had 1 party, at index 0, and thus the index for
        // parties didn't actually change. It went from row 0 to row 0. Therefore, when I set the cell
        // here myself by hand, I block the signals, set the cell, and then call currentCellChanged by
        // HAND as well. I made this same fix in various places.
    }
    else
    {
        ui->tableWidgetBylaws->setCurrentCell(-1, 0);
        ui->tableWidgetBylaws->blockSignals(false);
        on_tableWidgetBylaws_currentCellChanged(-1, 0, 0, 0);

        // NOTE: Why do I call currentCellChanged by hand here?
        // Because in the case where the tableWidget is empty,
        // setCurrentCell somehow DOESN'T automatically trigger
        // currentCellChanged, even though it DOES work when the
        // tableWidget has items in it.
        //
        // UPDATE: Ahh I see why: since the grid was already empty,
        // the current row was -1, so setting it to -1 again doesn't
        // CHANGE it, so therefore, no need for "currentCellChanged" event.
        // This can even happen in the case where it's populated (like if
        // it goes from 0 to 0.)
    }
    // -----------------------------------------------------
}


void MTAgreementDetails::on_tableWidgetBylaws_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    Q_UNUSED(currentColumn);
    Q_UNUSED(previousRow);
    Q_UNUSED(previousColumn);

    if (NULL != ui)
    {
        if ((-1) == currentRow)
            ui->tabWidgetBylaw->setVisible(false);
        else
            ui->tabWidgetBylaw->setVisible(true);
        // -------------------
        RefreshBylawDetails(currentRow);
    }
}

void MTAgreementDetails::on_tableWidgetParties_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    Q_UNUSED(currentColumn);
    Q_UNUSED(previousRow);
    Q_UNUSED(previousColumn);

    if (NULL != ui)
    {
        if ((-1) == currentRow)
            ui->tabWidgetParty->setVisible(false);
        else
            ui->tabWidgetParty->setVisible(true);
        // -------------------
        RefreshPartyDetails(currentRow);
    }
}

// Bylaw
void MTAgreementDetails::on_listWidgetVariables_currentRowChanged(int currentRow)
{
    if ((-1) == currentRow)
        ui->groupBoxVariables->setEnabled(false);
    else
        ui->groupBoxVariables->setEnabled(true);
    // -------------------
    RefreshVariableDetails(currentRow);
}

// Bylaw
void MTAgreementDetails::on_listWidgetClauses_currentRowChanged(int currentRow)
{
    if ((-1) == currentRow)
        ui->groupBoxClauses->setEnabled(false);
    else
        ui->groupBoxClauses->setEnabled(true);
    // -------------------
    RefreshClauseDetails(currentRow);
}

// Bylaw
void MTAgreementDetails::on_listWidgetHooks_currentRowChanged(int currentRow)
{
    if ((-1) == currentRow)
        ui->groupBoxHooks->setEnabled(false);
    else
        ui->groupBoxHooks->setEnabled(true);
    // -------------------
    RefreshHookDetails(currentRow);
}

// Bylaw
void MTAgreementDetails::on_listWidgetCallbacks_currentRowChanged(int currentRow)
{
    if ((-1) == currentRow)
        ui->groupBoxCallbacks->setEnabled(false);
    else
        ui->groupBoxCallbacks->setEnabled(true);
    // -------------------
    RefreshCallbackDetails(currentRow);
}


// Party
void MTAgreementDetails::on_listWidgetAgents_currentRowChanged(int currentRow)
{
    if ((-1) == currentRow)
        ui->groupBoxAgents->setEnabled(false);
    else
        ui->groupBoxAgents->setEnabled(true);
    // -------------------
    RefreshAgentDetails(currentRow);
}

// Party
void MTAgreementDetails::on_listWidgetAccounts_currentRowChanged(int currentRow)
{
    if ((-1) == currentRow)
        ui->groupBoxAccounts->setEnabled(false);
    else
        ui->groupBoxAccounts->setEnabled(true);
    // -------------------
    RefreshAccountDetails(currentRow);
}

void MTAgreementDetails::RefreshAgentDetails(int nRow)
{
    std::string party_name;

    if (ui->tableWidgetParties->currentRow() >= 0)
    {
        if (m_qstrTemplate.isEmpty())
            return;
        // ----------------------------------
        std::string str_template = m_qstrTemplate.toStdString();
        // ----------------------------------
        QWidget * widget = ui->tableWidgetParties->cellWidget(ui->tableWidgetParties->currentRow(), 0); // 0 is column for the name.
        QLabel  * label  = dynamic_cast<QLabel*>(widget);

        if (NULL != label)
        {
            party_name = label->text().toStdString();
            // ---------------------------------------
            if (nRow >= 0)
            {
                QListWidgetItem * item = ui->listWidgetAgents->item(nRow);

                if (NULL != item)
                {
                    std::string str_name = item->text().toStdString();

                    ui->lineEditAgentName->setText(QString::fromStdString(str_name));
                }
            }
            else
            {
                ui->lineEditAgentName->setText(QString(""));
            }
        }
    }
}


void MTAgreementDetails::RefreshAccountDetails(int nRow)
{
    std::string party_name;

    if (ui->tableWidgetParties->currentRow() >= 0)
    {
        if (m_qstrTemplate.isEmpty())
            return;
        // ----------------------------------
        std::string str_template = m_qstrTemplate.toStdString();
        // ----------------------------------
        QWidget * widget = ui->tableWidgetParties->cellWidget(ui->tableWidgetParties->currentRow(), 0); // 0 is column for the name.
        QLabel  * label  = dynamic_cast<QLabel*>(widget);

        if (NULL != label)
        {
            party_name = label->text().toStdString();
            // ---------------------------------------
            if (nRow >= 0)
            {
                QListWidgetItem * item = ui->listWidgetAccounts->item(nRow);

                if (NULL != item)
                {
                    std::string str_name = item->text().toStdString();

                    ui->lineEditAccountName->setText(QString::fromStdString(str_name));

                    ui->groupBoxAssetType->setVisible(ui->checkBoxSpecifyAsset->isChecked());

                    if (ui->checkBoxSpecifyAsset->isChecked())
                    {
                        std::string str_asset = Moneychanger::It()->OT().Exec().Party_GetAcctInstrumentDefinitionID(str_template, party_name, str_name);
                        ui->lineEditAssetID->setText(QString::fromStdString(str_asset));
                        ui->lineEditAssetName->setText(QString::fromStdString(Moneychanger::It()->OT().Exec().GetAssetType_Name(str_asset)));
                    }
                    else
                    {
                        ui->lineEditAssetID->setText(QString(""));
                        ui->lineEditAssetName->setText(QString(""));
                    }

                    ui->pushButtonDeleteAccount->setEnabled(true);
                }
            }
            else
            {
                ui->lineEditAccountName->setText(QString(""));

                ui->groupBoxAssetType->setVisible(ui->checkBoxSpecifyAsset->isChecked());

                ui->lineEditAssetID->setText(QString(""));
                ui->lineEditAssetName->setText(QString(""));

                ui->pushButtonDeleteAccount->setEnabled(false);
            }
        }
    }
}

void MTAgreementDetails::RefreshVariableDetails(int nRow)
{
    std::string bylaw_name;

    if (ui->tableWidgetBylaws->currentRow() >= 0)
    {
        if (m_qstrTemplate.isEmpty())
            return;
        // ----------------------------------
        std::string str_template = m_qstrTemplate.toStdString();
        // ----------------------------------
        QWidget * widget = ui->tableWidgetBylaws->cellWidget(ui->tableWidgetBylaws->currentRow(), 0); // 0 is column for the name.
        QLabel  * label  = dynamic_cast<QLabel*>(widget);

        if (NULL != label)
        {
            bylaw_name = label->text().toStdString();
            // ---------------------------------------
            if (nRow >= 0)
            {
                QListWidgetItem * item = ui->listWidgetVariables->item(nRow);

                if (NULL != item)
                {
                    std::string str_name = item->text().toStdString();

                    ui->lineEditVariableName->setText(QString::fromStdString(str_name));

                    std::string str_type     = Moneychanger::It()->OT().Exec().Variable_GetType    (str_template, bylaw_name, str_name);
                    std::string str_access   = Moneychanger::It()->OT().Exec().Variable_GetAccess  (str_template, bylaw_name, str_name);
                    std::string str_contents = Moneychanger::It()->OT().Exec().Variable_GetContents(str_template, bylaw_name, str_name);

                    if (0 == str_access.compare("constant"))
                        ui->comboBoxVariableAccess->setCurrentIndex(0);
                    else if (0 == str_access.compare("persistent"))
                        ui->comboBoxVariableAccess->setCurrentIndex(1);
                    else
                        ui->comboBoxVariableAccess->setCurrentIndex(2);
                    // -----------------------------------
                    if (0 == str_type.compare("string"))
                    {
                        ui->comboBoxVariableType->setCurrentIndex(0);

                        ui->plainTextEditVariableValue->setPlainText(QString::fromStdString(str_contents));
                    }
                    else if (0 == str_type.compare("integer"))
                    {
                        ui->comboBoxVariableType->setCurrentIndex(1);

                        ui->lineEditVariableValue->setText(QString::fromStdString(str_contents));
                    }
                    else
                    {
                        ui->comboBoxVariableType->setCurrentIndex(2);

                        if (0 == str_contents.compare("true"))
                            ui->comboBoxVariableValue->setCurrentIndex(0);
                        else
                            ui->comboBoxVariableValue->setCurrentIndex(1);
                    }
                    // -----------------------------------
                    ui->pushButtonDeleteVariable->setEnabled(true);
                }
            }
            else
            {
                ui->lineEditVariableName->setText(QString(""));
                ui->lineEditVariableValue->setText(QString(""));
                ui->comboBoxVariableValue->setCurrentIndex(0);
                ui->plainTextEditVariableValue->setPlainText(QString(""));

                ui->comboBoxVariableAccess->setCurrentIndex(1);
                ui->comboBoxVariableType->setCurrentIndex(0);

                ui->pushButtonDeleteVariable->setEnabled(false);
            }
        }
    }
}

void MTAgreementDetails::RefreshClauseDetails(int nRow)
{
    std::string bylaw_name;

    ui->pushButtonCancel->setEnabled(false);
    ui->pushButtonSave  ->setEnabled(false);

    if (ui->tableWidgetBylaws->currentRow() >= 0)
    {
        if (m_qstrTemplate.isEmpty())
            return;
        // ----------------------------------
        std::string str_template = m_qstrTemplate.toStdString();
        // ----------------------------------
        QWidget * widget = ui->tableWidgetBylaws->cellWidget(ui->tableWidgetBylaws->currentRow(), 0); // 0 is column for the name.
        QLabel  * label  = dynamic_cast<QLabel*>(widget);

        if (NULL != label)
        {
            bylaw_name = label->text().toStdString();
            // ---------------------------------------
            if (nRow >= 0)
            {
                QListWidgetItem * item = ui->listWidgetClauses->item(nRow);

                if (NULL != item)
                {
                    std::string str_name = item->text().toStdString();

                    ui->lineEditClauseName->setText(QString::fromStdString(str_name));

                    std::string str_script = Moneychanger::It()->OT().Exec().Clause_GetContents(str_template, bylaw_name, str_name);

                    ui->plainTextEditScript->blockSignals(true);
                    ui->plainTextEditScript->setPlainText(QString::fromStdString(str_script));
                    ui->plainTextEditScript->blockSignals(false);

                    ui->pushButtonDeleteClause->setEnabled(true);
                }
            }
            else
            {
                ui->lineEditClauseName->setText(QString(""));

                ui->plainTextEditScript->blockSignals(true);
                ui->plainTextEditScript->setPlainText(QString(""));
                ui->plainTextEditScript->blockSignals(false);

                ui->pushButtonDeleteClause->setEnabled(false);
            }
        }
    }
}

void MTAgreementDetails::RefreshHookDetails(int nRow)
{
    std::string bylaw_name;

    if (ui->tableWidgetBylaws->currentRow() >= 0)
    {
        if (m_qstrTemplate.isEmpty())
            return;
        // ----------------------------------
        std::string str_template = m_qstrTemplate.toStdString();
        // ----------------------------------
        QWidget * widget = ui->tableWidgetBylaws->cellWidget(ui->tableWidgetBylaws->currentRow(), 0); // 0 is column for the name.
        QLabel  * label  = dynamic_cast<QLabel*>(widget);

        if (NULL != label)
        {
            bylaw_name = label->text().toStdString();
            // ---------------------------------------
            if (nRow >= 0)
            {
                QListWidgetItem * item = ui->listWidgetHooks->item(nRow);

                if (NULL != item)
                {
                    std::string hook_name = item->text().toStdString();

                    QWidget * pHookWidget = createHookGroupWidget(str_template, bylaw_name, hook_name);

                    if (NULL != pHookWidget)
                    {
                        QLayout     * pLayout = ui->groupBoxHooks->layout();
                        QVBoxLayout * pVBox   = (NULL == pLayout) ? NULL : dynamic_cast<QVBoxLayout *>(pLayout);

                        if (NULL != pVBox)
                        {
                            if (m_pHookWidget)
                            {
                                pVBox->removeWidget(m_pHookWidget);

                                m_pHookWidget->setParent(NULL);
                                m_pHookWidget->disconnect();
                                m_pHookWidget->deleteLater();

                                m_pHookWidget = NULL;
                            }

                            pVBox->addWidget(pHookWidget);
                            m_pHookWidget = pHookWidget;
                        }
                    }
                }
            }
            else
            {
                QLayout     * pLayout = ui->groupBoxHooks->layout();
                QVBoxLayout * pVBox   = (NULL == pLayout) ? NULL : dynamic_cast<QVBoxLayout *>(pLayout);

                if (NULL != pVBox)
                {
                    if (m_pHookWidget)
                    {
                        pVBox->removeWidget(m_pHookWidget);

                        m_pHookWidget->setParent(NULL);
                        m_pHookWidget->disconnect();
                        m_pHookWidget->deleteLater();

                        m_pHookWidget = NULL;
                    }
                }
            }
        }
    }
    else
    {
        QLayout     * pLayout = ui->groupBoxHooks->layout();
        QVBoxLayout * pVBox   = (NULL == pLayout) ? NULL : dynamic_cast<QVBoxLayout *>(pLayout);

        if (NULL != pVBox)
        {
            if (m_pHookWidget)
            {
                pVBox->removeWidget(m_pHookWidget);

                m_pHookWidget->setParent(NULL);
                m_pHookWidget->disconnect();
                m_pHookWidget->deleteLater();

                m_pHookWidget = NULL;
            }
        }
    }
}





QWidget * MTAgreementDetails::createSingleHookWidget(std::string & bylaw_name, std::string & hook_name, std::string & clause_name)
{
    QString qstrBylawName  = QString::fromStdString(bylaw_name);
    QString qstrHookName   = QString::fromStdString(hook_name);
    QString qstrClauseName = QString::fromStdString(clause_name);

    QWidget     * pWidget    = new QWidget;
    QLineEdit   * pClause    = new QLineEdit(qstrClauseName);
    QPushButton * pBtnDelete = new QPushButton(tr("Delete"));
    // ----------------------------------------------------------
    pClause ->setReadOnly(true);
    pClause ->setStyleSheet("QLineEdit { background-color: lightgray }");

    pBtnDelete->setProperty("bylaw_name",   qstrBylawName);
    pBtnDelete->setProperty("hook_name",    qstrHookName);
    pBtnDelete->setProperty("clause_name",  qstrClauseName);
    pBtnDelete->setProperty("hookwidget",   VPtr<QWidget>::asQVariant(pWidget));
    // ----------------------------------------------------------
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(pClause);
    layout->addWidget(pBtnDelete);
    layout->setContentsMargins(1, 1, 1, 1);
    // ----------------------------------------------------------
    pWidget->setLayout(layout);

    connect(pBtnDelete, SIGNAL(clicked()), this, SLOT(on_btnHookDelete_clicked()));
    // ----------------------------------------------------------
    layout->setStretch(0,   1);
    layout->setStretch(1,   0);
    // ----------------------------------------------------------
    pClause->home(false);
    // ----------------------------------------------------------
    return pWidget;
}

QWidget * MTAgreementDetails::createHookGroupWidget(std::string & str_template, std::string & bylaw_name, std::string & hook_name)
{
    QWidget     * pBox = new QWidget;
    QVBoxLayout * vbox = new QVBoxLayout;

    vbox->setContentsMargins(1, 1, 1, 1);
    // -----------------------------------------------------------------
    int32_t nClauseCount = Moneychanger::It()->OT().Exec().Hook_GetClauseCount(str_template, bylaw_name, hook_name);

    for (int32_t ii = 0; ii < nClauseCount; ++ii)
    {
        std::string clause_name = Moneychanger::It()->OT().Exec().Hook_GetClauseAtIndex(str_template, bylaw_name, hook_name, ii);

        QWidget * pWidget = createSingleHookWidget(bylaw_name, hook_name, clause_name);

        if (NULL != pWidget)
            vbox->addWidget(pWidget);
    }
    vbox->addStretch();
    // -----------------------------------------------------------------
    pBox->setLayout(vbox);

    return pBox;
}


void MTAgreementDetails::on_btnHookDelete_clicked()
{
    if (m_qstrTemplate.isEmpty() || m_pOwner->m_qstrCurrentID.isEmpty())
        return;

    std::string str_template = m_qstrTemplate.toStdString();
    int nTemplateID = m_pOwner->m_qstrCurrentID.toInt();
    // ------------------------------------------------
    QString qstrLawyerID = m_pOwner->GetLawyerID();

    if (qstrLawyerID.isEmpty())
        return;

    std::string str_lawyer_id = qstrLawyerID.toStdString();
    // ------------------------------------------------
    QObject * pqobjSender = QObject::sender();

    if (NULL != pqobjSender)
    {
        QPushButton * pBtnDelete = dynamic_cast<QPushButton *>(pqobjSender);

        if (m_pHookWidget && (NULL != pBtnDelete))
        {
            QVariant  varBylawName   = pBtnDelete->property("bylaw_name");
            QVariant  varHookName    = pBtnDelete->property("hook_name");
            QVariant  varClauseName  = pBtnDelete->property("clause_name");
            // -------------------------------------------------------
            QString   qstrBylawName  = varBylawName .toString();
            QString   qstrHookName   = varHookName  .toString();
            QString   qstrClauseName = varClauseName.toString();
            // -------------------------------------------------------
            std::string bylaw_name   = qstrBylawName .toStdString();
            std::string hook_name    = qstrHookName  .toStdString();
            std::string clause_name  = qstrClauseName.toStdString();
            // -------------------------------------------------------
            QWidget * pWidget        = VPtr<QWidget>::asPtr(pBtnDelete->property("hookwidget"));

            if (NULL != pWidget)
            {
                std::string strTempResult = Moneychanger::It()->OT().Exec().SmartContract_RemoveHook(str_template, str_lawyer_id,
                                                                                                bylaw_name, hook_name, clause_name);

                if (!strTempResult.empty()) // Let's remove it from the GUI, too, then, and save it to the database as well.
                {
                    bool bWritten = MTContactHandler::getInstance()->SetValueByID(nTemplateID, QString::fromStdString(strTempResult),
                                                                                  QString("template_contents"), QString("smart_contract"), QString("template_id"));
                    // ------------------------------------------------
                    if (bWritten)
                    {
                        m_qstrTemplate = QString::fromStdString(strTempResult);
                        if (m_pPlainTextEdit)
                            m_pPlainTextEdit->setPlainText(m_qstrTemplate);
                    }
                    // ------------------------------------------------
                    QLayout * pLayout = m_pHookWidget->layout();

                    if (NULL != pLayout)
                    {
                        pLayout->removeWidget(pWidget);

                        pWidget->setParent(NULL);
                        pWidget->disconnect();
                        pWidget->deleteLater();

                        pWidget = NULL;
                    }
                }
            }
        }
    }
}


void MTAgreementDetails::RefreshCallbackDetails(int nRow)
{
    std::string bylaw_name;

    if (ui->tableWidgetBylaws->currentRow() >= 0)
    {
        if (m_qstrTemplate.isEmpty())
            return;
        // ----------------------------------
        std::string str_template = m_qstrTemplate.toStdString();
        // ----------------------------------
        QWidget * widget = ui->tableWidgetBylaws->cellWidget(ui->tableWidgetBylaws->currentRow(), 0); // 0 is column for the name.
        QLabel  * label  = dynamic_cast<QLabel*>(widget);

        if (NULL != label)
        {
            bylaw_name = label->text().toStdString();
            // ---------------------------------------
            if (nRow >= 0)
            {
                QListWidgetItem * item = ui->listWidgetCallbacks->item(nRow);

                if (NULL != item)
                {
                    std::string str_name = item->text().toStdString();

                    ui->lineEditCallbackName->setText(QString::fromStdString(str_name));

                    std::string str_clause = Moneychanger::It()->OT().Exec().Callback_GetClause(str_template, bylaw_name, str_name);

                    ui->lineEditCallbackClause->setText(QString::fromStdString(str_clause));

                    ui->pushButtonDeleteCallback->setEnabled(true);
                }
            }
            else
            {
                ui->lineEditCallbackName->setText(QString(""));

                ui->lineEditCallbackClause->setText(QString(""));

                ui->pushButtonDeleteCallback->setEnabled(false);
            }
        }
    }
}

void MTAgreementDetails::RefreshBylawDetails(int nRow)
{
    std::string bylaw_name;

    if (nRow >= 0)
    {
        if (m_qstrTemplate.isEmpty())
            return;
        // ----------------------------------
        std::string str_template = m_qstrTemplate.toStdString();
        // ----------------------------------
        QWidget * widget = ui->tableWidgetBylaws->cellWidget(nRow, 0); // 0 is column for the name.
        QLabel  * label  = dynamic_cast<QLabel*>(widget);

        if (NULL != label)
        {
            bylaw_name = label->text().toStdString();

            QString qstrLanguage = QString::fromStdString(Moneychanger::It()->OT().Exec().Bylaw_GetLanguage(str_template, bylaw_name));

            ui->lineEditBylawName->setText(QString::fromStdString(bylaw_name));
            ui->lineEditLanguage->setText(qstrLanguage);

            RefreshVariables(str_template, bylaw_name);
            RefreshClauses(str_template, bylaw_name);
            RefreshHooks(str_template, bylaw_name);
            RefreshCallbacks(str_template, bylaw_name);

            ui->pushButtonDeleteBylaw->setEnabled(true);
        }
    }
    else
    {
        ui->pushButtonDeleteBylaw->setEnabled(false);

        ui->lineEditBylawName->setText(QString(""));
        ui->lineEditLanguage->setText(QString(""));

        ClearVariables();
        ClearClauses();
        ClearHooks();
        ClearCallbacks();
    }
}

void MTAgreementDetails::RefreshPartyDetails(int nRow)
{
    std::string party_name;

    if (nRow >= 0)
    {
        if (m_qstrTemplate.isEmpty())
            return;
        // ----------------------------------
        std::string str_template = m_qstrTemplate.toStdString();
        // ----------------------------------
        QWidget * widget = ui->tableWidgetParties->cellWidget(nRow, 0); // 0 is column for the name.
        QLabel  * label  = dynamic_cast<QLabel*>(widget);

        if (NULL != label)
        {
            party_name = label->text().toStdString();

            ui->lineEditPartyName->setText(QString::fromStdString(party_name));

            ui->labelPartyNymID->setVisible(ui->checkBoxSpecifyNym->isChecked());
            ui->lineEditPartyNymID->setVisible(ui->checkBoxSpecifyNym->isChecked());

            if (ui->checkBoxSpecifyNym->isChecked())
            {
                QString qstrPartyNymID = QString::fromStdString(Moneychanger::It()->OT().Exec().Party_GetID(str_template, party_name));
                ui->lineEditPartyNymID->setText(qstrPartyNymID);
            }
            else
                ui->lineEditPartyNymID->setText(QString(""));

            RefreshAgents(str_template, party_name);
            RefreshAccounts(str_template, party_name);

            ui->pushButtonDeleteParty->setEnabled(true);
        }
    }
    else
    {
        ui->pushButtonDeleteParty->setEnabled(false);

        ui->lineEditPartyName->setText(QString(""));
        ui->labelPartyNymID->setText(QString(""));

        ui->labelPartyNymID->setVisible(false);
        ui->lineEditPartyNymID->setVisible(false);

        ClearAgents();
        ClearAccounts();
    }
}

void MTAgreementDetails::RefreshAgents(std::string str_template, std::string str_party)
{
    ui->listWidgetAgents->blockSignals(true);
    // -----------------------------------
    ui->listWidgetAgents->clear();
    // -----------------------------------
    const int32_t nCount = Moneychanger::It()->OT().Exec().Party_GetAgentCount(str_template, str_party);

    for (int32_t ii = 0; ii < nCount; ++ii)
    {
        std::string str_agent = Moneychanger::It()->OT().Exec().Party_GetAgentNameByIndex(str_template, str_party, ii);

        if (!str_agent.empty())
        {
            QString qstrName = QString::fromStdString(str_agent);
            ui->listWidgetAgents->addItem(qstrName);
        }
    }
    // -----------------------------------
    int currentRow = 0;

    if (ui->listWidgetAgents->count() > 0)
        currentRow = 0;
    else
        currentRow = -1;

    ui->listWidgetAgents->setCurrentRow(currentRow, 0);
    ui->listWidgetAgents->blockSignals(false);

    on_listWidgetAgents_currentRowChanged(currentRow);
}

void MTAgreementDetails::RefreshAccounts(std::string str_template, std::string str_party)
{
    ui->listWidgetAccounts->blockSignals(true);
    // -----------------------------------
    ui->listWidgetAccounts->clear();
    // -----------------------------------
    const int32_t nCount = Moneychanger::It()->OT().Exec().Party_GetAcctCount(str_template, str_party);

    for (int32_t ii = 0; ii < nCount; ++ii)
    {
        std::string str_acct = Moneychanger::It()->OT().Exec().Party_GetAcctNameByIndex(str_template, str_party, ii);

        if (!str_acct.empty())
        {
            QString qstrName = QString::fromStdString(str_acct);
            ui->listWidgetAccounts->addItem(qstrName);
        }
    }
    // -----------------------------------
    int currentRow = 0;

    if (ui->listWidgetAccounts->count() > 0)
        currentRow = 0;
    else
        currentRow = -1;

    ui->listWidgetAccounts->setCurrentRow(currentRow, 0);
    ui->listWidgetAccounts->blockSignals(false);

    on_listWidgetAccounts_currentRowChanged(currentRow);
}

void MTAgreementDetails::RefreshVariables(std::string str_template, std::string str_bylaw)
{
    ui->listWidgetVariables->blockSignals(true);
    // -----------------------------------
    ui->listWidgetVariables->clear();
    // -----------------------------------
    const int32_t nCount = Moneychanger::It()->OT().Exec().Bylaw_GetVariableCount(str_template, str_bylaw);

    for (int32_t ii = 0; ii < nCount; ++ii)
    {
        std::string str_name = Moneychanger::It()->OT().Exec().Variable_GetNameByIndex(str_template, str_bylaw, ii);

        if (!str_name.empty())
        {
            QString qstrName = QString::fromStdString(str_name);
            ui->listWidgetVariables->addItem(qstrName);
        }
    }
    // -----------------------------------
    int currentRow = 0;

    if (ui->listWidgetVariables->count() > 0)
        currentRow = 0;
    else
        currentRow = -1;

    ui->listWidgetVariables->setCurrentRow(currentRow, 0);
    ui->listWidgetVariables->blockSignals(false);

    on_listWidgetVariables_currentRowChanged(currentRow);
}

void MTAgreementDetails::RefreshClauses(std::string str_template, std::string str_bylaw)
{
    ui->listWidgetClauses->blockSignals(true);
    // -----------------------------------
    ui->listWidgetClauses->clear();
    // -----------------------------------
    const int32_t nCount = Moneychanger::It()->OT().Exec().Bylaw_GetClauseCount(str_template, str_bylaw);

    for (int32_t ii = 0; ii < nCount; ++ii)
    {
        std::string str_name = Moneychanger::It()->OT().Exec().Clause_GetNameByIndex(str_template, str_bylaw, ii);

        if (!str_name.empty())
        {
            QString qstrName = QString::fromStdString(str_name);
            ui->listWidgetClauses->addItem(qstrName);
        }
    }
    // -----------------------------------
    int currentRow = 0;

    if (ui->listWidgetClauses->count() > 0)
        currentRow = 0;
    else
        currentRow = -1;

    ui->listWidgetClauses->setCurrentRow(currentRow, 0);
    ui->listWidgetClauses->blockSignals(false);

    on_listWidgetClauses_currentRowChanged(currentRow);
}

void MTAgreementDetails::RefreshHooks(std::string str_template, std::string str_bylaw)
{
    ui->listWidgetHooks->blockSignals(true);
    // -----------------------------------
    ui->listWidgetHooks->clear();
    // -----------------------------------
    std::map<std::string, std::string> string_map;

    const int32_t nCount = Moneychanger::It()->OT().Exec().Bylaw_GetHookCount(str_template, str_bylaw);

    for (int32_t ii = 0; ii < nCount; ++ii)
    {
        std::string str_name = Moneychanger::It()->OT().Exec().Hook_GetNameByIndex(str_template, str_bylaw, ii);

        if (!str_name.empty())
        {
            std::map<std::string, std::string>::iterator it = string_map.find(str_name);

            // We do this because the same hook appears multiple times (once for each clause)
            // but we only want it to appear once in this list. (The multiple clauses will be
            // shown in the details page on the right.)
            //
            if (it == string_map.end()) // string wasn't already there.
            {
                QString qstrName = QString::fromStdString(str_name);
                ui->listWidgetHooks->addItem(qstrName);

                string_map.insert(std::pair<std::string, std::string>(str_name, str_name));
            }
        }
    }
    // -----------------------------------
    int currentRow = 0;

    if (ui->listWidgetHooks->count() > 0)
        currentRow = 0;
    else
        currentRow = -1;

    ui->listWidgetHooks->setCurrentRow(currentRow, 0);
    ui->listWidgetHooks->blockSignals(false);

    on_listWidgetHooks_currentRowChanged(currentRow);
}

void MTAgreementDetails::RefreshCallbacks(std::string str_template, std::string str_bylaw)
{
    ui->listWidgetCallbacks->blockSignals(true);
    // -----------------------------------
    ui->listWidgetCallbacks->clear();
    // -----------------------------------
    const int32_t nCount = Moneychanger::It()->OT().Exec().Bylaw_GetCallbackCount(str_template, str_bylaw);

    for (int32_t ii = 0; ii < nCount; ++ii)
    {
        std::string str_name = Moneychanger::It()->OT().Exec().Callback_GetNameByIndex(str_template, str_bylaw, ii);

        if (!str_name.empty())
        {
            QString qstrName = QString::fromStdString(str_name);
            ui->listWidgetCallbacks->addItem(qstrName);
        }
    }
    // -----------------------------------
    int currentRow = 0;

    if (ui->listWidgetCallbacks->count() > 0)
        currentRow = 0;
    else
        currentRow = -1;

    ui->listWidgetCallbacks->setCurrentRow(currentRow, 0);
    ui->listWidgetCallbacks->blockSignals(false);

    on_listWidgetCallbacks_currentRowChanged(currentRow);
}

void MTAgreementDetails::ClearVariables()
{
    ui->listWidgetVariables->blockSignals(true);
    // -----------------------------------
    ui->listWidgetVariables->clear();
    // -----------------------------------
    ui->listWidgetVariables->setCurrentRow(-1, 0);
    ui->listWidgetVariables->blockSignals(false);

    on_listWidgetVariables_currentRowChanged(-1);
}

void MTAgreementDetails::ClearClauses()
{
    ui->listWidgetClauses->blockSignals(true);
    // -----------------------------------
    ui->listWidgetClauses->clear();
    // -----------------------------------
    ui->listWidgetClauses->setCurrentRow(-1, 0);
    ui->listWidgetClauses->blockSignals(false);

    on_listWidgetClauses_currentRowChanged(-1);
}

void MTAgreementDetails::ClearHooks()
{
    ui->listWidgetHooks->blockSignals(true);
    // -----------------------------------
    ui->listWidgetHooks->clear();
    // -----------------------------------
    ui->listWidgetHooks->setCurrentRow(-1, 0);
    ui->listWidgetHooks->blockSignals(false);

    on_listWidgetHooks_currentRowChanged(-1);
}

void MTAgreementDetails::ClearCallbacks()
{
    ui->listWidgetCallbacks->blockSignals(true);
    // -----------------------------------
    ui->listWidgetCallbacks->clear();
    // -----------------------------------
    ui->listWidgetCallbacks->setCurrentRow(-1, 0);
    ui->listWidgetCallbacks->blockSignals(false);

    on_listWidgetCallbacks_currentRowChanged(-1);
}

void MTAgreementDetails::ClearAgents()
{
    ui->listWidgetAgents->blockSignals(true);
    // -----------------------------------
    ui->listWidgetAgents->clear();
    // -----------------------------------
    ui->listWidgetAgents->setCurrentRow(-1, 0);
    ui->listWidgetAgents->blockSignals(false);

    on_listWidgetAgents_currentRowChanged(-1);
}

void MTAgreementDetails::ClearAccounts()
{
    ui->listWidgetAccounts->blockSignals(true);
    // -----------------------------------
    ui->listWidgetAccounts->clear();
    // -----------------------------------
    ui->listWidgetAccounts->setCurrentRow(-1, 0);
    ui->listWidgetAccounts->blockSignals(false);

    on_listWidgetAccounts_currentRowChanged(-1);
}

void MTAgreementDetails::PopulateParties()
{
    ClearPartyMap();

    if (m_qstrTemplate.isEmpty())
        return;
    // ----------------------------------
    std::string str_template = m_qstrTemplate.toStdString();
    // ----------------------------------
    const int32_t nCount = Moneychanger::It()->OT().Exec().Smart_GetPartyCount(str_template);

    for (int32_t nIndex = 0; nIndex < nCount; ++nIndex)
    {
        std::string str_party = Moneychanger::It()->OT().Exec().Smart_GetPartyByIndex(str_template, nIndex);

        m_mapParties.insert(QString::fromStdString(str_party), QString::fromStdString(str_party));
    }
    // ----------------------------------
    PopulatePartyGrid();
}

void MTAgreementDetails::PopulatePartyGrid()
{
    if (m_qstrTemplate.isEmpty())
        return;
    // ----------------------------------
    std::string str_template = m_qstrTemplate.toStdString();
    // ----------------------------------
//  this->blockSignals(true);
    // -----------------------------------
    ui->tableWidgetParties->blockSignals(true);
    // -----------------------------------
    int nGridIndex = 0;
    int nRowCount  = m_mapParties.size();
    // -------------------------------------
    ui->tableWidgetParties->setRowCount(nRowCount);
    // -----------------------------------
    for (mapIDName::iterator it_top = m_mapParties.begin(); it_top != m_mapParties.end(); ++it_top)
    {
        QString     qstrName      = it_top.key();
        std::string str_name      = qstrName.toStdString();
        int32_t     nAgentCount   = Moneychanger::It()->OT().Exec().Party_GetAgentCount(str_template, str_name);
        int32_t     nAccountCount = Moneychanger::It()->OT().Exec().Party_GetAcctCount(str_template, str_name);
        // -----------------------------------------------------------------------
        QLabel * pLabelName         = new QLabel(QString::fromStdString(str_name));
        QLabel * pLabelAgentCount   = new QLabel(QString("%1").arg(nAgentCount));
        QLabel * pLabelAccountCount = new QLabel(QString("%1").arg(nAccountCount));
        // -----------------------------------------------------------------------
        pLabelName        ->setAlignment(Qt::AlignCenter);
        pLabelAgentCount  ->setAlignment(Qt::AlignCenter);
        pLabelAccountCount->setAlignment(Qt::AlignCenter);
        // -----------------------------------------------------------------------
        ui->tableWidgetParties->setCellWidget ( nGridIndex, 0, pLabelName         );
        ui->tableWidgetParties->setCellWidget ( nGridIndex, 1, pLabelAgentCount   );
        ui->tableWidgetParties->setCellWidget ( nGridIndex, 2, pLabelAccountCount );
        // -----------------------------------------------------------------------
        ++nGridIndex;
    }
    // -----------------------------------------------------
    if (ui->tableWidgetParties->rowCount() > 0)
    {
        ui->tableWidgetParties->setCurrentCell(0, 0);

        ui->tableWidgetParties->blockSignals(false);

        on_tableWidgetParties_currentCellChanged(0, 0, 0, 0); // Note: see comment in following block.

        // Sometimes setCurrentCell WOULDN'T trigger currentCellChanged.
        // Like in the case where we're refreshing the smart contract details as we select a different
        // contract. In that case, both smart contracts had 1 party, at index 0, and thus the index for
        // parties didn't actually change. It went from row 0 to row 0. Therefore, when I set the cell
        // here myself by hand, I block the signals, set the cell, and then call currentCellChanged by
        // HAND as well. I made this same fix in various places.
    }
    else
    {
        ui->tableWidgetParties->setCurrentCell(-1, 0);

        ui->tableWidgetParties->blockSignals(false);

        on_tableWidgetParties_currentCellChanged(-1, 0, 0, 0);

        // NOTE: Why do I call currentCellChanged by hand here?
        // Because in the case where the tableWidget is empty,
        // setCurrentCell somehow DOESN'T automatically trigger
        // currentCellChanged, even though it DOES work when the
        // tableWidget has items in it.
        //
        // UPDATE: Ahh I see why: since the grid was already empty,
        // the current row was -1, so setting it to -1 again doesn't
        // CHANGE it, so therefore, no need for "currentCellChanged" event.
        // This can even happen in the case where it's populated (like if
        // it goes from 0 to 0.)
    }
    // -----------------------------------------------------
}

void MTAgreementDetails::AddButtonClicked()
{
    QString qstrLawyerID = m_pOwner->GetLawyerID();

    if (qstrLawyerID.isEmpty())
        return;
    // ------------------------------
    MTWizardAddContract theWizard(this);

    theWizard.setWindowTitle(tr("Add Smart Contract"));

    QString qstrDefaultValue("https://raw.githubusercontent.com/Open-Transactions/opentxs/develop/scripts/smartcontracts/escrow/escrow.otc");
    QVariant varDefault(qstrDefaultValue);

    theWizard.setField(QString("NymID"), qstrLawyerID);
    theWizard.setField(QString("URL"), varDefault);
    theWizard.setField(QString("contractType"), QString("smart")); // So the wizard knows it's creating a smart contract.

    theWizard.setField(QString("contractXML"), QString("unused for smart contracts"));

    if (QDialog::Accepted == theWizard.exec())
    {
        bool bIsImporting = theWizard.field("isImporting").toBool();
        bool bIsCreating  = theWizard.field("isCreating").toBool();

        if (bIsImporting)
        {
            bool bIsURL      = theWizard.field("isURL").toBool();
            bool bIsFilename = theWizard.field("isFilename").toBool();
            bool bIsContents = theWizard.field("isContents").toBool();

            if (bIsURL)
            {
                QString qstrURL = theWizard.field("URL").toString();
                // --------------------------------
                if (qstrURL.isEmpty())
                {
                    QMessageBox::warning(this, tr("URL is Empty"),
                        tr("No URL was provided."));

                    return;
                }

                QUrl theURL(qstrURL);
                // --------------------------------
                if (m_pDownloader)
                {
                    m_pDownloader->deleteLater();
                    m_pDownloader = NULL;
                }
                // --------------------------------
                m_pDownloader = new FileDownloader(theURL, this);

                connect(m_pDownloader, SIGNAL(downloaded()), SLOT(DownloadedURL()));
            }
            // --------------------------------
            else if (bIsFilename)
            {
                QString fileName = theWizard.field("Filename").toString();

                if (fileName.isEmpty())
                {
                    QMessageBox::warning(this, tr("Filename is Empty"),
                        tr("No filename was provided."));

                    return;
                }
                // -----------------------------------------------
                QString qstrContents;
                QFile plainFile(fileName);

                if (plainFile.open(QIODevice::ReadOnly))//| QIODevice::Text)) // Text flag translates /n/r to /n
                {
                    QTextStream in(&plainFile); // Todo security: check filesize here and place a maximum size.
                    qstrContents = in.readAll();

                    plainFile.close();
                    // ----------------------------
                    if (qstrContents.isEmpty())
                    {
                        QMessageBox::warning(this, tr("File Was Empty"),
                                             QString("%1: %2").arg(tr("File was apparently empty")).arg(fileName));
                        return;
                    }
                    // ----------------------------
                }
                else
                {
                    QMessageBox::warning(this, tr("Failed Reading File"),
                                         QString("%1: %2").arg(tr("Failed trying to read file")).arg(fileName));
                    return;
                }
                // -----------------------------------------------
                ImportContract(qstrContents);
            }
            // --------------------------------
            else if (bIsContents)
            {
                QString qstrContents = theWizard.getContents();

                if (qstrContents.isEmpty())
                {
                    QMessageBox::warning(this, tr("Empty Contract"),
                        tr("Failure Importing: Contract is Empty."));
                    return;
                }
                // -------------------------
                ImportContract(qstrContents);
            }
        }
        // --------------------------------
        else if (bIsCreating)
        {
            QString qstrNymID = theWizard.field("NymID").toString();
            // --------------------------------------------
            QMessageBox::StandardButton reply;

            reply = QMessageBox::question(this, "", QString("%1").arg(tr("Do you want Nym IDs to be explicitly specified on this template? (Default yes.)")),
                                          QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
            const bool bSpecifyParties = (reply == QMessageBox::Yes);

            reply = QMessageBox::question(this, "", QString("%1").arg(tr("Do you want asset type IDs to be explicitly specified on this template? (Default yes.)")),
                                          QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
            const bool bSpecifyAssets = (reply == QMessageBox::Yes);
            // --------------------------------------------
            time64_t tDate1   = 0;  // 0 means "replace the 0 with the current time."
            time64_t tDate2   = 0;  // 0 means "never expires."

            std::string str_template = Moneychanger::It()->OT().Exec().Create_SmartContract(qstrNymID.toStdString(), tDate1, tDate2, bSpecifyAssets, bSpecifyParties);

            if (str_template.empty())
            {
                QMessageBox::warning(this, tr("Failed creating smart contract"),
                                     tr("Failed trying to create smart contract."));
                return;
            }
            else
            {
                int nTemplateID = MTContactHandler::getInstance()->CreateSmartContractTemplate(QString::fromStdString(str_template));

                if (nTemplateID <= 0)
                {
                    QMessageBox::warning(this, tr("Failed creating smart contract"),
                                         tr("Failed trying to create smart contract."));
                    return;
                }
                // -------------------------------------------------------
                // else (Successfully created the new Template...)
                // Now let's add this template to the Map, and refresh the dialog,
                // and then set the new template as the current one.
                //
                QString qstrTemplateID = QString("%1").arg(nTemplateID);

                m_pOwner->m_map.insert(qstrTemplateID, QString(tr("New Smart Contract")));
                m_pOwner->SetPreSelected(qstrTemplateID);
                // ------------------------------------------------
                emit RefreshRecordsAndUpdateMenu();
                // ------------------------------------------------
            }
        } // bCreating.
    }
}


void MTAgreementDetails::ImportContract(QString qstrContents)
{
    if (qstrContents.isEmpty())
    {
        QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
            tr("Failed importing: contract is empty."));
        return;
    }
    // ------------------------------------------------------
    else
    {
        opentxs::String strContract(qstrContents.toStdString());

        auto pScriptable{Moneychanger::It()->OT().Factory().Scriptable(Moneychanger::It()->OT(), strContract)};
        opentxs::OTSmartContract * pSmartContract = (false == bool(pScriptable)) ? nullptr : dynamic_cast<opentxs::OTSmartContract*>(pScriptable.get());

        if (nullptr == pSmartContract)
        {
            QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                tr("Failed importing: Unable to load the contract up as a smart contract."));
            return;
        }
        // --------------------------------------------
        int nTemplateID = MTContactHandler::getInstance()->CreateSmartContractTemplate(qstrContents);

        if (nTemplateID <= 0)
        {
            QMessageBox::warning(this, tr("Failed creating smart contract"),
                                 tr("Failed trying to create smart contract."));
            return;
        }
        // -------------------------------------------------------
        // else (Successfully created the new Template...)
        // Now let's add this template to the Map, and refresh the dialog,
        // and then set the new template as the current one.
        //
        QString qstrTemplateID = QString("%1").arg(nTemplateID);

        m_pOwner->m_map.insert(qstrTemplateID, QString(tr("New Smart Contract")));
        m_pOwner->SetPreSelected(qstrTemplateID);
        // ------------------------------------------------
        emit RefreshRecordsAndUpdateMenu();
        // ------------------------------------------------
    } // if (!qstrContractID.isEmpty())
}


void MTAgreementDetails::DownloadedURL()
{
    QString qstrContents;

    if (m_pDownloader)
        qstrContents = m_pDownloader->downloadedData();
    // ----------------------------
    if (qstrContents.isEmpty())
    {
        QMessageBox::warning(this, tr("File at URL Was Empty"),
                             tr("File at specified URL was apparently empty"));
        return;
    }
    // ----------------------------
    ImportContract(qstrContents);
}

// ------------------------------------------------------


void MTAgreementDetails::DeleteButtonClicked()
{
    if (!m_pOwner->m_qstrCurrentID.isEmpty())
    {
        QMessageBox::StandardButton reply;

        reply = QMessageBox::question(this, tr(MONEYCHANGER_APP_NAME), tr("Are you sure you want to delete this Smart Contract?"),
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes)
        {
            bool bSuccess = MTContactHandler::getInstance()->DeleteSmartContract(m_pOwner->m_qstrCurrentID.toInt());

            if (bSuccess)
            {
                m_pOwner->m_map.remove(m_pOwner->m_qstrCurrentID);
                // ------------------------------------------------
                emit RefreshRecordsAndUpdateMenu();
                // ------------------------------------------------
            }
            else
                QMessageBox::warning(this, tr(MONEYCHANGER_APP_NAME),
                                     tr("Failed trying to delete this Smart Contract."));
        }
    }
}

MTAgreementDetails::~MTAgreementDetails()
{
    delete ui;
}

bool MTAgreementDetails::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Resize)
    {
        // This insures that the left-most part of the IDs and Names
        // remains visible during all resize events.
        //
        FavorLeftSideForIDs();
    }
//    else
//    {
        // standard event processing
//        return QWidget::eventFilter(obj, event);
        return MTEditDetails::eventFilter(obj, event);

        // NOTE: Since the base class has definitely already installed this
        // function as the event filter, I must assume that this version
        // is overriding the version in the base class.
        //
        // Therefore I call the base class version here, since as it's overridden,
        // I don't expect it will otherwise ever get called.
//    }
}

// ----------------------------------
//virtual
int MTAgreementDetails::GetCustomTabCount()
{
    return 1;
}
// ----------------------------------
//virtual
QWidget * MTAgreementDetails::CreateCustomTab(int nTab)
{
    const int nCustomTabCount = this->GetCustomTabCount();
    // -----------------------------
    if ((nTab < 0) || (nTab >= nCustomTabCount))
        return NULL; // out of bounds.
    // -----------------------------
    QWidget * pReturnValue = NULL;
    // -----------------------------
    switch (nTab)
    {
    case 0:
    {
        if (m_pPlainTextEdit)
        {
            m_pPlainTextEdit->setParent(NULL);
            m_pPlainTextEdit->disconnect();
            m_pPlainTextEdit->deleteLater();

            m_pPlainTextEdit = NULL;
        }

        m_pPlainTextEdit = new QPlainTextEdit;

        m_pPlainTextEdit->setReadOnly(true);
        m_pPlainTextEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        // -------------------------------
        QVBoxLayout * pvBox = new QVBoxLayout;

        QLabel * pLabelContents = new QLabel(tr("Smart Contract:"));

        pvBox->setAlignment(Qt::AlignTop);
        pvBox->addWidget   (pLabelContents);
        pvBox->addWidget   (m_pPlainTextEdit);
        // -------------------------------
        pReturnValue = new QWidget;
        pReturnValue->setContentsMargins(0, 0, 0, 0);
        pReturnValue->setLayout(pvBox);
    }
        break;

    default:
        qDebug() << QString("Unexpected: MTAgreementDetails::CreateCustomTab was called with bad index: %1").arg(nTab);
        return NULL;
    }
    // -----------------------------
    return pReturnValue;
}
// ---------------------------------
//virtual
QString  MTAgreementDetails::GetCustomTabName(int nTab)
{
    const int nCustomTabCount = this->GetCustomTabCount();
    // -----------------------------
    if ((nTab < 0) || (nTab >= nCustomTabCount))
        return QString(""); // out of bounds.
    // -----------------------------
    QString qstrReturnValue("");
    // -----------------------------
    switch (nTab)
    {
    case 0:  qstrReturnValue = "Smart Contract";  break;

    default:
        qDebug() << QString("Unexpected: MTAgreementDetails::GetCustomTabName was called with bad index: %1").arg(nTab);
        return QString("");
    }
    // -----------------------------
    return qstrReturnValue;
}
// ------------------------------------------------------

void MTAgreementDetails::on_pushButtonNewBylaw_clicked()
{
    if (m_qstrTemplate.isEmpty() || m_pOwner->m_qstrCurrentID.isEmpty())
        return;

    std::string str_template = m_qstrTemplate.toStdString();
    int nTemplateID = m_pOwner->m_qstrCurrentID.toInt();
    // ------------------------------------------------
    QString qstrLawyerID = m_pOwner->GetLawyerID();

    if (qstrLawyerID.isEmpty())
        return;

    std::string str_lawyer_id = qstrLawyerID.toStdString();
    // ------------------------------------------------
    QString qstrNewName("");

    int nCurrentComboIndex = ui->comboBoxBylaw->currentIndex();
    int nBylawCount        = Moneychanger::It()->OT().Exec().Smart_GetBylawCount(str_template);
    // ------------------------------------------------
    if ((nCurrentComboIndex >= 0) && (nCurrentComboIndex <= 4))
    {
        MTGetStringDialog nameDlg(this, tr("Enter a Name:"));

        switch (nCurrentComboIndex)
        {
        case 0:
            nameDlg.setWindowTitle(tr("New Bylaw"));
            break;
        case 1:
            nameDlg.setWindowTitle(tr("New Variable"));
            break;
        case 2:
            nameDlg.setWindowTitle(tr("New Clause"));
            break;
        case 3:
            nameDlg.setWindowTitle(tr("New Hook"));
            break;
        case 4:
            nameDlg.setWindowTitle(tr("New Callback"));
            break;
        }
        // --------------
        // If it's the first Bylaw, then we already know the name to use: "main_bylaws"
        //
        if ((0 == nCurrentComboIndex) &&
            (0 == nBylawCount))
        {
            qstrNewName = QString("main_bylaws");
        }
        // --------------
        else if (3 == nCurrentComboIndex) // This is for hooks. User must choose from a pre-defined list.
        {
            // cron_process
            // cron_activate
            // -------------------------------------------
            // Select from the hooks.
            //
            DlgChooser theChooser(this);
            // -----------------------------------------------
            mapIDName & the_map = theChooser.m_map;
            // -----------------------------------------------
            QString OT_id;
            QString OT_name;
            // -----------------------------------------------
            OT_id   = QString("cron_process");
            OT_name = QString("OnProcess");

            the_map.insert(OT_id, OT_name);
            // -----------------------------------------------
            OT_id   = QString("cron_activate");
            OT_name = QString("OnActivate");

            the_map.insert(OT_id, OT_name);
            // -----------------------------------------------
            theChooser.setWindowTitle(tr("Select the Hook"));
            // -----------------------------------------------
            if (theChooser.exec() != QDialog::Accepted)
                return;
            if (theChooser.m_qstrCurrentID.isEmpty())
                return;
            qstrNewName = theChooser.m_qstrCurrentID;
        }
        // --------------
        else if (4 == nCurrentComboIndex) // This is for callbacks. User must choose from a pre-defined list.
        {
            // callback_party_may_execute_clause
            // callback_party_may_cancel_contract
            // -------------------------------------------
            // Select from the callbacks.
            //
            DlgChooser theChooser(this);
            // -----------------------------------------------
            mapIDName & the_map = theChooser.m_map;
            // -----------------------------------------------
            QString OT_id;
            QString OT_name;
            // -----------------------------------------------
            OT_id   = QString("callback_party_may_execute_clause");
            OT_name = QString("Party may execute clause?");

            the_map.insert(OT_id, OT_name);
            // -----------------------------------------------
            OT_id   = QString("callback_party_may_cancel_contract");
            OT_name = QString("Party may cancel contract?");

            the_map.insert(OT_id, OT_name);
            // -----------------------------------------------
            theChooser.setWindowTitle(tr("Select the Callback"));
            // -----------------------------------------------
            if (theChooser.exec() != QDialog::Accepted)
                return;
            if (theChooser.m_qstrCurrentID.isEmpty())
                return;
            qstrNewName = theChooser.m_qstrCurrentID;
        }
        else if (QDialog::Accepted == nameDlg.exec())
        {
            qstrNewName = nameDlg.GetOutputString();
        }
        else
            return;
    }
    else
        return;
    // ---------------------------------------------
    if (!qstrNewName.isEmpty())
    {
        // For adding variables, clauses, hooks, and callbacks, we need to know
        // which bylaw we are adding to. (Of course we don't need to know that if
        // we're adding a bylaw, but we do for adding everything else.) So just in
        // case we're going to need to know which bylaw is currently selected,
        // I look up that info here first.
        //
        std::string bylaw_name;

        int nCurrentIndex = ui->tableWidgetBylaws->currentRow();

        if (nCurrentIndex >= 0)
        {
            QWidget * widget = ui->tableWidgetBylaws->cellWidget(nCurrentIndex, 0); // 0 is column for the name.
            QLabel  * label  = dynamic_cast<QLabel*>(widget);

            if (NULL != label)
                bylaw_name = label->text().toStdString();
        }
        // ------------------------------
        std::string strSmartResult;

        switch (nCurrentComboIndex)
        {
        case 0: // Bylaw
        {
            strSmartResult = Moneychanger::It()->OT().Exec().SmartContract_AddBylaw(str_template, str_lawyer_id, qstrNewName.toStdString());
            break;
        }
        case 1: // Variable
        {
            if (bylaw_name.empty())
            {
                QString qstrErrMsg(tr("Please select a bylaw first, to add the variable to."));

                if (0 == nBylawCount)
                    qstrErrMsg = QString(tr("You must create a bylaw first, before creating any variables."));

                QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), qstrErrMsg);
                return;
            }

            DlgCreateVariable theDlg(this);

            theDlg.m_qstrName = qstrNewName;

            if (theDlg.exec() != QDialog::Accepted)
                return;

            QString qstrAccess = theDlg.m_qstrAccess;
            QString qstrType   = theDlg.m_qstrType;
            QString qstrValue;

            if (qstrAccess.isEmpty() || qstrType.isEmpty())
            {
                QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME),
                                         tr("Failed due to missing settings. Please make sure the variable 'Access' and 'Type' are selected, and then try again."));
                return;
            }
            // ----------------------------------------------
            qstrValue = theDlg.m_qstrValue;
            // ----------------------------------------------
            if (qstrValue.isEmpty())
            {
                QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME),
                                         tr("Failed due to empty value. Please make sure the variable's initial value is set."));
                return;
            }
            // ----------------------------------------------
            strSmartResult = Moneychanger::It()->OT().Exec().SmartContract_AddVariable(str_template, str_lawyer_id,
                                                                                  bylaw_name, qstrNewName.toStdString(),
                                                                                  qstrAccess.toStdString(), qstrType.toStdString(),
                                                                                  qstrValue.toStdString());
            break;
        }
        case 2: // Clause
        {
            if (bylaw_name.empty())
            {
                QString qstrErrMsg(tr("Please select a bylaw first, to add the clause to."));

                if (0 == nBylawCount)
                    qstrErrMsg = QString(tr("You must create a bylaw first, before creating any clauses."));

                QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), qstrErrMsg);
                return;
            }
            // --------------------------
            QString qstrScript("// script code goes here");

            strSmartResult = Moneychanger::It()->OT().Exec().SmartContract_AddClause(str_template, str_lawyer_id,
                                                                                bylaw_name, qstrNewName.toStdString(),
                                                                                qstrScript.toStdString());
            break;
        }
        case 3: // Hook
        {
            if (bylaw_name.empty())
            {
                QString qstrErrMsg(tr("Please select a bylaw first, to add the hook to."));

                if (0 == nBylawCount)
                    qstrErrMsg = QString(tr("You must create a bylaw first, before creating any hooks."));

                QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), qstrErrMsg);
                return;
            }
            // -------------------------------------
            // We must ask the user to choose the clause that will be triggered
            // by the hook.
            // And if there are no clauses, we will create one.
            //
            QString qstrClauseName, qstrScript("// Script code goes here.");

            QMessageBox msgBox(
                        QMessageBox::Question,
                        tr(MONEYCHANGER_APP_NAME),
                        tr("Create a new clause to be triggered by the hook, or select an existing clause?"),
                        QMessageBox::Cancel | QMessageBox::Yes | QMessageBox::No, this);

            msgBox.setButtonText(QMessageBox::Yes, tr("Create New Clause"));
            msgBox.setButtonText(QMessageBox::No, tr("Select Existing Clause"));

            int boxResult = QMessageBox::Cancel;

            if ( (0 == Moneychanger::It()->OT().Exec().Bylaw_GetClauseCount(str_template, bylaw_name)) ||
                 (QMessageBox::Yes == (boxResult = msgBox.exec())) )
            {
                qstrClauseName = qstrNewName.right(qstrNewName.length() - 4); // Remove the "hook" or "cron" prefix and add "on" to derive the clause name.
                qstrClauseName = QString("on%1").arg(qstrClauseName);

                std::string strTempResult = Moneychanger::It()->OT().Exec().SmartContract_AddClause(str_template, str_lawyer_id,
                                                                                               bylaw_name, qstrClauseName.toStdString(),
                                                                                               qstrScript.toStdString());
                if (!strTempResult.empty())
                    str_template = strTempResult;
                else
                {
                    QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME),
                                             tr("Failed creating hook: couldn't create a clause to attach it to."));
                    return;
                }
            }
            else if (QMessageBox::No == boxResult) // Here we pop up the DlgChooser for selected a clause.
            {
                // -----------------------------------------------
                // Select from existing clauses.
                //
                DlgChooser theChooser(this);
                // -----------------------------------------------
                mapIDName & the_map = theChooser.m_map;
                // -----------------------------------------------
                const int32_t the_count = Moneychanger::It()->OT().Exec().Bylaw_GetClauseCount(str_template, bylaw_name);
                // -----------------------------------------------
                for (int32_t ii = 0; ii < the_count; ++ii)
                {

                    QString OT_id = QString::fromStdString(Moneychanger::It()->OT().Exec().Clause_GetNameByIndex(str_template, bylaw_name, ii));
                    QString OT_name = OT_id;
                    // -----------------------------------------------
                    if (!OT_id.isEmpty())
                    {
                        the_map.insert(OT_id, OT_name);
                    }
                 }
                // -----------------------------------------------
                theChooser.setWindowTitle(tr("Select the Clause to be Triggered"));
                // -----------------------------------------------
                if (theChooser.exec() != QDialog::Accepted)
                    return;
                // -----------------------------------------------
                qstrClauseName = theChooser.m_qstrCurrentID;
            }
            else        // This means the user chose "cancel" when he was asked whether
                return; // to create a new clause or select an existing one.
            // --------------------------------------------------------
            // If the hook/clause association already exists, we'll just remove it first.
            //
            std::string strTempResult = Moneychanger::It()->OT().Exec().SmartContract_RemoveHook(str_template, str_lawyer_id,
                                                                                            bylaw_name, qstrNewName.toStdString(),
                                                                                            qstrClauseName.toStdString());
            if (!strTempResult.empty())
                str_template = strTempResult;
            // ---------------------------------------------------------
            strSmartResult = Moneychanger::It()->OT().Exec().SmartContract_AddHook(str_template, str_lawyer_id,
                                                                              bylaw_name, qstrNewName.toStdString(),
                                                                              qstrClauseName.toStdString());
            break;
        }
        case 4: // Callback
        {
            if (bylaw_name.empty())
            {
                QString qstrErrMsg(tr("Please select a bylaw first, to add the callback to."));

                if (0 == nBylawCount)
                    qstrErrMsg = QString(tr("You must create a bylaw first, before creating any callbacks."));

                QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME), qstrErrMsg);
                return;
            }
            // ------------------------------------------
            // First, let's make sure the same callback isn't already handled by the smart
            // contract. If it is, we pop up a message and return. Otherwise we can just
            // create the clause ourselves.
            //
            std::string str_clause = Moneychanger::It()->OT().Exec().Callback_GetClause(str_template, bylaw_name, qstrNewName.toStdString());

            if (!str_clause.empty())
            {
                QMessageBox::information(this, tr(MONEYCHANGER_APP_NAME),
                                         QString(tr("This callback already triggers clause '%1'. (Unable to add the same callback twice.) "
                                                    "Delete the callback first, if you wish to add it for a different clause.")).
                                         arg(QString::fromStdString(str_clause)) );
                return;
            }
            // ----------------------
            QString qstrClauseName = qstrNewName.right(qstrNewName.length() - 9); // Remove the "callback_" prefix to derive the clause name.
            QString qstrScript("return false;");

            std::string strTempResult = Moneychanger::It()->OT().Exec().SmartContract_AddClause(str_template, str_lawyer_id,
                                                                                           bylaw_name, qstrClauseName.toStdString(),
                                                                                           qstrScript.toStdString());
            if (!strTempResult.empty())
                str_template = strTempResult;
            // -------------------------------
            strSmartResult = Moneychanger::It()->OT().Exec().SmartContract_AddCallback(str_template, str_lawyer_id,
                                                                                  bylaw_name, qstrNewName.toStdString(),
                                                                                  qstrClauseName.toStdString());
            break;
        }
        } // switch
        // ------------------------------------------------
        if (strSmartResult.empty())
            return;

        int nTemplateID = m_pOwner->m_qstrCurrentID.toInt();
        bool bWritten = MTContactHandler::getInstance()->SetValueByID(nTemplateID, QString::fromStdString(strSmartResult),
                                                                      QString("template_contents"), QString("smart_contract"), QString("template_id"));
        // ------------------------------------------------
        if (bWritten)
        {
            m_qstrTemplate = QString::fromStdString(strSmartResult);
            emit RefreshRecordsAndUpdateMenu();
        }
        // ------------------------------------------------
    }
}



void MTAgreementDetails::on_pushButtonAddVariable_clicked()
{
    ui->comboBoxBylaw->setCurrentIndex(1);

    on_pushButtonNewBylaw_clicked();
}

void MTAgreementDetails::on_pushButtonDeleteVariable_clicked()
{
    if (m_qstrTemplate.isEmpty() || m_pOwner->m_qstrCurrentID.isEmpty())
        return;

    std::string str_template = m_qstrTemplate.toStdString();
    int nTemplateID = m_pOwner->m_qstrCurrentID.toInt();
    // ------------------------------------------------
    QString qstrLawyerID = m_pOwner->GetLawyerID();

    if (qstrLawyerID.isEmpty())
        return;

    std::string str_lawyer_id = qstrLawyerID.toStdString();
    // ------------------------------------------------
    std::string bylaw_name;

    int nCurrentIndex = ui->tableWidgetBylaws->currentRow();

    if (nCurrentIndex >= 0)
    {
        QWidget * widget = ui->tableWidgetBylaws->cellWidget(nCurrentIndex, 0); // 0 is column for the name.
        QLabel  * label  = dynamic_cast<QLabel*>(widget);

        if (NULL != label)
            bylaw_name = label->text().toStdString();
    }
    // ------------------------------
    if (bylaw_name.empty())
        return;
    // ------------------------------
    int nRow = ui->listWidgetVariables->currentRow();

    if (nRow < 0)
        return;

    QListWidgetItem * item = ui->listWidgetVariables->item(nRow);

    if (NULL == item)
        return;

    std::string str_name = item->text().toStdString();

    if (str_name.empty())
        return;
    // ------------------------------
    QMessageBox::StandardButton reply;

    reply = QMessageBox::question(this, tr(MONEYCHANGER_APP_NAME), tr("Are you sure you want to delete this Variable?"),
                                  QMessageBox::Yes|QMessageBox::No);
    if (QMessageBox::Yes != reply)
        return;
    // ------------------------------
    std::string strTempResult = Moneychanger::It()->OT().Exec().SmartContract_RemoveVariable(str_template, str_lawyer_id, bylaw_name, str_name);

    if (!strTempResult.empty())
    {
        bool bWritten = MTContactHandler::getInstance()->SetValueByID(nTemplateID, QString::fromStdString(strTempResult),
                                                                      QString("template_contents"), QString("smart_contract"), QString("template_id"));
        if (bWritten)
        {
            m_qstrTemplate = QString::fromStdString(strTempResult);
            emit RefreshRecordsAndUpdateMenu();
        }
    }
}


void MTAgreementDetails::on_pushButtonAddClause_clicked()
{
    ui->comboBoxBylaw->setCurrentIndex(2);

    on_pushButtonNewBylaw_clicked();
}

void MTAgreementDetails::on_pushButtonDeleteClause_clicked()
{
    if (m_qstrTemplate.isEmpty() || m_pOwner->m_qstrCurrentID.isEmpty())
        return;

    std::string str_template = m_qstrTemplate.toStdString();
    int nTemplateID = m_pOwner->m_qstrCurrentID.toInt();
    // ------------------------------------------------
    QString qstrLawyerID = m_pOwner->GetLawyerID();

    if (qstrLawyerID.isEmpty())
        return;

    std::string str_lawyer_id = qstrLawyerID.toStdString();
    // ------------------------------------------------
    std::string bylaw_name;

    int nCurrentIndex = ui->tableWidgetBylaws->currentRow();

    if (nCurrentIndex >= 0)
    {
        QWidget * widget = ui->tableWidgetBylaws->cellWidget(nCurrentIndex, 0); // 0 is column for the name.
        QLabel  * label  = dynamic_cast<QLabel*>(widget);

        if (NULL != label)
            bylaw_name = label->text().toStdString();
    }
    // ------------------------------
    if (bylaw_name.empty())
        return;
    // ------------------------------
    int nRow = ui->listWidgetClauses->currentRow();

    if (nRow < 0)
        return;

    QListWidgetItem * item = ui->listWidgetClauses->item(nRow);

    if (NULL == item)
        return;

    std::string str_name = item->text().toStdString();

    if (str_name.empty())
        return;
    // ------------------------------
    QMessageBox::StandardButton reply;

    reply = QMessageBox::question(this, tr(MONEYCHANGER_APP_NAME), tr("Are you sure you want to delete this Clause?"),
                                  QMessageBox::Yes|QMessageBox::No);
    if (QMessageBox::Yes != reply)
        return;
    // ------------------------------
    std::string strTempResult = Moneychanger::It()->OT().Exec().SmartContract_RemoveClause(str_template, str_lawyer_id, bylaw_name, str_name);

    if (!strTempResult.empty())
    {
        bool bWritten = MTContactHandler::getInstance()->SetValueByID(nTemplateID, QString::fromStdString(strTempResult),
                                                                      QString("template_contents"), QString("smart_contract"), QString("template_id"));
        if (bWritten)
        {
            m_qstrTemplate = QString::fromStdString(strTempResult);
            emit RefreshRecordsAndUpdateMenu();
        }
    }
}


void MTAgreementDetails::on_pushButtonAddHook_clicked()
{
    ui->comboBoxBylaw->setCurrentIndex(3);

    on_pushButtonNewBylaw_clicked();
}



void MTAgreementDetails::on_pushButtonAddCallback_clicked()
{
    ui->comboBoxBylaw->setCurrentIndex(4);

    on_pushButtonNewBylaw_clicked();
}

void MTAgreementDetails::on_pushButtonDeleteCallback_clicked()
{
    if (m_qstrTemplate.isEmpty() || m_pOwner->m_qstrCurrentID.isEmpty())
        return;

    std::string str_template = m_qstrTemplate.toStdString();
    int nTemplateID = m_pOwner->m_qstrCurrentID.toInt();
    // ------------------------------------------------
    QString qstrLawyerID = m_pOwner->GetLawyerID();

    if (qstrLawyerID.isEmpty())
        return;

    std::string str_lawyer_id = qstrLawyerID.toStdString();
    // ------------------------------------------------
    std::string bylaw_name;

    int nCurrentIndex = ui->tableWidgetBylaws->currentRow();

    if (nCurrentIndex >= 0)
    {
        QWidget * widget = ui->tableWidgetBylaws->cellWidget(nCurrentIndex, 0); // 0 is column for the name.
        QLabel  * label  = dynamic_cast<QLabel*>(widget);

        if (NULL != label)
            bylaw_name = label->text().toStdString();
    }
    // ------------------------------
    if (bylaw_name.empty())
        return;
    // ------------------------------
    int nRow = ui->listWidgetCallbacks->currentRow();

    if (nRow < 0)
        return;

    QListWidgetItem * item = ui->listWidgetCallbacks->item(nRow);

    if (NULL == item)
        return;

    std::string str_name = item->text().toStdString();

    if (str_name.empty())
        return;
    // ------------------------------
    QMessageBox::StandardButton reply;

    reply = QMessageBox::question(this, tr(MONEYCHANGER_APP_NAME), tr("Are you sure you want to delete this Callback?"),
                                  QMessageBox::Yes|QMessageBox::No);
    if (QMessageBox::Yes != reply)
        return;
    // ------------------------------
    std::string strTempResult = Moneychanger::It()->OT().Exec().SmartContract_RemoveCallback(str_template, str_lawyer_id, bylaw_name, str_name);

    if (!strTempResult.empty())
    {
        bool bWritten = MTContactHandler::getInstance()->SetValueByID(nTemplateID, QString::fromStdString(strTempResult),
                                                                      QString("template_contents"), QString("smart_contract"), QString("template_id"));
        if (bWritten)
        {
            m_qstrTemplate = QString::fromStdString(strTempResult);
            emit RefreshRecordsAndUpdateMenu();
        }
    }
}


void MTAgreementDetails::on_pushButtonAddAccount_clicked()
{
    ui->comboBoxParty->setCurrentIndex(2);

    on_pushButtonNewParty_clicked();
}

void MTAgreementDetails::on_pushButtonDeleteAccount_clicked()
{
    if (m_qstrTemplate.isEmpty() || m_pOwner->m_qstrCurrentID.isEmpty())
        return;

    std::string str_template = m_qstrTemplate.toStdString();
    int nTemplateID = m_pOwner->m_qstrCurrentID.toInt();
    // ------------------------------------------------
    QString qstrLawyerID = m_pOwner->GetLawyerID();

    if (qstrLawyerID.isEmpty())
        return;

    std::string str_lawyer_id = qstrLawyerID.toStdString();
    // ------------------------------------------------
    std::string party_name;

    int nCurrentIndex = ui->tableWidgetParties->currentRow();

    if (nCurrentIndex >= 0)
    {
        QWidget * widget = ui->tableWidgetParties->cellWidget(nCurrentIndex, 0); // 0 is column for the name.
        QLabel  * label  = dynamic_cast<QLabel*>(widget);

        if (NULL != label)
            party_name = label->text().toStdString();
    }
    // ------------------------------
    if (party_name.empty())
        return;
    // ------------------------------
    int nRow = ui->listWidgetAccounts->currentRow();

    if (nRow < 0)
        return;

    QListWidgetItem * item = ui->listWidgetAccounts->item(nRow);

    if (NULL == item)
        return;

    std::string str_name = item->text().toStdString();

    if (str_name.empty())
        return;
    // ------------------------------
    QMessageBox::StandardButton reply;

    reply = QMessageBox::question(this, tr(MONEYCHANGER_APP_NAME), tr("Are you sure you want to delete this Account?"),
                                  QMessageBox::Yes|QMessageBox::No);
    if (QMessageBox::Yes != reply)
        return;
    // ------------------------------
    std::string strTempResult = Moneychanger::It()->OT().Exec().SmartContract_RemoveAccount(str_template, str_lawyer_id, party_name, str_name);

    if (!strTempResult.empty())
    {
        bool bWritten = MTContactHandler::getInstance()->SetValueByID(nTemplateID, QString::fromStdString(strTempResult),
                                                                      QString("template_contents"), QString("smart_contract"), QString("template_id"));
        if (bWritten)
        {
            m_qstrTemplate = QString::fromStdString(strTempResult);
            emit RefreshRecordsAndUpdateMenu();
        }
    }
}

void MTAgreementDetails::on_pushButtonDeleteParty_clicked()
{
    if (m_qstrTemplate.isEmpty() || m_pOwner->m_qstrCurrentID.isEmpty())
        return;

    std::string str_template = m_qstrTemplate.toStdString();
    int nTemplateID = m_pOwner->m_qstrCurrentID.toInt();
    // ------------------------------------------------
    QString qstrLawyerID = m_pOwner->GetLawyerID();

    if (qstrLawyerID.isEmpty())
        return;

    std::string str_lawyer_id = qstrLawyerID.toStdString();
    // ------------------------------------------------
    std::string party_name;

    int nCurrentIndex = ui->tableWidgetParties->currentRow();

    if (nCurrentIndex >= 0)
    {
        QWidget * widget = ui->tableWidgetParties->cellWidget(nCurrentIndex, 0); // 0 is column for the name.
        QLabel  * label  = dynamic_cast<QLabel*>(widget);

        if (NULL != label)
            party_name = label->text().toStdString();
    }
    // ------------------------------
    if (party_name.empty())
        return;
    // ------------------------------
    QMessageBox::StandardButton reply;

    reply = QMessageBox::question(this, tr(MONEYCHANGER_APP_NAME), tr("Are you sure you want to delete this Party?"),
                                  QMessageBox::Yes|QMessageBox::No);
    if (QMessageBox::Yes != reply)
        return;
    // ------------------------------
    std::string strTempResult = Moneychanger::It()->OT().Exec().SmartContract_RemoveParty(str_template, str_lawyer_id, party_name);

    if (!strTempResult.empty())
    {
        bool bWritten = MTContactHandler::getInstance()->SetValueByID(nTemplateID, QString::fromStdString(strTempResult),
                                                                      QString("template_contents"), QString("smart_contract"), QString("template_id"));
        if (bWritten)
        {
            m_qstrTemplate = QString::fromStdString(strTempResult);
            emit RefreshRecordsAndUpdateMenu();
        }
    }
}

void MTAgreementDetails::on_pushButtonDeleteBylaw_clicked()
{
    if (m_qstrTemplate.isEmpty() || m_pOwner->m_qstrCurrentID.isEmpty())
        return;

    std::string str_template = m_qstrTemplate.toStdString();
    int nTemplateID = m_pOwner->m_qstrCurrentID.toInt();
    // ------------------------------------------------
    QString qstrLawyerID = m_pOwner->GetLawyerID();

    if (qstrLawyerID.isEmpty())
        return;

    std::string str_lawyer_id = qstrLawyerID.toStdString();
    // ------------------------------------------------
    std::string bylaw_name;

    int nCurrentIndex = ui->tableWidgetBylaws->currentRow();

    if (nCurrentIndex >= 0)
    {
        QWidget * widget = ui->tableWidgetBylaws->cellWidget(nCurrentIndex, 0); // 0 is column for the name.
        QLabel  * label  = dynamic_cast<QLabel*>(widget);

        if (NULL != label)
            bylaw_name = label->text().toStdString();
    }
    // ------------------------------
    if (bylaw_name.empty())
        return;
    // ------------------------------
    QMessageBox::StandardButton reply;

    reply = QMessageBox::question(this, tr(MONEYCHANGER_APP_NAME), tr("Are you sure you want to delete this Bylaw?"),
                                  QMessageBox::Yes|QMessageBox::No);
    if (QMessageBox::Yes != reply)
        return;
    // ------------------------------
    std::string strTempResult = Moneychanger::It()->OT().Exec().SmartContract_RemoveBylaw(str_template, str_lawyer_id, bylaw_name);

    if (!strTempResult.empty())
    {
        bool bWritten = MTContactHandler::getInstance()->SetValueByID(nTemplateID, QString::fromStdString(strTempResult),
                                                                      QString("template_contents"), QString("smart_contract"), QString("template_id"));
        if (bWritten)
        {
            m_qstrTemplate = QString::fromStdString(strTempResult);
            emit RefreshRecordsAndUpdateMenu();
        }
    }
}

void MTAgreementDetails::on_pushButtonNewParty_clicked()
{
    if (m_qstrTemplate.isEmpty() || m_pOwner->m_qstrCurrentID.isEmpty())
        return;

    std::string str_template = m_qstrTemplate.toStdString();
    int nTemplateID = m_pOwner->m_qstrCurrentID.toInt();
    // ------------------------------------------------
    QString qstrLawyerID = m_pOwner->GetLawyerID();

    if (qstrLawyerID.isEmpty())
        return;

    std::string str_lawyer_id = qstrLawyerID.toStdString();
    // ------------------------------------------------
    QString qstrNewName("");

    int nCurrentComboIndex = ui->comboBoxParty->currentIndex();
    // ------------------------------------------------
    if ((nCurrentComboIndex >= 0) && (nCurrentComboIndex <= 2))
    {
        MTGetStringDialog nameDlg(this, tr("Enter a Name:"));

        switch (nCurrentComboIndex)
        {
        case 0:
            nameDlg.setWindowTitle(tr("New Party"));
            break;
        case 1:
            nameDlg.setWindowTitle(tr("New Agent"));
            break;
        case 2:
            nameDlg.setWindowTitle(tr("New Account"));
            break;
        }
        // --------------
        if (QDialog::Accepted == nameDlg.exec())
        {
            qstrNewName = nameDlg.GetOutputString();
        }
        else
            return;
    }
    else
        return;
    // ---------------------------------------------
    if (!qstrNewName.isEmpty())
    {
        std::string strSmartResult;

        switch (nCurrentComboIndex)
        {
        case 0:
        {
            std::string strParty = "party_"+qstrNewName.toStdString();
            std::string strAgent = "agent_"+qstrNewName.toStdString();

            QString qstrPartyNymID;

            if (ui->checkBoxSpecifyNym->isChecked())
            {
                MTGetStringDialog stringDlg(this, tr("Paste a NymID:"));

                stringDlg.setWindowTitle(tr("New Party"));

                if (QDialog::Accepted == stringDlg.exec())
                    qstrPartyNymID = stringDlg.GetOutputString();
                else
                    return;
             }

            strSmartResult = Moneychanger::It()->OT().Exec().SmartContract_AddParty(str_template, str_lawyer_id, qstrPartyNymID.toStdString(),
                                                                               strParty, strAgent);
            break;
        }
        case 1:
        {
            QMessageBox::information(this, tr("Coming soon"), tr("No need to add agents until we have corporations working."));
            return;
        }
        case 2:
        {
            int nCurrentIndex = ui->tableWidgetParties->currentRow();

            if (nCurrentIndex < 0)
                return;

            QWidget * widget = ui->tableWidgetParties->cellWidget(nCurrentIndex, 0); // 0 is column for the name.
            QLabel  * label  = dynamic_cast<QLabel*>(widget);

            if (NULL == label)
                return;

            std::string party_name = label->text().toStdString();
            if (party_name.empty())
                return;

            std::string strAcct = "acct_"+qstrNewName.toStdString();
            std::string strType("");
            // -------------------------------
            if (ui->checkBoxSpecifyAsset->isChecked())
            {
                // -------------------------------------------
                // Select from Asset Types in local wallet.
                //
                DlgChooser theChooser(this);
                // -----------------------------------------------
                mapIDName & the_map = theChooser.m_map;
                // -----------------------------------------------
                const int32_t the_count = Moneychanger::It()->OT().Exec().GetAssetTypeCount();
                // -----------------------------------------------
                for (int32_t ii = 0; ii < the_count; ++ii)
                {
                    QString OT_id = QString::fromStdString(Moneychanger::It()->OT().Exec().GetAssetType_ID(ii));
                    QString OT_name("");
                    // -----------------------------------------------
                    if (!OT_id.isEmpty())
                    {
                        OT_name = QString::fromStdString(Moneychanger::It()->OT().Exec().GetAssetType_Name(OT_id.toStdString()));
                        // -----------------------------------------------
                        the_map.insert(OT_id, OT_name);
                    }
                 }
                // -----------------------------------------------
                theChooser.setWindowTitle(tr("Select the Asset Type"));
                // -----------------------------------------------
                if (theChooser.exec() == QDialog::Accepted)
                {
                    if (!theChooser.m_qstrCurrentID.isEmpty())
                    {
                        strType = theChooser.m_qstrCurrentID.toStdString();
                    }
                }
            }

            strSmartResult = Moneychanger::It()->OT().Exec().SmartContract_AddAccount(str_template,
                                                           str_lawyer_id,
                                                           party_name,	// The Party's NAME as referenced in the smart contract.
                                                           // ----------------------------------------
                                                           strAcct,         // The Account's name as referenced in the smart contract
                                                           strType);        // Asset type for the account.

            break;
        }
        }
        // ------------------------------------------------
        if (strSmartResult.empty())
            return;

        int nTemplateID = m_pOwner->m_qstrCurrentID.toInt();
        bool bWritten = MTContactHandler::getInstance()->SetValueByID(nTemplateID, QString::fromStdString(strSmartResult),
                                                                      QString("template_contents"), QString("smart_contract"), QString("template_id"));
        // ------------------------------------------------
        if (bWritten)
        {
            m_qstrTemplate = QString::fromStdString(strSmartResult);
            emit RefreshRecordsAndUpdateMenu();
        }
        // ------------------------------------------------
    }
}


void MTAgreementDetails::on_lineEditName_editingFinished()
{
    if (m_qstrTemplate.isEmpty() || m_pOwner->m_qstrCurrentID.isEmpty())
        return;

    int nTemplateID = m_pOwner->m_qstrCurrentID.toInt();
    // ------------------------------------------------
    bool bWritten = MTContactHandler::getInstance()->SetValueByID(nTemplateID, ui->lineEditName->text(),
                                                                  QString("template_display_name"), QString("smart_contract"), QString("template_id"));
    // ------------------------------------------------
    if (bWritten)
    {
        m_pOwner->m_map.remove(m_pOwner->m_qstrCurrentID);
        m_pOwner->m_map.insert(m_pOwner->m_qstrCurrentID, ui->lineEditName->text());

        m_pOwner->SetPreSelected(m_pOwner->m_qstrCurrentID);
        // ------------------------------------------------
        emit RefreshRecordsAndUpdateMenu();
        // ------------------------------------------------
    }
}

void MTAgreementDetails::on_comboBoxVariableType_currentIndexChanged(int index)
{
    switch (index)
    {
    case 0: // String
        ui->lineEditVariableValue->setVisible(false);
        ui->comboBoxVariableValue->setVisible(false);
        ui->plainTextEditVariableValue->setVisible(true);
        break;
    case 1: // Integer
        ui->lineEditVariableValue->setVisible(true);
        ui->comboBoxVariableValue->setVisible(false);
        ui->plainTextEditVariableValue->setVisible(false);
         break;
    case 2: // Bool
        ui->lineEditVariableValue->setVisible(false);
        ui->comboBoxVariableValue->setVisible(true);
        ui->plainTextEditVariableValue->setVisible(false);
        break;
    default:
        ui->lineEditVariableValue->setVisible(false);
        ui->comboBoxVariableValue->setVisible(false);
        ui->plainTextEditVariableValue->setVisible(false);
        break;
    }
}




void MTAgreementDetails::on_pushButtonSave_clicked()
{
    ui->pushButtonCancel->setEnabled(false);
    ui->pushButtonSave  ->setEnabled(false);

    std::string bylaw_name;

    if (ui->tableWidgetBylaws->currentRow() >= 0)
    {
        if (m_qstrTemplate.isEmpty())
            return;
        // ----------------------------------
        std::string str_template = m_qstrTemplate.toStdString();
        // ----------------------------------
        QWidget * widget = ui->tableWidgetBylaws->cellWidget(ui->tableWidgetBylaws->currentRow(), 0); // 0 is column for the name.
        QLabel  * label  = dynamic_cast<QLabel*>(widget);

        if (NULL != label)
        {
            bylaw_name = label->text().toStdString();
            // ---------------------------------------
            int nRow = ui->listWidgetClauses->currentRow();

            if (nRow >= 0)
            {
                QListWidgetItem * item = ui->listWidgetClauses->item(nRow);

                if (NULL != item)
                {
                    std::string str_name   = item->text().toStdString();

                    QString qstrLawyerID = m_pOwner->GetLawyerID();

                    if (qstrLawyerID.isEmpty())
                        return;
                    // ------------------------------------------
                    QString qstrScript = ui->plainTextEditScript->toPlainText();

                    std::string strTemp = Moneychanger::It()->OT().Exec().SmartContract_UpdateClause(str_template,
                                                                                                qstrLawyerID.toStdString(),
                                                                                                bylaw_name,
                                                                                                str_name,
                                                                                                qstrScript.toStdString());

                    if (!strTemp.empty())
                    {
                        int nTemplateID = m_pOwner->m_qstrCurrentID.toInt();

                        if (nTemplateID > 0)
                        {
                            bool bWritten = MTContactHandler::getInstance()->SetValueByID(nTemplateID, QString::fromStdString(strTemp),
                                                                                          QString("template_contents"), QString("smart_contract"), QString("template_id"));
                            if (bWritten)
                            {
                                m_qstrTemplate = QString::fromStdString(strTemp);
                                if (m_pPlainTextEdit)
                                    m_pPlainTextEdit->setPlainText(m_qstrTemplate);
                            }
                        }
                    }
                }
            }
            else
            {
                ui->lineEditClauseName->setText(QString(""));

                ui->plainTextEditScript->blockSignals(true);
                ui->plainTextEditScript->setPlainText(QString(""));
                ui->plainTextEditScript->blockSignals(false);
            }
        }
    }
}

void MTAgreementDetails::on_pushButtonCancel_clicked()
{
    ui->pushButtonCancel->setEnabled(false);
    ui->pushButtonSave  ->setEnabled(false);

    std::string bylaw_name;

    if (ui->tableWidgetBylaws->currentRow() >= 0)
    {
        if (m_qstrTemplate.isEmpty())
            return;
        // ----------------------------------
        std::string str_template = m_qstrTemplate.toStdString();
        // ----------------------------------
        QWidget * widget = ui->tableWidgetBylaws->cellWidget(ui->tableWidgetBylaws->currentRow(), 0); // 0 is column for the name.
        QLabel  * label  = dynamic_cast<QLabel*>(widget);

        if (NULL != label)
        {
            bylaw_name = label->text().toStdString();
            // ---------------------------------------
            int nRow = ui->listWidgetClauses->currentRow();

            if (nRow >= 0)
            {
                QListWidgetItem * item = ui->listWidgetClauses->item(nRow);

                if (NULL != item)
                {
                    std::string str_name   = item->text().toStdString();
                    std::string str_script = Moneychanger::It()->OT().Exec().Clause_GetContents(str_template, bylaw_name, str_name);

                    ui->plainTextEditScript->blockSignals(true);
                    ui->plainTextEditScript->setPlainText(QString::fromStdString(str_script));
                    ui->plainTextEditScript->blockSignals(false);
                }
            }
            else
            {
                ui->lineEditClauseName->setText(QString(""));

                ui->plainTextEditScript->blockSignals(true);
                ui->plainTextEditScript->setPlainText(QString(""));
                ui->plainTextEditScript->blockSignals(false);
            }
        }
    }
}



void MTAgreementDetails::on_pushButtonRunContract_clicked()
{
    if (m_qstrTemplate.isEmpty() || m_pOwner->m_qstrCurrentID.isEmpty())
        return;
    // ------------------------------------------------
    QString qstrLawyerID = m_pOwner->GetLawyerID();

    if (qstrLawyerID.isEmpty())
        return;
    // ------------------------------------------------
    emit runSmartContract(m_qstrTemplate, qstrLawyerID, -1); // -1 instead of a payments inbox index, which is what I'd otherwise pass.
}

void MTAgreementDetails::onGroupButtonValidFromClicked(int nButton)
{
    time64_t theTime = 0;

    if (-2 == nButton)
    {
        ui->dateTimeEditValidFrom->setEnabled(false);

        theTime    = 0;
        m_dateTimeValidFrom = QDateTime::fromTime_t(theTime);

        QDateTime currentDateTime = QDateTime::currentDateTime();

        ui->dateTimeEditValidFrom->blockSignals(true);
        ui->dateTimeEditValidFrom->setDateTime(currentDateTime);
        ui->dateTimeEditValidFrom->blockSignals(false);

        // Since we just changed the "valid from" date, we have to make sure
        // that the "valid to" date is 0, or is validfrom+30 days.
        //
        QDateTime dateInValidToEditBox = ui->dateTimeEditValidTo->dateTime();

        if ((m_dateTimeValidTo > QDateTime::fromTime_t(0)) && (dateInValidToEditBox < currentDateTime.addDays(30)))
        {
            m_dateTimeValidTo = currentDateTime.addDays(30);
            ui->dateTimeEditValidTo->blockSignals(true);
            ui->dateTimeEditValidTo->setDateTime(m_dateTimeValidTo);
            ui->dateTimeEditValidTo->blockSignals(false);
        }
    }
    else if (-3 == nButton)
    {
        ui->dateTimeEditValidFrom->setEnabled(true);

        theTime    = ui->dateTimeEditValidFrom->dateTime().toTime_t();
        m_dateTimeValidFrom = QDateTime::fromTime_t(theTime);

        {
            theTime  = ui->dateTimeEditValidTo->dateTime().toTime_t();
            QDateTime currentDateTime = QDateTime::currentDateTime();
            QDateTime dateInValidFromEditBox = ui->dateTimeEditValidFrom->dateTime();

            if (ui->dateTimeEditValidTo->isEnabled() &&
                (
                 (0 == theTime) ||
                 ((theTime > 0) && (QDateTime::fromTime_t(theTime) < dateInValidFromEditBox.addDays(30)))
                )
               )
            {
                m_dateTimeValidTo = currentDateTime.addDays(30); // expires in 30 days.;

                ui->dateTimeEditValidTo->blockSignals(true);
                ui->dateTimeEditValidTo->setDateTime(m_dateTimeValidTo);
                ui->dateTimeEditValidTo->blockSignals(false);
            }

        }
    }
    //-----------------------------
    SaveDates();
}

void MTAgreementDetails::onGroupButtonValidToClicked(int nButton)
{
    time64_t theTime = 0;

    if (-2 == nButton)
    {
        ui->dateTimeEditValidTo->setEnabled(false);

        theTime  = 0;
        m_dateTimeValidTo = QDateTime::fromTime_t(theTime);
    }
    else if (-3 == nButton)
    {
        ui->dateTimeEditValidTo->setEnabled(true);

        theTime  = ui->dateTimeEditValidTo->dateTime().toTime_t();
        QDateTime currentDateTime = QDateTime::currentDateTime();
        QDateTime dateInValidFromEditBox = ui->dateTimeEditValidFrom->dateTime();

        if ((0 == theTime) ||
            ((theTime > 0) && (QDateTime::fromTime_t(theTime) < dateInValidFromEditBox.addDays(30))))
        {
            m_dateTimeValidTo = currentDateTime.addDays(30); // expires in 30 days.;

            ui->dateTimeEditValidTo->blockSignals(true);
            ui->dateTimeEditValidTo->setDateTime(m_dateTimeValidTo);
            ui->dateTimeEditValidTo->blockSignals(false);
        }
        else
            m_dateTimeValidTo = QDateTime::fromTime_t(theTime);
    }
    //-----------------------------
    SaveDates();
}

void MTAgreementDetails::SaveDates()
{
    if (m_qstrTemplate.isEmpty() || m_pOwner->m_qstrCurrentID.isEmpty())
        return;

    std::string str_template = m_qstrTemplate.toStdString();
    int nTemplateID = m_pOwner->m_qstrCurrentID.toInt();
    // ------------------------------------------------
    QString qstrLawyerID = m_pOwner->GetLawyerID();

    if (qstrLawyerID.isEmpty())
        return;

    std::string str_lawyer_id = qstrLawyerID.toStdString();
    // ------------------------------------------------
    time64_t validFrom = m_dateTimeValidFrom.toTime_t();
    time64_t validTo   = m_dateTimeValidTo  .toTime_t();
    // ------------------------------------------------------
    std::string strTempResult = Moneychanger::It()->OT().Exec().SmartContract_SetDates(str_template, str_lawyer_id, validFrom, validTo);

    if (!strTempResult.empty())
    {
        bool bWritten = MTContactHandler::getInstance()->SetValueByID(nTemplateID, QString::fromStdString(strTempResult),
                                                                      QString("template_contents"), QString("smart_contract"), QString("template_id"));
        // ------------------------------------------------
        if (bWritten)
        {
            m_qstrTemplate = QString::fromStdString(strTempResult);
            if (m_pPlainTextEdit)
                m_pPlainTextEdit->setPlainText(m_qstrTemplate);

            if (!m_qstrTemplate.isEmpty())
            {
                std::string contract_id = Moneychanger::It()->OT().Exec().CalculateContractID(strTempResult);
                ui->lineEditContractID->setText(QString::fromStdString(contract_id));
            }
        }
    }
}

void MTAgreementDetails::on_dateTimeEditValidFrom_dateTimeChanged(const QDateTime &dateTime)
{
    time64_t theTime = dateTime.toTime_t();
    m_dateTimeValidFrom = QDateTime::fromTime_t(theTime);

    SaveDates();
}

void MTAgreementDetails::on_dateTimeEditValidTo_dateTimeChanged(const QDateTime &dateTime)
{
    time64_t theTime = dateTime.toTime_t();
    m_dateTimeValidTo = QDateTime::fromTime_t(theTime);

    SaveDates();
}

void MTAgreementDetails::on_plainTextEditScript_textChanged()
{
    ui->pushButtonCancel->setEnabled(true);
    ui->pushButtonSave  ->setEnabled(true);
}

void MTAgreementDetails::on_toolButtonCopy_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();

    if (NULL != clipboard)
    {
        clipboard->setText(ui->lineEditContractID->text());

        QMessageBox::information(this, tr("ID copied"), QString("%1:<br/>%2").
                                 arg(tr("Copied Contract ID to the clipboard")).
                                 arg(ui->lineEditContractID->text()));
    }
}

void MTAgreementDetails::on_toolButtonAssetID_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();

    if (NULL != clipboard)
    {
        clipboard->setText(ui->lineEditAssetID->text());

        QMessageBox::information(this, tr("ID copied"), QString("%1:<br/>%2").
                                 arg(tr("Copied Asset ID to the clipboard")).
                                 arg(ui->lineEditAssetID->text()));
    }
}

void MTAgreementDetails::on_checkBoxSpecifyAsset_toggled(bool checked)
{
    ui->groupBoxAssetType->setVisible(checked);
}


void MTAgreementDetails::on_checkBoxSpecifyNym_toggled(bool checked)
{
    ui->labelPartyNymID->setVisible(checked);
    ui->lineEditPartyNymID->setVisible(checked);
}


void MTAgreementDetails::on_toolButtonPartyNymID_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();

    if (NULL != clipboard)
    {
        clipboard->setText(ui->lineEditPartyNymID->text());

        QMessageBox::information(this, tr("ID copied"), QString("%1:<br/>%2").
                                 arg(tr("Copied Nym ID to the clipboard")).
                                 arg(ui->lineEditPartyNymID->text()));
    }
}
