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
    QJsonValue createNym(QString Username, QString APIKey,
                         int KeySize, QString NymIDSource,
                         QString AltLocation);
    QJsonValue getNymActiveCronItemIDs(QString Username, QString APIKey,
                                       QString NymID, QString NotaryID);
    QJsonValue getActiveCronItem(QString Username, QString APIKey,
                                 QString NotaryID, qint64 TransNum);
    QJsonValue getNymSourceForID(QString Username, QString APIKey,
                                 QString NymID);
    QJsonValue getNymAltSourceLocation(QString Username, QString APIKey,
                                       QString NymID);
    QJsonValue getNymCredentialCount(QString Username, QString APIKey,
                                     QString NymID);
    QJsonValue getNymCredentialID(QString Username, QString APIKey,
                                  QString NymID, int Index);
    QJsonValue getNymCredentialContents(QString Username, QString APIKey,
                                        QString NymID, QString CredentialID);
    QJsonValue getNymRevokedCount(QString Username, QString APIKey,
                                  QString NymID);
    QJsonValue getNymRevokedCredID(QString Username, QString APIKey,
                                   QString NymID, int Index);
    QJsonValue getNymRevokedCredContents(QString Username, QString APIKey,
                                         QString NymID, QString CredentialID);
    QJsonValue getNymSubCredentialCount(QString Username, QString APIKey,
                                        QString NymID, QString MasterCredID);
    QJsonValue getNymSubCredentialID(QString Username, QString APIKey,
                                     QString NymID, QString MasterCredID,
                                     int Index);
    QJsonValue getNymSubCredentialContents(QString Username, QString APIKey,
                                           QString NymID, QString MasterCredID,
                                           QString SubCredID);
    QJsonValue addSubCredential(QString Username, QString APIKey,
                                QString NymID, QString MasterCredID,
                                int KeySize);
    QJsonValue revokeSubcredential(QString Username, QString APIKey,
                                   QString NymID, QString MasterCredID,
                                   QString SubCredID);
    QJsonValue getSignerNymID(QString Username, QString APIKey,
                              QString Contract);
    QJsonValue calculateAssetContractID(QString Username, QString APIKey,
                                        QString Contract);
    QJsonValue calculateServerContractID(QString Username, QString APIKey,
                                         QString Contract);
    QJsonValue calculateContractID(QString Username, QString APIKey,
                                   QString Contract);
    QJsonValue createServerContract(QString Username, QString APIKey,
                                    QString NymID, QString XMLContents);
    QJsonValue createAssetContract(QString Username, QString APIKey,
                                   QString NymID, QString XMLContents);
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
    QJsonValue addAssetContract(QString Contract);
    QJsonValue getNymCount();
    QJsonValue getServerCount();
    QJsonValue getAssetTypeCount();
    QJsonValue getAccountCount();
    QJsonValue walletCanRemoveServer(QString NotaryID);
    QJsonValue walletRemoveServer(QString NotaryID);
    QJsonValue walletCanRemoveAssetType(QString InstrumentDefinitionID);
    QJsonValue walletRemoveAssetType(QString InstrumentDefinitionID);
    QJsonValue walletCanRemoveNym(QString NymID);
    QJsonValue walletRemoveNym(QString NymID);
    QJsonValue walletCanRemoveAccount(QString AccountID);
    QJsonValue doesBoxReceiptExist(QString NotaryID, QString NymID,
                                   QString AccountID, int BoxType,
                                   qint64 TransactionNumber);
    QJsonValue getBoxReceipt(QString NotaryID, QString NymID,
                                   QString AccountID, int BoxType,
                                   qint64 TransactionNumber);
    QJsonValue deleteAssetAccount(QString NotaryID, QString NymID, QString AccountID);
    QJsonValue walletExportNym(QString NymID);
    QJsonValue walletExportCert(QString NymID);
    QJsonValue walletImportNym(QString FileContents);
    QJsonValue walletImportCert(QString DisplayName, QString FileContents);
// QJsonValue walletChangePassphrase(void);
    QJsonValue walletGetNymIdFromPartial(QString PartialID);
    QJsonValue walletGetNotaryIdFromPartial(QString PartialID);
    QJsonValue walletGetInstrumentDefinitionIdFromPartial(QString PartialID);
    QJsonValue walletGetAccountIdFromPartial(QString PartialID);
    QJsonValue getNymID(int Index);
    QJsonValue getNymName(QString NymID);
    QJsonValue isNymRegisteredAtServer(QString NymID, QString NotaryID);
    QJsonValue getNymStats(QString NymID);
    QJsonValue getNymNymboxHash(QString NotaryID, QString NymID);
    QJsonValue getNymRecentHash(QString NotaryID, QString NymID);
    QJsonValue getNymInboxHash(QString NotaryID, QString NymID);
    QJsonValue getNymOutboxHash(QString NotaryID, QString NymID);
    QJsonValue getNymMailCount(QString NymID);
    QJsonValue getNymContentsByIndex(QString NymID, int Index);
    QJsonValue getNymMailSenderIDByIndex(QString NymID, int Index);
    QJsonValue getNymMailNotaryIDByIndex(QString NymID, int Index);
    QJsonValue nymRemoveMailByIndex(QString NymID, int Index);
    QJsonValue nymVerifyMailByIndex(QString NymID, int Index);
    QJsonValue getNymOutmailCount(QString NymID);
    QJsonValue getNymOutmailContentsByIndex(QString NymID, int Index);
    QJsonValue getNymOutmailRecipientIDByIndex(QString NymID, int Index);
    QJsonValue getNymOutmailNotaryIDByIndex(QString NymID, int Index);
    QJsonValue nymRemoveOutmailByIndex(QString NymID, int Index);
    QJsonValue nymVerifyOutmailByIndex(QString NymID, int Index);
    QJsonValue getNymOutpaymentsCount(QString NymID);
    QJsonValue getNymOutpaymentsContentsByIndex(QString NymID, int Index);
    QJsonValue getNymOutpaymentsRecipientIDByIndex(QString NymID, int Index);
    QJsonValue getNymOutpaymentsNotaryIDByIndex(QString NymID, int Index);
    QJsonValue nymRemoveOutpaymentsByIndex(QString NymID, int Index);
    QJsonValue nymVerifyOutpaymentsByIndex(QString NymID, int Index);
    QJsonValue instrumentGetAmount(QString Instrument);
    QJsonValue instrumentGetTransactionNumber(QString Instrument);
    QJsonValue instrumentGetValidFrom(QString Instrument);
    QJsonValue instrumentGetValidTo(QString Instrument);
    QJsonValue instrumentGetType(QString Instrument);
    QJsonValue instrumentGetMemo(QString Instrument);
    QJsonValue instrumentGetNotaryID(QString Instrument);
    QJsonValue instrumentGetInstrumentDefinitionID(QString Instrument);
    QJsonValue instrumentGetRemitterNymID(QString Instrument);
    QJsonValue instrumentGetRemitterAccountID(QString Instrument);
    QJsonValue instrumentGetSenderNymID(QString Instrument);
    QJsonValue instrumentGetSenderAccountID(QString Instrument);
    QJsonValue instrumentGetRecipientNymID(QString Instrument);
    QJsonValue instrumentGetRecipientAccountID(QString Instrument);
    QJsonValue setNymName(QString NymID, QString SignerNymID,
                          QString NewName);
    QJsonValue setServerName(QString NotaryID, QString NewName);
    QJsonValue setAssetTypeName(QString InstrumentDefinitionID, QString NewName);
    QJsonValue getNymTransactionNumberCount(QString NotaryID, QString NymID);
    QJsonValue getServerID(int Index);
    QJsonValue getServerName(QString ServerID);
    QJsonValue getAssetTypeID(int Index);
    QJsonValue getAssetTypeName(QString AssetTypeID);
    QJsonValue getAssetTypeTLA(QString AssetTypeID);
    QJsonValue getAccountWalletID(int Index);
    QJsonValue getAccountWalletName(QString AccountWalletID);
    QJsonValue getAccountWalletInboxHash(QString AccountWalletID);
    QJsonValue getAccountWalletOutboxHash(QString AccountWalletID);
    QJsonValue getTime(void);
    QJsonValue encode(QString Plaintext, bool LineBreaks);
    QJsonValue decode(QString Plaintext, bool LineBreaks);
    QJsonValue encrypt(QString RecipientNymID, QString Plaintext);
    QJsonValue decrypt(QString RecipientNymID, QString CipherText);
    QJsonValue createSymmetricKey(void);
    QJsonValue symmetricEncrypt(QString SymmetricKey, QString Plaintext);
    QJsonValue symmetricDecrypt(QString SymmetricKey, QString CipherTextEnvelope);
    QJsonValue signContract(QString SignerNymID, QString Contract);
    QJsonValue flatSign(QString SignerNymID, QString Input,
                        QString ContractType);
    QJsonValue addSignature(QString SignerNymID, QString Contract);
    QJsonValue verifySignature(QString SignerNymID, QString Contract);
    QJsonValue verifyAndRetrieveXMLContents(QString Contract, QString SignerID);
    QJsonValue verifyAccountReceipt(QString NotaryID, QString NymID,
                                    QString AccountID);
    QJsonValue setAccountWalletName(QString AccountID, QString SignerNymID,
                                    QString AccountName);
    QJsonValue getAccountWalletBalance(QString AccountWalletID);
    QJsonValue getAccountWalletType(QString AccountWalletID);
    QJsonValue getAccountWalletInstrumentDefinitionID(QString AccountWalletID);
    QJsonValue getAccountWalletNotaryID(QString AccountWalletID);
    QJsonValue getAccountWalletNymID(QString AccountWalletID);
    QJsonValue writeCheque(QString NotaryID, qint64 ChequeAmount,
                           time64_t ValidFrom, time64_t ValidTo,
                           QString SenderAccountID, QString SenderNymID,
                           QString ChequeMemo, QString RecipientNymID);
    QJsonValue discardCheque(QString NotaryId, QString NymID,
                             QString AccountID, QString Cheque);
    QJsonValue proposePaymentPlan(QString NotaryID, time64_t ValidFrom,
                                  time64_t ValidTo, QString SenderAccountID,
                                  QString SenderNymID, QString PlanConsideration,
                                  QString RecipientAccountID, QString RecipientNymID,
                                  qint64 InitialPaymentAmount, time64_t InitialPaymentDelay,
                                  qint64 PaymentPlanAmount, time64_t PaymentPlanDelay,
                                  time64_t PaymentPlanPeriod, time64_t PaymentPlanLength,
                                  int MaxPayments);
    QJsonValue easyProposePlan(QString NotaryID, QString DateRange,
                               QString SenderAccountID, QString SenderNymID,
                               QString PlanConsideration, QString RecipientAccountID,
                               QString RecipientNymID, QString InitialPayment,
                               QString PaymentPlan, QString PlanExpiry);
    QJsonValue confirmPaymentPlan(QString NotaryID, QString SenderNymID,
                                  QString SenderAccountID, QString RecipientNymID,
                                  QString PaymentPlan);
    QJsonValue createSmartContract(QString SignerNymID, time64_t ValidFrom,
                                   time64_t ValidTo, bool SpecifyAssets,
                                   bool SpecifyParties);
    QJsonValue smartContractSetDates(QString Contract, QString SignerNymID,
                                     time64_t ValidFrom, time64_t ValidTo);
    QJsonValue smartArePartiesSpecified(QString Contract);
    QJsonValue smartAreAssetTypesSpecified(QString Contract);
    QJsonValue smartContractAddBylaw(QString Contract, QString SignerNymID,
                                     QString BylawName);
    QJsonValue smartContractAddClause(QString Contract, QString SignerNymID,
                                      QString BylawName, QString ClauseName,
                                      QString SourceCode);
    QJsonValue smartContractAddVariable(QString Contract, QString SignerNymID,
                                        QString BylawName, QString VarName,
                                        QString VarAccess, QString VarType,
                                        QString VarValue);
    QJsonValue smartContractAddCallback(QString Contract, QString SignerNymID,
                                        QString BylawName, QString CallbackName,
                                        QString ClauseName);
    QJsonValue smartContractAddHook(QString Contract, QString SignerNymID,
                                    QString BylawName, QString HookName,
                                    QString ClauseName);
    QJsonValue smartContractAddParty(QString Contract, QString SignerNymID,
                                     QString PartyNymID, QString PartyName,
                                     QString AgentName);
    QJsonValue smartContractAddAccount(QString Contract, QString SignerNymID,
                                      QString PartyName, QString AccountName,
                                      QString InstrumentDefinitionID);
    QJsonValue smartContractRemoveBylaw(QString Contract, QString SignerNymID,
                                        QString BylawName);
    QJsonValue smartContractUpdateClause(QString Contract, QString SignerNymID,
                                         QString BylawName, QString ClauseName,
                                         QString SourceCode);
    QJsonValue smartContractRemoveClause(QString Contract, QString SignerNymID,
                                         QString BylawName, QString ClauseName);
    QJsonValue smartContractRemoveVariable(QString Contract, QString SignerNymID,
                                           QString BylawName, QString VarName);
    QJsonValue smartContractRemoveCallback(QString Contract, QString SignerNymID,
                                           QString BylawName, QString CallbackName);
    QJsonValue smartContractRemoveHook(QString Contract, QString SignerNymID,
                                       QString BylawName, QString HookName,
                                       QString ClauseName);
    QJsonValue smartContractRemoveParty(QString Contract, QString SignerNymID,
                                        QString PartyName);
    QJsonValue smartContractCountNumbersNeeded(QString Contract, QString AgentName);
    QJsonValue smartContractConfirmAccount(QString Contract, QString SignerNymID,
                                           QString PartyName, QString AccountName,
                                           QString AgentName, QString AccountID);
    QJsonValue smartContractConfirmParty(QString Contract, QString PartyName,
                                         QString NymID);
    QJsonValue smartAreAllPartiesConfirmed(QString Contract);
    QJsonValue smartIsPartyConfirmed(QString Contract, QString PartyName);
    QJsonValue smartGetPartyCount(QString Contract);
    QJsonValue smartGetBylawCount(QString Contract);
    QJsonValue smartGetPartyByIndex(QString Contract, int Index);
    QJsonValue smartGetBylawByIndex(QString Contract, int Index);
    QJsonValue bylawGetLanguage(QString Contract, QString BylawName);
    QJsonValue bylawGetClauseCount(QString Contract, QString BylawName);
    QJsonValue bylawGetVariableCount(QString Contract, QString BylawName);
    QJsonValue bylawGetHookCount(QString Contract, QString BylawName);
    QJsonValue bylawGetCallbackCount(QString Contract, QString BylawName);
    QJsonValue clauseGetNameByIndex(QString Contract, QString BylawName,
                                    int Index);
    QJsonValue clauseGetContents(QString Contract, QString BylawName,
                                 QString ClauseName);
    QJsonValue variableGetNameByIndex(QString Contract, QString BylawName,
                                      int Index);
    QJsonValue variableGetType(QString Contract, QString BylawName,
                               QString VariableName);
    QJsonValue variableGetAccess(QString Contract, QString BylawName,
                                 QString VariableName);
    QJsonValue variableGetContents(QString Contract, QString BylawName,
                                   QString VariableName);
    QJsonValue hookGetNameByIndex(QString Contract, QString BylawName,
                                  int Index);
    QJsonValue hookGetClauseCount(QString Contract, QString BylawName,
                                  QString HookName);
    QJsonValue hookGetClauseAtIndex(QString Contract, QString BylawName,
                                    QString HookName, int Index);
    QJsonValue callbackGetNameByIndex(QString Contract, QString BylawName,
                                      int Index);
    QJsonValue callbackGetClause(QString Contract, QString BylawName,
                                 QString ClauseName);
    QJsonValue partyGetAccountCount(QString Contract, QString PartyName);
    QJsonValue partyGetAgentCount(QString Contract, QString PartyName);
    QJsonValue partyGetID(QString Contract, QString PartyName);
    QJsonValue partyGetAccountNameByIndex(QString Contract, QString PartyName,
                                          int Index);
    QJsonValue partyGetAccountID(QString Contract, QString PartyName,
                                 QString AccountName);
    QJsonValue partyGetAccountInstrumentDefinitionID(QString Contract, QString PartyName,
                                                     QString AccountName);
    QJsonValue partyGetAccountAgentName(QString Contract, QString PartyName,
                                        QString AccountName);
    QJsonValue partyGetAgentNameByIndex(QString Contract, QString PartyName,
                                        int Index);
    QJsonValue partyGetAgentID(QString Contract, QString PartyName,
                               QString AgentName);
    QJsonValue activateSmartContract(QString NotaryID, QString NymID,
                                     QString SmartContract);
    QJsonValue triggerClause(QString NotaryID, QString NymID,
                             qint64 TransactionNumber, QString ClauseName,
                             QString Parameter);
    QJsonValue messageHarvestTransactionNumbers(QString Message, QString NymID,
                                                bool HarvestingForRetry, bool ReplyWasSuccess,
                                                bool ReplyWasFailure, bool TransactionWasSuccess,
                                                bool TransactionWasFailure);
    QJsonValue loadPubkeyEncryption(QString NymID);
    QJsonValue loadPubkeySigning(QString NymID);
    QJsonValue loadUserPubkeyEncryption(QString NymID);
    QJsonValue loadUserPubkeySigning(QString NymID);
    QJsonValue verifyUserPrivateKey(QString NymID);
    QJsonValue mintIsStillGood(QString NotaryID, QString InstrumentDefinitionID);
    QJsonValue loadMint(QString NotaryID, QString InstrumentDefinitionID);
    QJsonValue loadAssetContract(QString InstrumentDefinitionID);
    QJsonValue loadServerContract(QString NotaryID);
    QJsonValue loadAssetAccount(QString NotaryID, QString NymID,
                                QString AccountID);
    QJsonValue nymboxGetReplyNotice(QString NotaryID, QString NymID,
                                    qint64 RequestNumber);
    QJsonValue haveAlreadySeenReply(QString NotaryID, QString NymID,
                                    qint64 RequestNumber);
    QJsonValue loadNymbox(QString NotaryID, QString NymID);
    QJsonValue loadNymboxNoVerify(QString NotaryID, QString NymID);
    QJsonValue loadInbox(QString NotaryID, QString NymID,
                         QString AccountID);
    QJsonValue loadInboxNoVerify(QString NotaryID, QString NymID,
                                 QString AccountID);
    QJsonValue loadOutbox(QString NotaryID, QString NymID,
                          QString AccountID);
    QJsonValue loadOutboxNoVerify(QString NotaryID, QString NymID,
                                  QString AccountID);
    QJsonValue loadPaymentInbox(QString NotaryID, QString NymID);
    QJsonValue loadPaymentInboxNoVerify(QString NotaryID, QString NymID);
    QJsonValue loadRecordBox(QString NotaryID, QString NymID,
                             QString AccountID);
    QJsonValue loadRecordBoxNoVerify(QString NotaryID, QString NymID,
                                     QString AccountID);
    QJsonValue loadExpiredBox(QString NotaryID, QString NymID);
    QJsonValue loadExpiredBoxNoVerify(QString NotaryID, QString NymID);
    QJsonValue recordPayment(QString NotaryID, QString NymID,
                             bool IsInbox, int Index,
                             bool SaveCopy);
    QJsonValue clearRecord(QString NotaryID, QString NymID,
                           QString AccountID, int Index,
                           bool ClearAll);
    QJsonValue clearExpired(QString NotaryID, QString NymID,
                            int Index, bool ClearAll);
    QJsonValue ledgerGetCount(QString NotaryID, QString NymID,
                              QString AccountID, QString Ledger);
    QJsonValue ledgerCreateResponse(QString NotaryID, QString NymID,
                                    QString AccountID, QString OriginalLedger);
    QJsonValue ledgerGetTransactionByIndex(QString NotaryID, QString NymID,
                                           QString AccountID, QString Ledger,
                                           int Index);
    QJsonValue ledgerGetTransactionByID(QString NotaryID, QString NymID,
                                        QString AccountID, QString Ledger,
                                        qint64 TransactionNumber);
    QJsonValue ledgerGetInstrument(QString NotaryID, QString NymID,
                                   QString AccountID, QString Ledger,
                                   int Index);
    QJsonValue ledgerGetTransactionIDByIndex(QString NotaryID, QString NymID,
                                             QString AccountID, QString Ledger,
                                             int Index);
    QJsonValue ledgerAddTransaction(QString NotaryID, QString NymID,
                                    QString AccountID, QString Ledger,
                                    QString Transaction);
    QJsonValue transactionCreateResponse(QString NotaryID, QString NymID,
                                         QString AccountID, QString Ledger,
                                         QString Transaction, bool DoIAccept);
    QJsonValue ledgerFinalizeResponse(QString NotaryID, QString NymID,
                                      QString AccountID, QString Ledger);
    QJsonValue transactionGetVoucher(QString NotaryID, QString NymID,
                                     QString AccountID, QString Transaction);
    QJsonValue transactionGetSenderNymID(QString NotaryID, QString NymID,
                                         QString AccountID, QString Transaction);
    QJsonValue transactionGetRecipientNymID(QString NotaryID, QString NymID,
                                            QString AccountID, QString Transaction);
    QJsonValue transactionGetSenderAccountID(QString NotaryID, QString NymID,
                                             QString AccountID, QString Transaction);
    QJsonValue transactionGetRecipientAccountID(QString NotaryID, QString NymID,
                                                QString AccountID, QString Transaction);
    QJsonValue pendingGetNote(QString NotaryID, QString NymID,
                              QString AccountID, QString Transaction);
    QJsonValue transactionGetAmount(QString NotaryID, QString NymID,
                                    QString AccountID, QString Transaction);
    QJsonValue transactionGetDisplayReferenceToNumber(QString NotaryID, QString NymID,
                                                      QString AccountID, QString Transaction);
    QJsonValue transactionGetType(QString NotaryID, QString NymID,
                                  QString AccountID, QString Transaction);
    QJsonValue replyNoticeGetRequestNumber(QString NotaryID, QString NymID,
                                           QString Transaction);
    QJsonValue transactionGetDateSigned(QString NotaryID, QString NymID,
                                        QString AccountID, QString Transaction);
    QJsonValue transactionGetSuccess(QString NotaryID, QString NymID,
                                     QString AccountID, QString Transaction);
    QJsonValue transactionIsCanceled(QString NotaryID, QString NymID,
                                     QString AccountID, QString Transaction);
    QJsonValue transactionGetBalanceAgreementSuccess(QString NotaryID, QString NymID,
                                                     QString AccountID, QString Transaction);
    QJsonValue messageGetBalanceAgreementSuccess(QString NotaryID, QString NymID,
                                                 QString AccountID, QString Message);
    QJsonValue savePurse(QString NotaryID, QString InstrumentDefinitionID,
                         QString NymID, QString Purse);
    QJsonValue loadPurse(QString NotaryID, QString InstrumentDefinitionID,
                         QString NymID);
    QJsonValue purseGetTotalValue(QString NotaryID, QString InstrumentDefinitionID,
                                  QString Purse);
    QJsonValue purseCount(QString NotaryID, QString InstrumentDefinitionID,
                          QString Purse);
    QJsonValue purseHasPassword(QString NotaryID, QString Purse);
    QJsonValue createPurse(QString NotaryID, QString InstrumentDefinitionID,
                           QString OwnerID, QString SignerID);
    QJsonValue createPursePassphrase(QString NotaryID, QString InstrumentDefinitionID,
                                     QString SignerID);
    QJsonValue pursePeek(QString NotaryID, QString InstrumentDefinitionID,
                         QString OwnerID, QString Purse);
    QJsonValue pursePop(QString NotaryID, QString InstrumentDefinitionID,
                        QString OwnerOrSignerID, QString Purse);
    QJsonValue purseEmpty(QString NotaryID, QString InstrumentDefinitionID,
                          QString SignerID, QString Purse);
    QJsonValue pursePush(QString NotaryID, QString InstrumentDefinitionID,
                         QString SignerID, QString OwnerID,
                         QString Purse, QString Token);
    QJsonValue walletImportPurse(QString NotaryID, QString InstrumentDefinitionID,
                                 QString NymID, QString Purse);
    QJsonValue exchangePurse(QString NotaryID, QString InstrumentDefinitionID,
                             QString NymID, QString Purse);
    QJsonValue tokenChangeOwner(QString NotaryID, QString InstrumentDefinitionID,
                                QString Token, QString SignerNymID,
                                QString OldOwner, QString NewOwner);
    QJsonValue tokenGetID(QString NotaryID, QString InstrumentDefinitionID,
                          QString Token);
    QJsonValue tokenGetDenomination(QString NotaryID, QString InstrumentDefinitionID,
                                    QString Token);
    QJsonValue tokenGetSeries(QString NotaryID, QString InstrumentDefinitionID,
                              QString Token);
    QJsonValue tokenGetValidFrom(QString NotaryID, QString InstrumentDefinitionID,
                                 QString Token);
    QJsonValue tokenGetValidTo(QString NotaryID, QString InstrumentDefinitionID,
                               QString Token);
    QJsonValue tokenGetInstrumentDefinitionID(QString Token);
    QJsonValue tokenGetNotaryID(QString Token);
    QJsonValue isBasketCurrency(QString InstrumentDefinitionID);
    QJsonValue basketGetMemberCount(QString InstrumentDefinitionID);
    QJsonValue basketGetMemberType(QString BasketInstrumentDefinitionID, int Index);
    QJsonValue basketGetMinimumTransferAmount(QString BasketInstrumentDefinitionID);
    QJsonValue basketGetMemberMinimumTransferAmount(QString BasketInstrumentDefinitionID, int Index);
    QJsonValue pingNotary(QString NotaryID, QString NymID);
    QJsonValue registerNym(QString NotaryID, QString NymID);
    QJsonValue unregisterNym(QString NotaryID, QString NymID);
    QJsonValue messageGetUsageCredits(QString Message);
    QJsonValue usageCredits(QString NotaryID, QString NymID,
                            QString NymIDCheck, qint64 Adjustment);
    QJsonValue checkNym(QString NotaryID, QString NymID,
                        QString NymIDCheck);
    QJsonValue sendNymMessage(QString NotaryID, QString NymID,
                              QString NymIDRecipient, QString RecipientPubkey,
                              QString Message);
    QJsonValue sendNymInstrument(QString NotaryID, QString NymID,
                                 QString NymIDRecipient, QString RecipientPubkey,
                                 QString Instrument, QString InstrumentForSender);
    QJsonValue getRequestNumber(QString NotaryID, QString NymID);
    QJsonValue registerInstrumentDefinition(QString NotaryID, QString NymID,
                                            QString Contract);
    QJsonValue getInstrumentDefinition(QString NotaryID, QString NymID,
                                       QString InstrumentDefinitionID);
    QJsonValue getMint(QString NotaryID, QString NymID,
                       QString InstrumentDefinitionID);
    QJsonValue registerAccount(QString NotaryID, QString NymID,
                               QString InstrumentDefinitionID);
    QJsonValue getAccountData(QString NotaryID, QString NymID,
                              QString AccountID);
    QJsonValue generateBasketCreation(QString NymID, qint64 MinimumTransfer);
    QJsonValue addBasketCreationItem(QString NymID, QString Basket,
                                     QString InstrumentDefinitionID, qint64 MinimumTransfer);
    QJsonValue issueBasket(QString NotaryID, QString NymID,
                           QString Basket);
    QJsonValue generateBasketExchange(QString NotaryID, QString NymID,
                                      QString BasketInstrumentDefinitionID, QString BasketAssetAccountID,
                                      int TransferMultiple);
    QJsonValue addBasketExchangeItem(QString NotaryID, QString NymID,
                                     QString Basket, QString InstrumentDefinitionID,
                                     QString AssetAccountID);
    QJsonValue exchangeBasket(QString NotaryID, QString NymID,
                              QString BasketInstrumentDefinitionID, QString Basket,
                              bool ExchangeDirection);
    QJsonValue getTransactionNumbers(QString NotaryID, QString NymID);
    QJsonValue notarizeWithdrawal(QString NotaryID, QString NymID,
                                  QString AccountID, qint64 Amount);
    QJsonValue notarizeDeposit(QString NotaryID, QString NymID,
                               QString AccountID, QString Purse);
    QJsonValue notarizeTransfer(QString NotaryID, QString NymID,
                                QString AccountFrom, QString AccountTo,
                                qint64 Amount, QString Note);
    QJsonValue getNymbox(QString NotaryID, QString NymID);
    QJsonValue processInbox(QString NotaryID, QString NymID,
                            QString AccountID, QString AccountLedger);
    QJsonValue processNymbox(QString NotaryID, QString NymID);
    QJsonValue withdrawVoucher(QString NotaryID, QString NymID,
                               QString AccountID, QString RecipientNymID,
                               QString ChequeMemo, qint64 Amount);
    QJsonValue payDividend(QString NotaryID, QString IssuerNymID,
                           QString DividendFromAccountID, QString SharesInstrumentDefinitionID,
                           QString DividendMemo, qint64 AmountPerShare);
    QJsonValue depositCheque(QString NotaryID, QString NymID,
                             QString AccountID, QString Cheque);
    QJsonValue depositPaymentPlan(QString NotaryID, QString NymID,
                                  QString PaymentPlan);
    QJsonValue killMarketOffer(QString NotaryID, QString NymID,
                               QString AssetAccountID, qint64 TransactionNumber);
    QJsonValue killPaymentPlan(QString NotaryID, QString NymID,
                               QString FromAccountID, qint64 TransactionNumber);
    QJsonValue issueMarketOffer(QString AssetAccountID, QString CurrencyAccountID,
                                qint64 MarketScale, qint64 MinimumIncrement,
                                qint64 TotalAssetsOnOffer, qint64 PriceLimit,
                                bool BuyingOrSelling, time64_t LifeSpanInSeconds,
                                QString StopSign, qint64 ActivationPrice);
    QJsonValue getMarketList(QString NotaryID, QString NymID);
    QJsonValue getMarketOffers(QString NotaryID, QString NymID,
                               QString MarketID, qint64 MaxDepth);
    QJsonValue getMarketRecentTrades(QString NotaryID, QString NymID,
                                     QString MarketID);
    QJsonValue getNymMarketOffers(QString NotaryID, QString NymID);
    QJsonValue popMessageBuffer(qint64 RequestNumber, QString NotaryID,
                                QString NymID);
    QJsonValue flushMessageBuffer();
    QJsonValue getSentMessage(qint64 RequestNumber, QString NotaryID,
                              QString NymID);
    QJsonValue removeSentMessage(qint64 RequestNumber, QString NotaryID,
                                 QString NymID);
    QJsonValue flushSentMessages(bool HarvestingForRetry, QString NotaryID,
                                 QString NymID, QString NymBox);
    QJsonValue sleep(qint64 Milliseconds);
    QJsonValue resyncNymWithServer(QString NotaryID, QString NymID,
                                   QString Message);
    QJsonValue queryInstrumentDefinitions(QString NotaryID, QString NymID,
                                          QString EncodedMap);
    QJsonValue messageGetPayload(QString Message);
    QJsonValue messageGetCommand(QString Message);
    QJsonValue messageGetLedger(QString Message);
    QJsonValue messageGetNewInstrumentDefinitionID(QString Message);
    QJsonValue messageGetNewIssuerAccountID(QString Message);
    QJsonValue messageGetNewAccountID(QString Message);
    QJsonValue messageGetNymboxHash(QString Message);
    QJsonValue messageGetSuccess(QString Message);
    QJsonValue messageGetDepth(QString Message);
    QJsonValue messageIsTransactionCanceled(QString NotaryID, QString NymID,
                                            QString AccountID, QString Message);
    QJsonValue messageGetTransactionSuccess(QString NotaryID, QString NymID,
                                            QString AccountID, QString Message);



    // Moneychanger::It() methods
    QString mcSendDialog(QString Account, QString Recipient,
                         QString Asset, QString Amount);
    QString mcRequestFundsDialog(QString Account, QString Recipient,
                                 QString Asset, QString Amount);

    bool mcActivateSmartContract(); // Init Wizard
    QJsonValue mcListSmartContracts();


    // API Key Methods
    QJsonValue userLogin(QString Username, QString PlaintextPassword);
    QJsonValue userLogout(QString Username, QString PlaintextPassword);
    QJsonValue refreshAPIKey(QString Username, QString PlaintextPassword);


    // RecordList Methods
    QJsonValue recordListPopulate();
    QJsonValue recordListCount();
    QJsonValue recordListRetrieve(int Index);
    QJsonValue recordListRetrieve(int BeginIndex, int EndIndex);

private:

    // RecordList Methods
    opentxs::OTRecordList * m_RecordList=nullptr;
    void createRecordList();

    RPCUserManager m_userManager;

    bool validateAPIKey(QString Username, QString APIKey);

};

#endif // MCRPCSERVICE_H
