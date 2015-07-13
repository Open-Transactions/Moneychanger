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
    QJsonValue createNym(int32_t KeySize, QString NymIDSource, QString AltLocation);
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
    QJsonValue getNymSubCredentialID(QString NymID, QString MasterCredID, int32_t Index);
    QJsonValue getNymSubCredentialContents(QString NymID, QString MasterCredID, QString SubCredID);
    QJsonValue addSubCredential(QString NymID, QString MasterCredID, int32_t KeySize);
    QJsonValue revokeSubcredential(QString NymID, QString MasterCredID, QString SubCredID);
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
    QJsonValue stringToAmount(QString InstrumentDefinitionID,
                              QString Input);
    QJsonValue formatAmount(QString InstrumentDefinitionID,
                            int64_t Amount);
    QJsonValue formatAmountWithoutSymbol(QString InstrumentDefinitionID,
                                         int64_t Amount);
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




    // Moneychanger::It() methods
    QString mcSendDialog(QString Account, QString Recipient,
                         QString Asset, QString Amount);
    QString mcRequestFundsDialog(QString Account, QString Recipient,
                                 QString Asset, QString Amount);

    bool mcActivateSmartContract(); // Init Wizard
    QJsonValue mcListSmartContracts();


};

#endif // MCRPCSERVICE_H
