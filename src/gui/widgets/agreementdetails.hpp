#ifndef AGREEMENTDETAILS_HPP
#define AGREEMENTDETAILS_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <gui/widgets/editdetails.hpp>

#include <core/handlers/contacthandler.hpp>
#include <core/filedownloader.hpp>

#include <QPlainTextEdit>

#include <string>

namespace Ui {
class MTAgreementDetails;
}

class MTAgreementDetails : public MTEditDetails
{
    Q_OBJECT

public:
    explicit MTAgreementDetails(QWidget *parent, MTDetailEdit & theOwner);
    ~MTAgreementDetails();

    virtual void refresh(QString strID, QString strName);
    virtual void AddButtonClicked();
    virtual void DeleteButtonClicked();

    virtual void ClearContents();

    void PopulateBylaws();
    void PopulateParties();

    void PopulateBylawGrid();
    void PopulatePartyGrid();

    void ClearBylaws();
    void ClearParties();

    void ClearBylawsGrid();
    void ClearPartiesGrid();

    void ClearBylawMap();
    void ClearPartyMap();

    void RefreshAgents(std::string str_template, std::string str_party);
    void RefreshAccounts(std::string str_template, std::string str_party);
    void ClearAgents();
    void ClearAccounts();

    void RefreshVariables(std::string str_template, std::string str_bylaw);
    void RefreshClauses(std::string str_template, std::string str_bylaw);
    void RefreshHooks(std::string str_template, std::string str_bylaw);
    void RefreshCallbacks(std::string str_template, std::string str_bylaw);
    void ClearVariables();
    void ClearClauses();
    void ClearHooks();
    void ClearCallbacks();

    void RefreshBylawDetails(int nRow);
    void RefreshPartyDetails(int nRow);

    void RefreshAgentDetails(int nRow);
    void RefreshAccountDetails(int nRow);
    void RefreshVariableDetails(int nRow);
    void RefreshClauseDetails(int nRow);
    void RefreshHookDetails(int nRow);
    void RefreshCallbackDetails(int nRow);

    void ImportContract(QString qstrContents);

    // ----------------------------------
    // Only used on construction (aka when dialog() is called for first time.)
    //
    virtual int       GetCustomTabCount();
    virtual QWidget * CreateCustomTab (int nTab);
    virtual QString   GetCustomTabName(int nTab);
    // ----------------------------------
    void SaveDates();

private:
    QPointer<QPlainTextEdit> m_pPlainTextEdit;

protected:
    bool eventFilter(QObject *obj, QEvent *event);

    void FavorLeftSideForIDs();

    QPointer<FileDownloader> m_pDownloader;

private slots:
    void on_toolButtonCopy_clicked();
    void on_toolButtonAssetID_clicked();
    void on_tableWidgetBylaws_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void on_tableWidgetParties_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void on_listWidgetVariables_currentRowChanged(int currentRow);
    void on_listWidgetClauses_currentRowChanged(int currentRow);
    void on_listWidgetHooks_currentRowChanged(int currentRow);
    void on_listWidgetCallbacks_currentRowChanged(int currentRow);
    void on_listWidgetAgents_currentRowChanged(int currentRow);
    void on_listWidgetAccounts_currentRowChanged(int currentRow);
    void on_pushButtonNewBylaw_clicked();
    void on_pushButtonNewParty_clicked();
    void on_comboBoxVariableType_currentIndexChanged(int index);
    void on_plainTextEditScript_textChanged();
    void on_pushButtonSave_clicked();
    void on_pushButtonCancel_clicked();
    void on_btnHookDelete_clicked();
    void on_pushButtonAddVariable_clicked();
    void on_pushButtonDeleteVariable_clicked();
    void on_pushButtonAddClause_clicked();
    void on_pushButtonDeleteClause_clicked();
    void on_pushButtonAddHook_clicked();
    void on_pushButtonAddCallback_clicked();
    void on_pushButtonDeleteCallback_clicked();
    void on_pushButtonAddAccount_clicked();
    void on_pushButtonDeleteAccount_clicked();
    void on_pushButtonDeleteParty_clicked();
    void on_pushButtonDeleteBylaw_clicked();
    void on_lineEditName_editingFinished();
    void DownloadedURL();
    void on_pushButtonRunContract_clicked();
    void onGroupButtonValidFromClicked(int nButton);
    void onGroupButtonValidToClicked(int nButton);
    void on_dateTimeEditValidFrom_dateTimeChanged(const QDateTime &dateTime);
    void on_dateTimeEditValidTo_dateTimeChanged(const QDateTime &dateTime);
    void on_checkBoxSpecifyAsset_toggled(bool checked);
    void on_checkBoxSpecifyNym_toggled(bool checked);
    void on_toolButtonPartyNymID_clicked();

private:
    QWidget * createHookGroupWidget (std::string & str_template, std::string & bylaw_name, std::string & hook_name);
    QWidget * createSingleHookWidget(std::string & bylaw_name, std::string & hook_name, std::string & clause_name);

    QPointer<QWidget> m_pHookWidget;

    mapIDName m_mapBylaws;
    mapIDName m_mapParties;

    QDateTime m_dateTimeValidFrom;
    QDateTime m_dateTimeValidTo;

    // Probably don't need pointers, so I'm using a regular mapIDName.
//    QMap <QString, QVariant> m_mapBylaws;
//    QMap <QString, QVariant> m_mapParties;

    QString   m_qstrTemplate;
    QWidget * m_pHeaderWidget;
    Ui::MTAgreementDetails *ui;
};

#endif // AGREEMENTDETAILS_HPP
