#ifndef MCRPCSERVICE_H
#define MCRPCSERVICE_H

#include <qjsonrpcservice.h>

class MCRPCService : public QJsonRpcService
{
private:
    Q_OBJECT
    Q_CLASSINFO("serviceName", "moneychanger")

public:
    MCRPCService(QObject *parent = 0);

public Q_SLOTS:

    // opentxs::OTAPI methods
    QJsonValue numListAdd(QString NumList, QString Numbers);
    QJsonValue numListRemove(QString NumList, QString Numbers);
    QJsonValue numListVerifyQuery(QString NumList, QString Numbers);
    QJsonValue numListVerifyAll(QString NumList, QString Numbers);
    QJsonValue numListCount(QString NumList);
    QJsonValue createNym(int32_t KeySize, QString NymIDSource,
                         QString AltLocation);
    QJsonValue getNymActiveCronItemIDs(QString NymID, QString NotaryID);
    QJsonValue getActiveCronItem(QString NotaryID, int64_t TransNum);
    QJsonValue getNymSourceForID(QString NymID);
    QJsonValue getNymAltSourceLocation(QString NymID);
    QJsonValue getNymCredentialCount(QString NymID);
    QJsonValue getNymCredentialID(QString NymID, int32_t Index);
    QJsonValue getNymCredentialContents(QString NymID, QString CredentialID);
    QJsonValue getNymRevokedCount(QString NymID);
    QJsonValue getNymRevokedCredID(QString NymID, int32_t Index);
    QJsonValue getNymRevokedCredContents(QString NymID, QString CredentialID);
    QJsonValue getNymSubCredentialCount(QString NymID, QString MasterCredID);
    QJsonValue getNymSubCredentialID(QString NymID, QString MasterCredID,
                                     int32_t Index);
    QJsonValue getNymSubCredentialContents(QString NymID, QString MasterCredID,
                                           QString SubCredID);
    QJsonValue addSubCredential(QString NymID, QString MasterCredID,
                                int32_t KeySize);
    QJsonValue revokeSubcredential(QString NymID, QString MasterCredID,
                                   QString SubCredID);
    QJsonValue getSignerNymID(QString Contract);
    QJsonValue calculateAssetContractID(QString Contract);
    QJsonValue calculateServerContractID(QString Contract);
    QJsonValue calculateContractID(QString Contract);
    QJsonValue createServerContract(QString NymID, QString XMLContents);
    QJsonValue createAssetContract(QString NymID, QString XMLContents);
    QJsonValue getServerContract(QString NotaryID);
    QJsonValue getCurrencyFactor(QString InstrumentDefinitionID);
    QJsonValue getCurrencyDecimalPower(QString InstrumentDefinitionID);
    QJsonValue getCurrencyTLA(QString InstrumentDefinitionID);
    QJsonValue getCurrencySymbol(QString InstrumentDefinitionID);
    QJsonValue stringToAmountLocale(QString InstrumentDefinitionID, QString Input,
                                    QString ThousandsSep, QString DecimalPoint);
    QJsonValue formatAmountLocale(QString InstrumentDefinitionID, int64_t Amount,
                                  QString ThousandsSep, QString DecimalPoint);
    QJsonValue formatAmountWithoutSymbolLocale(QString InstrumentDefinitionID, int64_t Amount,
                                               QString ThousandsSep, QString DecimalPoint);
    QJsonValue stringToAmount(QString InstrumentDefinitionID, QString Input);
    QJsonValue formatAmount(QString InstrumentDefinitionID, int64_t Amount);
    QJsonValue formatAmountWithoutSymbol(QString InstrumentDefinitionID, int64_t Amount);
    QJsonValue getAssetTypeContract(QString InstrumentDefinitionID);
    QJsonValue addServerContract(QString Contract);
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
                                   QString AccountID, int32_t BoxType,
                                   int64_t TransactionNumber);
    QJsonValue getBoxReceipt(QString NotaryID, QString NymID,
                                   QString AccountID, int32_t BoxType,
                                   int64_t TransactionNumber);
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
    QJsonValue getNymID(int32_t Index);
    QJsonValue getNymName(QString NymID);
    QJsonValue isNymRegisteredAtServer(QString NymID, QString NotaryID);
    QJsonValue getNymStats(QString NymID);
    QJsonValue getNymNymboxHash(QString NotaryID, QString NymID);
    QJsonValue getNymRecentHash(QString NotaryID, QString NymID);
    QJsonValue getNymInboxHash(QString NotaryID, QString NymID);
    QJsonValue getNymOutboxHash(QString NotaryID, QString NymID);
    QJsonValue getNymMailCount(QString NymID);
    QJsonValue getNymContentsByIndex(QString NymID, int32_t Index);
    QJsonValue getNymMailSenderIDByIndex(QString NymID, int32_t Index);
    QJsonValue getNymMailNotaryIDByIndex(QString NymID, int32_t Index);
    QJsonValue nymRemoveMailByIndex(QString NymID, int32_t Index);
    QJsonValue nymVerifyMailByIndex(QString NymID, int32_t Index);
    QJsonValue getNymOutmailCount(QString NymID);
    QJsonValue getNymOutmailContentsByIndex(QString NymID, int32_t Index);
    QJsonValue getNymOutmailRecipientIDByIndex(QString NymID, int32_t Index);
    QJsonValue getNymOutmailNotaryIDByIndex(QString NymID, int32_t Index);
    QJsonValue nymRemoveOutmailByIndex(QString NymID, int32_t Index);
    QJsonValue nymVerifyOutmailByIndex(QString NymID, int32_t Index);
    QJsonValue getNymOutpaymentsCount(QString NymID);
    QJsonValue getNymOutpaymentsContentsByIndex(QString NymID, int32_t Index);
    QJsonValue getNymOutpaymentsRecipientIDByIndex(QString NymID, int32_t Index);
    QJsonValue getNymOutpaymentsNotaryIDByIndex(QString NymID, int32_t Index);
    QJsonValue nymRemoveOutpaymentsByIndex(QString NymID, int32_t Index);
    QJsonValue nymVerifyOutpaymentsByIndex(QString NymID, int32_t Index);
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
    QJsonValue getServerID(int32_t Index);
    QJsonValue getServerName(QString ServerID);
    QJsonValue getAssetTypeID(int32_t Index);
    QJsonValue getAssetTypeName(QString AssetTypeID);
    QJsonValue getAssetTypeTLA(QString AssetTypeID);
    QJsonValue getAccountWalletID(int32_t Index);
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






    // Moneychanger::It() methods
    QString mcSendDialog(QString Account, QString Recipient,
                         QString Asset, QString Amount);
    QString mcRequestFundsDialog(QString Account, QString Recipient,
                                 QString Asset, QString Amount);

    bool mcActivateSmartContract(); // Init Wizard
    QJsonValue mcListSmartContracts();


};

#endif // MCRPCSERVICE_H
