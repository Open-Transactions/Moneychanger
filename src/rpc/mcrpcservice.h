#ifndef MCRPCSERVICE_H
#define MCRPCSERVICE_H

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"
#include "rpcuser.h"
#include "rpcusermanager.h"

#include <qjsonrpcservice.h>
#include <opentxs/client/OTRecordList.hpp>
#include <opentxs/core/crypto/OTPassword.hpp>


class MCRPCService : public QJsonRpcService
{

    typedef qint64 time64_t;

private:
    Q_OBJECT
    Q_CLASSINFO("serviceName", "moneychanger")

public:
    MCRPCService(QObject *parent = 0);
    ~MCRPCService();

public Q_SLOTS:

    // opentxs::OTAPI methods
    QJsonValue numListAdd(QString Username, QString APIKey,
                          QString NumList, QString Numbers);
    QJsonValue numListRemove(QString Username, QString APIKey,
                             QString NumList, QString Numbers);
    QJsonValue numListVerifyQuery(QString Username, QString APIKey,
                                  QString NumList, QString Numbers);
    QJsonValue numListVerifyAll(QString Username, QString APIKey,
                                QString NumList, QString Numbers);
    QJsonValue numListCount(QString Username, QString APIKey,
                            QString NumList);
    QJsonValue createNymLegacy(QString Username, QString APIKey,
                         int KeySize, QString NymIDSource);
    QJsonValue createNymEcdsa(QString Username, QString APIKey,
                              QString NymIDSource);
    QJsonValue getNymActiveCronItemIDs(QString Username, QString APIKey,
                                       QString NymID, QString NotaryID);
    QJsonValue getActiveCronItem(QString Username, QString APIKey,
                                 QString NotaryID, qint64 TransNum);
    QJsonValue getNymSourceForID(QString Username, QString APIKey,
                                 QString NymID);
    QJsonValue getNymDescription(QString Username, QString APIKey,
                                       QString NymID);
    QJsonValue getNymMasterCredentialCount(QString Username, QString APIKey,
                                     QString NymID);
    QJsonValue getNymMasterCredentialID(QString Username, QString APIKey,
                                  QString NymID, int Index);
    QJsonValue getNymMasterCredentialContents(QString Username, QString APIKey,
                                        QString NymID, QString CredentialID);
    QJsonValue getNymRevokedCount(QString Username, QString APIKey,
                                  QString NymID);
    QJsonValue getNymRevokedCredID(QString Username, QString APIKey,
                                   QString NymID, int Index);
    QJsonValue getNymRevokedCredContents(QString Username, QString APIKey,
                                         QString NymID, QString CredentialID);
    QJsonValue getNymChildCredentialCount(QString Username, QString APIKey,
                                        QString NymID, QString MasterCredID);
    QJsonValue getNymChildCredentialID(QString Username, QString APIKey,
                                     QString NymID, QString MasterCredID,
                                     int Index);
    QJsonValue getNymChildCredentialContents(QString Username, QString APIKey,
                                           QString NymID, QString MasterCredID,
                                           QString SubCredID);
    QJsonValue revokeChildcredential(QString Username, QString APIKey,
                                   QString NymID, QString MasterCredID,
                                   QString SubCredID);
    QJsonValue getSignerNymID(QString Username, QString APIKey,
                              QString Contract);
    QJsonValue calculateUnitDefinitionID(QString Username, QString APIKey,
                                        QString Contract);
    QJsonValue calculateServerContractID(QString Username, QString APIKey,
                                         QString Contract);
    QJsonValue calculateContractID(QString Username, QString APIKey,
                                   QString Contract);
    QJsonValue createCurrencyContract(
        QString Username,
        QString APIKey,
        QString NymID,
        QString shortname,
        QString terms,
        QString names,
        QString symbol,
        QString tla,
        QString factor,
        QString power,
        QString fraction);
    QJsonValue getServerContract(QString Username, QString APIKey,
                                 QString NotaryID);
    QJsonValue getCurrencyFactor(QString Username, QString APIKey,
                                 QString InstrumentDefinitionID);
    QJsonValue getCurrencyDecimalPower(QString Username, QString APIKey,
                                       QString InstrumentDefinitionID);
    QJsonValue getCurrencyTLA(QString Username, QString APIKey,
                              QString InstrumentDefinitionID);
    QJsonValue getCurrencySymbol(QString Username, QString APIKey,
                                 QString InstrumentDefinitionID);
    QJsonValue stringToAmountLocale(QString Username, QString APIKey,
                                    QString InstrumentDefinitionID, QString Input,
                                    QString ThousandsSep, QString DecimalPoint);
    QJsonValue formatAmountLocale(QString Username, QString APIKey,
                                  QString InstrumentDefinitionID, qint64 Amount,
                                  QString ThousandsSep, QString DecimalPoint);
    QJsonValue formatAmountWithoutSymbolLocale(QString Username, QString APIKey,
                                               QString InstrumentDefinitionID, qint64 Amount,
                                               QString ThousandsSep, QString DecimalPoint);
    QJsonValue stringToAmount(QString Username, QString APIKey,
                              QString InstrumentDefinitionID, QString Input);
    QJsonValue formatAmount(QString Username, QString APIKey,
                            QString InstrumentDefinitionID, qint64 Amount);
    QJsonValue formatAmountWithoutSymbol(QString Username, QString APIKey,
                                         QString InstrumentDefinitionID, qint64 Amount);
    QJsonValue getAssetTypeContract(QString Username, QString APIKey,
                                    QString InstrumentDefinitionID);
    QJsonValue addServerContract(QString Username, QString APIKey,
                                 QString Contract);
    QJsonValue addUnitDefinition(QString Username, QString APIKey,
                                QString Contract);
    QJsonValue getNymCount(QString Username, QString APIKey);
    QJsonValue getServerCount(QString Username, QString APIKey);
    QJsonValue getAssetTypeCount(QString Username, QString APIKey);
    QJsonValue getAccountCount(QString Username, QString APIKey);
    QJsonValue walletCanRemoveServer(QString Username, QString APIKey,
                                     QString NotaryID);
    QJsonValue walletRemoveServer(QString Username, QString APIKey,
                                  QString NotaryID);
    QJsonValue walletCanRemoveAssetType(QString Username, QString APIKey,
                                        QString InstrumentDefinitionID);
    QJsonValue walletRemoveAssetType(QString Username, QString APIKey,
                                     QString InstrumentDefinitionID);
    QJsonValue walletCanRemoveNym(QString Username, QString APIKey,
                                  QString NymID);
    QJsonValue walletRemoveNym(QString Username, QString APIKey,
                               QString NymID);
    QJsonValue walletCanRemoveAccount(QString Username, QString APIKey,
                                      QString AccountID);
    QJsonValue doesBoxReceiptExist(QString Username, QString APIKey,
                                   QString NotaryID, QString NymID,
                                   QString AccountID, int BoxType,
                                   qint64 TransactionNumber);
    QJsonValue getBoxReceipt(QString Username, QString APIKey,
                             QString NotaryID, QString NymID,
                             QString AccountID, int BoxType,
                             qint64 TransactionNumber);
    QJsonValue deleteAssetAccount(QString Username, QString APIKey,
                                  QString NotaryID, QString NymID, QString AccountID);
    QJsonValue walletExportNym(QString Username, QString APIKey,
                               QString NymID);
    QJsonValue walletImportNym(QString Username, QString APIKey,
                               QString FileContents);
// QJsonValue walletChangePassphrase(void);
    QJsonValue walletGetNymIdFromPartial(QString Username, QString APIKey,
                                         QString PartialID);
    QJsonValue walletGetNotaryIdFromPartial(QString Username, QString APIKey,
                                            QString PartialID);
    QJsonValue walletGetInstrumentDefinitionIdFromPartial(QString Username, QString APIKey,
                                                          QString PartialID);
    QJsonValue walletGetAccountIdFromPartial(QString Username, QString APIKey,
                                             QString PartialID);
    QJsonValue getNymID(QString Username, QString APIKey,
                        int Index);
    QJsonValue getNymName(QString Username, QString APIKey,
                          QString NymID);
    QJsonValue isNymRegisteredAtServer(QString Username, QString APIKey,
                                       QString NymID, QString NotaryID);
    QJsonValue getNymStats(QString Username, QString APIKey,
                           QString NymID);
    QJsonValue getNymNymboxHash(QString Username, QString APIKey,
                                QString NotaryID, QString NymID);
    QJsonValue getNymRecentHash(QString Username, QString APIKey,
                                QString NotaryID, QString NymID);
    QJsonValue getNymInboxHash(QString Username, QString APIKey,
                               QString NotaryID, QString NymID);
    QJsonValue getNymOutboxHash(QString Username, QString APIKey,
                                QString NotaryID, QString NymID);
    QJsonValue getNymMailCount(QString Username, QString APIKey,
                               QString NymID);
    QJsonValue getNymContentsByIndex(QString Username, QString APIKey,
                                     QString NymID, int Index);
    QJsonValue getNymMailSenderIDByIndex(QString Username, QString APIKey,
                                         QString NymID, int Index);
    QJsonValue getNymMailNotaryIDByIndex(QString Username, QString APIKey,
                                         QString NymID, int Index);
    QJsonValue nymRemoveMailByIndex(QString Username, QString APIKey,
                                    QString NymID, int Index);
    QJsonValue nymVerifyMailByIndex(QString Username, QString APIKey,
                                    QString NymID, int Index);
    QJsonValue getNymOutmailCount(QString Username, QString APIKey,
                                  QString NymID);
    QJsonValue getNymOutmailContentsByIndex(QString Username, QString APIKey,
                                            QString NymID, int Index);
    QJsonValue getNymOutmailRecipientIDByIndex(QString Username, QString APIKey,
                                               QString NymID, int Index);
    QJsonValue getNymOutmailNotaryIDByIndex(QString Username, QString APIKey,
                                            QString NymID, int Index);
    QJsonValue nymRemoveOutmailByIndex(QString Username, QString APIKey,
                                       QString NymID, int Index);
    QJsonValue nymVerifyOutmailByIndex(QString Username, QString APIKey,
                                       QString NymID, int Index);
    QJsonValue getNymOutpaymentsCount(QString Username, QString APIKey,
                                      QString NymID);
    QJsonValue getNymOutpaymentsContentsByIndex(QString Username, QString APIKey,
                                                QString NymID, int Index);
    QJsonValue getNymOutpaymentsRecipientIDByIndex(QString Username, QString APIKey,
                                                   QString NymID, int Index);
    QJsonValue getNymOutpaymentsNotaryIDByIndex(QString Username, QString APIKey,
                                                QString NymID, int Index);
    QJsonValue nymRemoveOutpaymentsByIndex(QString Username, QString APIKey,
                                           QString NymID, int Index);
    QJsonValue nymVerifyOutpaymentsByIndex(QString Username, QString APIKey,
                                           QString NymID, int Index);
    QJsonValue instrumentGetAmount(QString Username, QString APIKey,
                                   QString Instrument);
    QJsonValue instrumentGetTransactionNumber(QString Username, QString APIKey,
                                              QString Instrument);
    QJsonValue instrumentGetValidFrom(QString Username, QString APIKey,
                                      QString Instrument);
    QJsonValue instrumentGetValidTo(QString Username, QString APIKey,
                                    QString Instrument);
    QJsonValue instrumentGetType(QString Username, QString APIKey,
                                 QString Instrument);
    QJsonValue instrumentGetMemo(QString Username, QString APIKey,
                                 QString Instrument);
    QJsonValue instrumentGetNotaryID(QString Username, QString APIKey,
                                     QString Instrument);
    QJsonValue instrumentGetInstrumentDefinitionID(QString Username, QString APIKey,
                                                   QString Instrument);
    QJsonValue instrumentGetRemitterNymID(QString Username, QString APIKey,
                                          QString Instrument);
    QJsonValue instrumentGetRemitterAccountID(QString Username, QString APIKey,
                                              QString Instrument);
    QJsonValue instrumentGetSenderNymID(QString Username, QString APIKey,
                                        QString Instrument);
    QJsonValue instrumentGetSenderAccountID(QString Username, QString APIKey,
                                            QString Instrument);
    QJsonValue instrumentGetRecipientNymID(QString Username, QString APIKey,
                                           QString Instrument);
    QJsonValue instrumentGetRecipientAccountID(QString Username, QString APIKey,
                                               QString Instrument);
    QJsonValue setNymName(QString Username, QString APIKey,
                          QString NymID, QString SignerNymID,
                          QString NewName);
    QJsonValue setServerName(QString Username, QString APIKey,
                             QString NotaryID, QString NewName);
    QJsonValue setAssetTypeName(QString Username, QString APIKey,
                                QString InstrumentDefinitionID, QString NewName);
    QJsonValue getNymTransactionNumberCount(QString Username, QString APIKey,
                                            QString NotaryID, QString NymID);
    QJsonValue getServerID(QString Username, QString APIKey,
                           int Index);
    QJsonValue getServerName(QString Username, QString APIKey,
                             QString ServerID);
    QJsonValue getAssetTypeID(QString Username, QString APIKey,
                              int Index);
    QJsonValue getAssetTypeName(QString Username, QString APIKey,
                                QString AssetTypeID);
    QJsonValue getAssetTypeTLA(QString Username, QString APIKey,
                               QString AssetTypeID);
    QJsonValue getAccountWalletID(QString Username, QString APIKey,
                                  int Index);
    QJsonValue getAccountWalletName(QString Username, QString APIKey,
                                    QString AccountWalletID);
    QJsonValue getAccountWalletInboxHash(QString Username, QString APIKey,
                                         QString AccountWalletID);
    QJsonValue getAccountWalletOutboxHash(QString Username, QString APIKey,
                                          QString AccountWalletID);
    QJsonValue getTime(QString Username, QString APIKey);
    QJsonValue encode(QString Username, QString APIKey,
                      QString Plaintext, bool LineBreaks);
    QJsonValue decode(QString Username, QString APIKey,
                      QString Plaintext, bool LineBreaks);
    QJsonValue encrypt(QString Username, QString APIKey,
                       QString RecipientNymID, QString Plaintext);
    QJsonValue decrypt(QString Username, QString APIKey,
                       QString RecipientNymID, QString CipherText);
    QJsonValue createSymmetricKey(QString Username, QString APIKey);
    QJsonValue symmetricEncrypt(QString Username, QString APIKey,
                                QString SymmetricKey, QString Plaintext);
    QJsonValue symmetricDecrypt(QString Username, QString APIKey,
                                QString SymmetricKey, QString CipherTextEnvelope);
    QJsonValue signContract(QString Username, QString APIKey,
                            QString SignerNymID, QString Contract);
    QJsonValue flatSign(QString Username, QString APIKey,
                        QString SignerNymID, QString Input,
                        QString ContractType);
    QJsonValue addSignature(QString Username, QString APIKey,
                            QString SignerNymID, QString Contract);
    QJsonValue verifySignature(QString Username, QString APIKey,
                               QString SignerNymID, QString Contract);
    QJsonValue verifyAndRetrieveXMLContents(QString Username, QString APIKey,
                                            QString Contract, QString SignerID);
    QJsonValue verifyAccountReceipt(QString Username, QString APIKey,
                                    QString NotaryID, QString NymID,
                                    QString AccountID);
    QJsonValue setAccountWalletName(QString Username, QString APIKey,
                                    QString AccountID, QString SignerNymID,
                                    QString AccountName);
    QJsonValue getAccountWalletBalance(QString Username, QString APIKey,
                                       QString AccountWalletID);
    QJsonValue getAccountWalletType(QString Username, QString APIKey,
                                    QString AccountWalletID);
    QJsonValue getAccountWalletInstrumentDefinitionID(QString Username, QString APIKey,
                                                      QString AccountWalletID);
    QJsonValue getAccountWalletNotaryID(QString Username, QString APIKey,
                                        QString AccountWalletID);
    QJsonValue getAccountWalletNymID(QString Username, QString APIKey,
                                     QString AccountWalletID);
    QJsonValue writeCheque(QString Username, QString APIKey,
                           QString NotaryID, qint64 ChequeAmount,
                           time64_t ValidFrom, time64_t ValidTo,
                           QString SenderAccountID, QString SenderNymID,
                           QString ChequeMemo, QString RecipientNymID);
    QJsonValue discardCheque(QString Username, QString APIKey,
                             QString NotaryID, QString NymID,
                             QString AccountID, QString Cheque);
    QJsonValue proposePaymentPlan(QString Username, QString APIKey,
                                  QString NotaryID, time64_t ValidFrom,
                                  time64_t ValidTo, QString SenderAccountID,
                                  QString SenderNymID, QString PlanConsideration,
                                  QString RecipientAccountID, QString RecipientNymID,
                                  qint64 InitialPaymentAmount, time64_t InitialPaymentDelay,
                                  qint64 PaymentPlanAmount, time64_t PaymentPlanDelay,
                                  time64_t PaymentPlanPeriod, time64_t PaymentPlanLength,
                                  int MaxPayments);
    QJsonValue easyProposePlan(QString Username, QString APIKey,
                               QString NotaryID, QString DateRange,
                               QString SenderAccountID, QString SenderNymID,
                               QString PlanConsideration, QString RecipientAccountID,
                               QString RecipientNymID, QString InitialPayment,
                               QString PaymentPlan, QString PlanExpiry);
    QJsonValue confirmPaymentPlan(QString Username, QString APIKey,
                                  QString NotaryID, QString SenderNymID,
                                  QString SenderAccountID, QString RecipientNymID,
                                  QString PaymentPlan);
    QJsonValue createSmartContract(QString Username, QString APIKey,
                                   QString SignerNymID, time64_t ValidFrom,
                                   time64_t ValidTo, bool SpecifyAssets,
                                   bool SpecifyParties);
    QJsonValue smartContractSetDates(QString Username, QString APIKey,
                                     QString Contract, QString SignerNymID,
                                     time64_t ValidFrom, time64_t ValidTo);
    QJsonValue smartArePartiesSpecified(QString Username, QString APIKey,
                                        QString Contract);
    QJsonValue smartAreAssetTypesSpecified(QString Username, QString APIKey,
                                           QString Contract);
    QJsonValue smartContractAddBylaw(QString Username, QString APIKey,
                                     QString Contract, QString SignerNymID,
                                     QString BylawName);
    QJsonValue smartContractAddClause(QString Username, QString APIKey,
                                      QString Contract, QString SignerNymID,
                                      QString BylawName, QString ClauseName,
                                      QString SourceCode);
    QJsonValue smartContractAddVariable(QString Username, QString APIKey,
                                        QString Contract, QString SignerNymID,
                                        QString BylawName, QString VarName,
                                        QString VarAccess, QString VarType,
                                        QString VarValue);
    QJsonValue smartContractAddCallback(QString Username, QString APIKey,
                                        QString Contract, QString SignerNymID,
                                        QString BylawName, QString CallbackName,
                                        QString ClauseName);
    QJsonValue smartContractAddHook(QString Username, QString APIKey,
                                    QString Contract, QString SignerNymID,
                                    QString BylawName, QString HookName,
                                    QString ClauseName);
    QJsonValue smartContractAddParty(QString Username, QString APIKey,
                                     QString Contract, QString SignerNymID,
                                     QString PartyNymID, QString PartyName,
                                     QString AgentName);
    QJsonValue smartContractAddAccount(QString Username, QString APIKey,
                                       QString Contract, QString SignerNymID,
                                      QString PartyName, QString AccountName,
                                      QString InstrumentDefinitionID);
    QJsonValue smartContractRemoveBylaw(QString Username, QString APIKey,
                                        QString Contract, QString SignerNymID,
                                        QString BylawName);
    QJsonValue smartContractUpdateClause(QString Username, QString APIKey,
                                         QString Contract, QString SignerNymID,
                                         QString BylawName, QString ClauseName,
                                         QString SourceCode);
    QJsonValue smartContractRemoveClause(QString Username, QString APIKey,
                                         QString Contract, QString SignerNymID,
                                         QString BylawName, QString ClauseName);
    QJsonValue smartContractRemoveVariable(QString Username, QString APIKey,
                                           QString Contract, QString SignerNymID,
                                           QString BylawName, QString VarName);
    QJsonValue smartContractRemoveCallback(QString Username, QString APIKey,
                                           QString Contract, QString SignerNymID,
                                           QString BylawName, QString CallbackName);
    QJsonValue smartContractRemoveHook(QString Username, QString APIKey,
                                       QString Contract, QString SignerNymID,
                                       QString BylawName, QString HookName,
                                       QString ClauseName);
    QJsonValue smartContractRemoveParty(QString Username, QString APIKey,
                                        QString Contract, QString SignerNymID,
                                        QString PartyName);
    QJsonValue smartContractCountNumbersNeeded(QString Username, QString APIKey,
                                               QString Contract, QString AgentName);
    QJsonValue smartContractConfirmAccount(QString Username, QString APIKey,
                                           QString Contract, QString SignerNymID,
                                           QString PartyName, QString AccountName,
                                           QString AgentName, QString AccountID);
    QJsonValue smartContractConfirmParty(QString Username, QString APIKey,
                                         QString Contract, QString PartyName,
                                         QString NymID);
    QJsonValue smartAreAllPartiesConfirmed(QString Username, QString APIKey,
                                           QString Contract);
    QJsonValue smartIsPartyConfirmed(QString Username, QString APIKey,
                                     QString Contract, QString PartyName);
    QJsonValue smartGetPartyCount(QString Username, QString APIKey,
                                  QString Contract);
    QJsonValue smartGetBylawCount(QString Username, QString APIKey,
                                  QString Contract);
    QJsonValue smartGetPartyByIndex(QString Username, QString APIKey,
                                    QString Contract, int Index);
    QJsonValue smartGetBylawByIndex(QString Username, QString APIKey,
                                    QString Contract, int Index);
    QJsonValue bylawGetLanguage(QString Username, QString APIKey,
                                QString Contract, QString BylawName);
    QJsonValue bylawGetClauseCount(QString Username, QString APIKey,
                                   QString Contract, QString BylawName);
    QJsonValue bylawGetVariableCount(QString Username, QString APIKey,
                                     QString Contract, QString BylawName);
    QJsonValue bylawGetHookCount(QString Username, QString APIKey,
                                 QString Contract, QString BylawName);
    QJsonValue bylawGetCallbackCount(QString Username, QString APIKey,
                                     QString Contract, QString BylawName);
    QJsonValue clauseGetNameByIndex(QString Username, QString APIKey,
                                    QString Contract, QString BylawName,
                                    int Index);
    QJsonValue clauseGetContents(QString Username, QString APIKey,
                                 QString Contract, QString BylawName,
                                 QString ClauseName);
    QJsonValue variableGetNameByIndex(QString Username, QString APIKey,
                                      QString Contract, QString BylawName,
                                      int Index);
    QJsonValue variableGetType(QString Username, QString APIKey,
                               QString Contract, QString BylawName,
                               QString VariableName);
    QJsonValue variableGetAccess(QString Username, QString APIKey,
                                 QString Contract, QString BylawName,
                                 QString VariableName);
    QJsonValue variableGetContents(QString Username, QString APIKey,
                                   QString Contract, QString BylawName,
                                   QString VariableName);
    QJsonValue hookGetNameByIndex(QString Username, QString APIKey,
                                  QString Contract, QString BylawName,
                                  int Index);
    QJsonValue hookGetClauseCount(QString Username, QString APIKey,
                                  QString Contract, QString BylawName,
                                  QString HookName);
    QJsonValue hookGetClauseAtIndex(QString Username, QString APIKey,
                                    QString Contract, QString BylawName,
                                    QString HookName, int Index);
    QJsonValue callbackGetNameByIndex(QString Username, QString APIKey,
                                      QString Contract, QString BylawName,
                                      int Index);
    QJsonValue callbackGetClause(QString Username, QString APIKey,
                                 QString Contract, QString BylawName,
                                 QString ClauseName);
    QJsonValue partyGetAccountCount(QString Username, QString APIKey,
                                    QString Contract, QString PartyName);
    QJsonValue partyGetAgentCount(QString Username, QString APIKey,
                                  QString Contract, QString PartyName);
    QJsonValue partyGetID(QString Username, QString APIKey,
                          QString Contract, QString PartyName);
    QJsonValue partyGetAccountNameByIndex(QString Username, QString APIKey,
                                          QString Contract, QString PartyName,
                                          int Index);
    QJsonValue partyGetAccountID(QString Username, QString APIKey,
                                 QString Contract, QString PartyName,
                                 QString AccountName);
    QJsonValue partyGetAccountInstrumentDefinitionID(QString Username, QString APIKey,
                                                     QString Contract, QString PartyName,
                                                     QString AccountName);
    QJsonValue partyGetAccountAgentName(QString Username, QString APIKey,
                                        QString Contract, QString PartyName,
                                        QString AccountName);
    QJsonValue partyGetAgentNameByIndex(QString Username, QString APIKey,
                                        QString Contract, QString PartyName,
                                        int Index);
    QJsonValue partyGetAgentID(QString Username, QString APIKey,
                               QString Contract, QString PartyName,
                               QString AgentName);
    QJsonValue activateSmartContract(QString Username, QString APIKey,
                                     QString NotaryID, QString NymID,
                                     QString SmartContract);
    QJsonValue triggerClause(QString Username, QString APIKey,
                             QString NotaryID, QString NymID,
                             qint64 TransactionNumber, QString ClauseName,
                             QString Parameter);
    QJsonValue messageHarvestTransactionNumbers(QString Username, QString APIKey,
                                                QString Message, QString NymID,
                                                bool HarvestingForRetry, bool ReplyWasSuccess,
                                                bool ReplyWasFailure, bool TransactionWasSuccess,
                                                bool TransactionWasFailure);
    QJsonValue loadPubkeyEncryption(QString Username, QString APIKey,
                                    QString NymID);
    QJsonValue loadPubkeySigning(QString Username, QString APIKey,
                                 QString NymID);
    QJsonValue loadUserPubkeyEncryption(QString Username, QString APIKey,
                                        QString NymID);
    QJsonValue loadUserPubkeySigning(QString Username, QString APIKey,
                                     QString NymID);
    QJsonValue verifyUserPrivateKey(QString Username, QString APIKey,
                                    QString NymID);
    QJsonValue mintIsStillGood(QString Username, QString APIKey,
                               QString NotaryID, QString InstrumentDefinitionID);
    QJsonValue loadMint(QString Username, QString APIKey,
                        QString NotaryID, QString InstrumentDefinitionID);
    QJsonValue loadUnitDefinition(QString Username, QString APIKey,
                                 QString InstrumentDefinitionID);
    QJsonValue loadServerContract(QString Username, QString APIKey,
                                  QString NotaryID);
    QJsonValue loadAssetAccount(QString Username, QString APIKey,
                                QString NotaryID, QString NymID,
                                QString AccountID);
    QJsonValue nymboxGetReplyNotice(QString Username, QString APIKey,
                                    QString NotaryID, QString NymID,
                                    qint64 RequestNumber);
    QJsonValue haveAlreadySeenReply(QString Username, QString APIKey,
                                    QString NotaryID, QString NymID,
                                    qint64 RequestNumber);
    QJsonValue loadNymbox(QString Username, QString APIKey,
                          QString NotaryID, QString NymID);
    QJsonValue loadNymboxNoVerify(QString Username, QString APIKey,
                                  QString NotaryID, QString NymID);
    QJsonValue loadInbox(QString Username, QString APIKey,
                         QString NotaryID, QString NymID,
                         QString AccountID);
    QJsonValue loadInboxNoVerify(QString Username, QString APIKey,
                                 QString NotaryID, QString NymID,
                                 QString AccountID);
    QJsonValue loadOutbox(QString Username, QString APIKey,
                          QString NotaryID, QString NymID,
                          QString AccountID);
    QJsonValue loadOutboxNoVerify(QString Username, QString APIKey,
                                  QString NotaryID, QString NymID,
                                  QString AccountID);
    QJsonValue loadPaymentInbox(QString Username, QString APIKey,
                                QString NotaryID, QString NymID);
    QJsonValue loadPaymentInboxNoVerify(QString Username, QString APIKey,
                                        QString NotaryID, QString NymID);
    QJsonValue loadRecordBox(QString Username, QString APIKey,
                             QString NotaryID, QString NymID,
                             QString AccountID);
    QJsonValue loadRecordBoxNoVerify(QString Username, QString APIKey,
                                     QString NotaryID, QString NymID,
                                     QString AccountID);
    QJsonValue loadExpiredBox(QString Username, QString APIKey,
                              QString NotaryID, QString NymID);
    QJsonValue loadExpiredBoxNoVerify(QString Username, QString APIKey,
                                      QString NotaryID, QString NymID);
    QJsonValue recordPayment(QString Username, QString APIKey,
                             QString NotaryID, QString NymID,
                             bool IsInbox, int Index,
                             bool SaveCopy);
    QJsonValue clearRecord(QString Username, QString APIKey,
                           QString NotaryID, QString NymID,
                           QString AccountID, int Index,
                           bool ClearAll);
    QJsonValue clearExpired(QString Username, QString APIKey,
                            QString NotaryID, QString NymID,
                            int Index, bool ClearAll);
    QJsonValue ledgerGetCount(QString Username, QString APIKey,
                              QString NotaryID, QString NymID,
                              QString AccountID, QString Ledger);
    QJsonValue ledgerCreateResponse(QString Username, QString APIKey,
                                    QString NotaryID, QString NymID,
                                    QString AccountID, QString OriginalLedger);
    QJsonValue ledgerGetTransactionByIndex(QString Username, QString APIKey,
                                           QString NotaryID, QString NymID,
                                           QString AccountID, QString Ledger,
                                           int Index);
    QJsonValue ledgerGetTransactionByID(QString Username, QString APIKey,
                                        QString NotaryID, QString NymID,
                                        QString AccountID, QString Ledger,
                                        qint64 TransactionNumber);
    QJsonValue ledgerGetInstrument(QString Username, QString APIKey,
                                   QString NotaryID, QString NymID,
                                   QString AccountID, QString Ledger,
                                   int Index);
    QJsonValue ledgerGetTransactionIDByIndex(QString Username, QString APIKey,
                                             QString NotaryID, QString NymID,
                                             QString AccountID, QString Ledger,
                                             int Index);
    QJsonValue ledgerAddTransaction(QString Username, QString APIKey,
                                    QString NotaryID, QString NymID,
                                    QString AccountID, QString Ledger,
                                    QString Transaction);
    QJsonValue transactionCreateResponse(QString Username, QString APIKey,
                                         QString NotaryID, QString NymID,
                                         QString AccountID, QString Ledger,
                                         QString Transaction, bool DoIAccept);
    QJsonValue ledgerFinalizeResponse(QString Username, QString APIKey,
                                      QString NotaryID, QString NymID,
                                      QString AccountID, QString Ledger);
    QJsonValue transactionGetVoucher(QString Username, QString APIKey,
                                     QString NotaryID, QString NymID,
                                     QString AccountID, QString Transaction);
    QJsonValue transactionGetSenderNymID(QString Username, QString APIKey,
                                         QString NotaryID, QString NymID,
                                         QString AccountID, QString Transaction);
    QJsonValue transactionGetRecipientNymID(QString Username, QString APIKey,
                                            QString NotaryID, QString NymID,
                                            QString AccountID, QString Transaction);
    QJsonValue transactionGetSenderAccountID(QString Username, QString APIKey,
                                             QString NotaryID, QString NymID,
                                             QString AccountID, QString Transaction);
    QJsonValue transactionGetRecipientAccountID(QString Username, QString APIKey,
                                                QString NotaryID, QString NymID,
                                                QString AccountID, QString Transaction);
    QJsonValue pendingGetNote(QString Username, QString APIKey,
                              QString NotaryID, QString NymID,
                              QString AccountID, QString Transaction);
    QJsonValue transactionGetAmount(QString Username, QString APIKey,
                                    QString NotaryID, QString NymID,
                                    QString AccountID, QString Transaction);
    QJsonValue transactionGetDisplayReferenceToNumber(QString Username, QString APIKey,
                                                      QString NotaryID, QString NymID,
                                                      QString AccountID, QString Transaction);
    QJsonValue transactionGetType(QString Username, QString APIKey,
                                  QString NotaryID, QString NymID,
                                  QString AccountID, QString Transaction);
    QJsonValue replyNoticeGetRequestNumber(QString Username, QString APIKey,
                                           QString NotaryID, QString NymID,
                                           QString Transaction);
    QJsonValue transactionGetDateSigned(QString Username, QString APIKey,
                                        QString NotaryID, QString NymID,
                                        QString AccountID, QString Transaction);
    QJsonValue transactionGetSuccess(QString Username, QString APIKey,
                                     QString NotaryID, QString NymID,
                                     QString AccountID, QString Transaction);
    QJsonValue transactionIsCanceled(QString Username, QString APIKey,
                                     QString NotaryID, QString NymID,
                                     QString AccountID, QString Transaction);
    QJsonValue transactionGetBalanceAgreementSuccess(QString Username, QString APIKey,
                                                     QString NotaryID, QString NymID,
                                                     QString AccountID, QString Transaction);
    QJsonValue messageGetBalanceAgreementSuccess(QString Username, QString APIKey,
                                                 QString NotaryID, QString NymID,
                                                 QString AccountID, QString Message);
    QJsonValue savePurse(QString Username, QString APIKey,
                         QString NotaryID, QString InstrumentDefinitionID,
                         QString NymID, QString Purse);
    QJsonValue loadPurse(QString Username, QString APIKey,
                         QString NotaryID, QString InstrumentDefinitionID,
                         QString NymID);
    QJsonValue purseGetTotalValue(QString Username, QString APIKey,
                                  QString NotaryID, QString InstrumentDefinitionID,
                                  QString Purse);
    QJsonValue purseCount(QString Username, QString APIKey,
                          QString NotaryID, QString InstrumentDefinitionID,
                          QString Purse);
    QJsonValue purseHasPassword(QString Username, QString APIKey,
                                QString NotaryID, QString Purse);
    QJsonValue createPurse(QString Username, QString APIKey,
                           QString NotaryID, QString InstrumentDefinitionID,
                           QString OwnerID, QString SignerID);
    QJsonValue createPursePassphrase(QString Username, QString APIKey,
                                     QString NotaryID, QString InstrumentDefinitionID,
                                     QString SignerID);
    QJsonValue pursePeek(QString Username, QString APIKey,
                         QString NotaryID, QString InstrumentDefinitionID,
                         QString OwnerID, QString Purse);
    QJsonValue pursePop(QString Username, QString APIKey,
                        QString NotaryID, QString InstrumentDefinitionID,
                        QString OwnerOrSignerID, QString Purse);
    QJsonValue purseEmpty(QString Username, QString APIKey,
                          QString NotaryID, QString InstrumentDefinitionID,
                          QString SignerID, QString Purse);
    QJsonValue pursePush(QString Username, QString APIKey,
                         QString NotaryID, QString InstrumentDefinitionID,
                         QString SignerID, QString OwnerID,
                         QString Purse, QString Token);
    QJsonValue walletImportPurse(QString Username, QString APIKey,
                                 QString NotaryID, QString InstrumentDefinitionID,
                                 QString NymID, QString Purse);
    QJsonValue exchangePurse(QString Username, QString APIKey,
                             QString NotaryID, QString InstrumentDefinitionID,
                             QString NymID, QString Purse);
    QJsonValue tokenChangeOwner(QString Username, QString APIKey,
                                QString NotaryID, QString InstrumentDefinitionID,
                                QString Token, QString SignerNymID,
                                QString OldOwner, QString NewOwner);
    QJsonValue tokenGetID(QString Username, QString APIKey,
                          QString NotaryID, QString InstrumentDefinitionID,
                          QString Token);
    QJsonValue tokenGetDenomination(QString Username, QString APIKey,
                                    QString NotaryID, QString InstrumentDefinitionID,
                                    QString Token);
    QJsonValue tokenGetSeries(QString Username, QString APIKey,
                              QString NotaryID, QString InstrumentDefinitionID,
                              QString Token);
    QJsonValue tokenGetValidFrom(QString Username, QString APIKey,
                                 QString NotaryID, QString InstrumentDefinitionID,
                                 QString Token);
    QJsonValue tokenGetValidTo(QString Username, QString APIKey,
                               QString NotaryID, QString InstrumentDefinitionID,
                               QString Token);
    QJsonValue tokenGetInstrumentDefinitionID(QString Username, QString APIKey,
                                              QString Token);
    QJsonValue tokenGetNotaryID(QString Username, QString APIKey,
                                QString Token);
    QJsonValue isBasketCurrency(QString Username, QString APIKey,
                                QString InstrumentDefinitionID);
    QJsonValue basketGetMemberCount(QString Username, QString APIKey,
                                    QString InstrumentDefinitionID);
    QJsonValue basketGetMemberType(QString Username, QString APIKey,
                                   QString BasketInstrumentDefinitionID, int Index);
    QJsonValue basketGetMinimumTransferAmount(QString Username, QString APIKey,
                                              QString BasketInstrumentDefinitionID);
    QJsonValue basketGetMemberMinimumTransferAmount(QString Username, QString APIKey,
                                                    QString BasketInstrumentDefinitionID, int Index);
    QJsonValue pingNotary(QString Username, QString APIKey,
                          QString NotaryID, QString NymID);
    QJsonValue registerNym(QString Username, QString APIKey,
                           QString NotaryID, QString NymID);
    QJsonValue unregisterNym(QString Username, QString APIKey,
                             QString NotaryID, QString NymID);
    QJsonValue messageGetUsageCredits(QString Username, QString APIKey,
                                      QString Message);
    QJsonValue usageCredits(QString Username, QString APIKey,
                            QString NotaryID, QString NymID,
                            QString NymIDCheck, qint64 Adjustment);
    QJsonValue checkNym(QString Username, QString APIKey,
                        QString NotaryID, QString NymID,
                        QString NymIDCheck);
    QJsonValue sendNymMessage(QString Username, QString APIKey,
                              QString NotaryID, QString NymID,
                              QString NymIDRecipient,
                              QString Message);
    QJsonValue sendNymInstrument(QString Username, QString APIKey,
                                 QString NotaryID, QString NymID,
                                 QString NymIDRecipient,
                                 QString Instrument, QString InstrumentForSender);
    QJsonValue getRequestNumber(QString Username, QString APIKey,
                                QString NotaryID, QString NymID);
    QJsonValue registerInstrumentDefinition(QString Username, QString APIKey,
                                            QString NotaryID, QString NymID,
                                            QString Contract);
    QJsonValue getInstrumentDefinition(QString Username, QString APIKey,
                                       QString NotaryID, QString NymID,
                                       QString InstrumentDefinitionID);
    QJsonValue getMint(QString Username, QString APIKey,
                       QString NotaryID, QString NymID,
                       QString InstrumentDefinitionID);
    QJsonValue registerAccount(QString Username, QString APIKey,
                               QString NotaryID, QString NymID,
                               QString InstrumentDefinitionID);
    QJsonValue getAccountData(QString Username, QString APIKey,
                              QString NotaryID, QString NymID,
                              QString AccountID);
    QJsonValue generateBasketCreation(QString Username, QString APIKey,
                                      QString NymID, QString Shortname,
                                      QString Name, QString Symbol,
                                      QString Terms, qint64 Weight);
    QJsonValue addBasketCreationItem(QString Username, QString APIKey,
                                     QString Basket, QString InstrumentDefinitionID,
                                     qint64 MinimumTransfer);
    QJsonValue issueBasket(QString Username, QString APIKey,
                           QString NotaryID, QString NymID,
                           QString Basket);
    QJsonValue generateBasketExchange(QString Username, QString APIKey,
                                      QString NotaryID, QString NymID,
                                      QString BasketInstrumentDefinitionID, QString BasketAssetAccountID,
                                      int TransferMultiple);
    QJsonValue addBasketExchangeItem(QString Username, QString APIKey,
                                     QString NotaryID, QString NymID,
                                     QString Basket, QString InstrumentDefinitionID,
                                     QString AssetAccountID);
    QJsonValue exchangeBasket(QString Username, QString APIKey,
                              QString NotaryID, QString NymID,
                              QString BasketInstrumentDefinitionID, QString Basket,
                              bool ExchangeDirection);
    QJsonValue getTransactionNumbers(QString Username, QString APIKey,
                                     QString NotaryID, QString NymID);
    QJsonValue notarizeWithdrawal(QString Username, QString APIKey,
                                  QString NotaryID, QString NymID,
                                  QString AccountID, qint64 Amount);
    QJsonValue notarizeDeposit(QString Username, QString APIKey,
                               QString NotaryID, QString NymID,
                               QString AccountID, QString Purse);
    QJsonValue notarizeTransfer(QString Username, QString APIKey,
                                QString NotaryID, QString NymID,
                                QString AccountFrom, QString AccountTo,
                                qint64 Amount, QString Note);
    QJsonValue getNymbox(QString Username, QString APIKey,
                         QString NotaryID, QString NymID);
    QJsonValue processInbox(QString Username, QString APIKey,
                            QString NotaryID, QString NymID,
                            QString AccountID, QString AccountLedger);
    QJsonValue processNymbox(QString Username, QString APIKey,
                             QString NotaryID, QString NymID);
    QJsonValue withdrawVoucher(QString Username, QString APIKey,
                               QString NotaryID, QString NymID,
                               QString AccountID, QString RecipientNymID,
                               QString ChequeMemo, qint64 Amount);
    QJsonValue payDividend(QString Username, QString APIKey,
                           QString NotaryID, QString IssuerNymID,
                           QString DividendFromAccountID, QString SharesInstrumentDefinitionID,
                           QString DividendMemo, qint64 AmountPerShare);
    QJsonValue depositCheque(QString Username, QString APIKey,
                             QString NotaryID, QString NymID,
                             QString AccountID, QString Cheque);
    QJsonValue depositPaymentPlan(QString Username, QString APIKey,
                                  QString NotaryID, QString NymID,
                                  QString PaymentPlan);
    QJsonValue killMarketOffer(QString Username, QString APIKey,
                               QString NotaryID, QString NymID,
                               QString AssetAccountID, qint64 TransactionNumber);
    QJsonValue killPaymentPlan(QString Username, QString APIKey,
                               QString NotaryID, QString NymID,
                               QString FromAccountID, qint64 TransactionNumber);
    QJsonValue issueMarketOffer(QString Username, QString APIKey,
                                QString AssetAccountID, QString CurrencyAccountID,
                                qint64 MarketScale, qint64 MinimumIncrement,
                                qint64 TotalAssetsOnOffer, qint64 PriceLimit,
                                bool BuyingOrSelling, time64_t LifeSpanInSeconds,
                                QString StopSign, qint64 ActivationPrice);
    QJsonValue getMarketList(QString Username, QString APIKey,
                             QString NotaryID, QString NymID);
    QJsonValue getMarketOffers(QString Username, QString APIKey,
                               QString NotaryID, QString NymID,
                               QString MarketID, qint64 MaxDepth);
    QJsonValue getMarketRecentTrades(QString Username, QString APIKey,
                                     QString NotaryID, QString NymID,
                                     QString MarketID);
    QJsonValue getNymMarketOffers(QString Username, QString APIKey,
                                  QString NotaryID, QString NymID);
    QJsonValue popMessageBuffer(QString Username, QString APIKey,
                                qint64 RequestNumber, QString NotaryID,
                                QString NymID);
    QJsonValue flushMessageBuffer(QString Username, QString APIKey);
    QJsonValue getSentMessage(QString Username, QString APIKey,
                              qint64 RequestNumber, QString NotaryID,
                              QString NymID);
    QJsonValue removeSentMessage(QString Username, QString APIKey,
                                 qint64 RequestNumber, QString NotaryID,
                                 QString NymID);
    QJsonValue flushSentMessages(QString Username, QString APIKey,
                                 bool HarvestingForRetry, QString NotaryID,
                                 QString NymID, QString NymBox);
    QJsonValue sleep(QString Username, QString APIKey,
                     qint64 Milliseconds);
    QJsonValue resyncNymWithServer(QString Username, QString APIKey,
                                   QString NotaryID, QString NymID,
                                   QString Message);
    QJsonValue queryInstrumentDefinitions(QString Username, QString APIKey,
                                          QString NotaryID, QString NymID,
                                          QString EncodedMap);
    QJsonValue messageGetPayload(QString Username, QString APIKey,
                                 QString Message);
    QJsonValue messageGetCommand(QString Username, QString APIKey,
                                 QString Message);
    QJsonValue messageGetLedger(QString Username, QString APIKey,
                                QString Message);
    QJsonValue messageGetNewInstrumentDefinitionID(QString Username, QString APIKey,
                                                   QString Message);
    QJsonValue messageGetNewIssuerAccountID(QString Username, QString APIKey,
                                            QString Message);
    QJsonValue messageGetNewAccountID(QString Username, QString APIKey,
                                      QString Message);
    QJsonValue messageGetNymboxHash(QString Username, QString APIKey,
                                    QString Message);
    QJsonValue messageGetSuccess(QString Username, QString APIKey,
                                 QString Message);
    QJsonValue messageGetDepth(QString Username, QString APIKey,
                               QString Message);
    QJsonValue messageIsTransactionCanceled(QString Username, QString APIKey,
                                            QString NotaryID, QString NymID,
                                            QString AccountID, QString Message);
    QJsonValue messageGetTransactionSuccess(QString Username, QString APIKey,
                                            QString NotaryID, QString NymID,
                                            QString AccountID, QString Message);



    // Moneychanger::It() Dialog methods
    QString mcSendDialog(QString Username, QString APIKey,
                         QString Account, QString Recipient,
                         QString Asset, QString Amount);
    QString mcRequestFundsDialog(QString Username, QString APIKey,
                                 QString Account, QString Recipient,
                                 QString Asset, QString Amount);
    QString mcMessagesDialog(QString Username, QString APIKey);
    QString mcExchangeDialog(QString Username, QString APIKey);
    QString mcPaymentsDialog(QString Username, QString APIKey);
    QString mcManageAccountsDialog(QString Username, QString APIKey);
    QString mcManageNymsDialog(QString Username, QString APIKey);
    QString mcManageAssetsDialog(QString Username, QString APIKey);
    QString mcManageSmartContractsDialog(QString Username, QString APIKey);


    bool mcActivateSmartContract(QString Username, QString APIKey); // Init Wizard
    QJsonValue mcListSmartContracts(QString Username, QString APIKey);


    // API Key Methods
    QJsonValue userLogin(QString Username, QString PlaintextPassword);
    QJsonValue userLogout(QString Username, QString PlaintextPassword);
    QJsonValue refreshAPIKey(QString Username, QString PlaintextPassword);


    // RecordList Methods
    QJsonValue recordListPopulate(QString Username, QString APIKey);
    QJsonValue recordListCount(QString Username, QString APIKey);
    QJsonValue recordListRetrieve(QString Username, QString APIKey,
                                  int Index);
    QJsonValue recordListRetrieve(QString Username, QString APIKey,
                                  int BeginIndex, int EndIndex);

    QJsonValue setDefaultNym(QString Username, QString APIKey,
                             QString nym_id, QString nym_name);
    QJsonValue getDefaultNym(QString Username, QString APIKey);

    QJsonValue setDefaultAccount(QString Username, QString APIKey,
                                 QString AccountID, QString AccountName);
    QJsonValue getDefaultAccount(QString Username, QString APIKey);

    QJsonValue setDefaultServer(QString Username, QString APIKey,
                                QString NotaryID, QString ServerName);
    QJsonValue getDefaultServer(QString Username, QString APIKey);

    QJsonValue setDefaultAsset(QString Username, QString APIKey,
                               QString AssetID, QString AssetName);
    QJsonValue getDefaultAsset(QString Username, QString APIKey);

    QJsonValue isValidID(QString ID, QString Username, QString APIKey);


private:

    // RecordList Methods
    opentxs::OTRecordList * m_RecordList=nullptr;
    bool createRecordList(QString Username, QString APIKey);

    RPCUserManager m_userManager;

    bool validateAPIKey(QString Username, QString APIKey);

};

#endif // MCRPCSERVICE_H
