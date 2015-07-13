#include "mcrpcservice.h"

#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <core/ot_worker.hpp>

#include <QDebug>
#include <QTimer>
#include <QEventLoop>
#include <QThreadPool>
#include <QRunnable>

#include <opentxs/client/OTAPI.hpp>
#include <opentxs/client/OTAPI_Exec.hpp>
#include <opentxs/core/Log.hpp>

#include <core/moneychanger.cpp>


MCRPCService::MCRPCService(QObject *parent)
    : QJsonRpcService(parent)
{
    QThreadPool::globalInstance()->setMaxThreadCount(10);
}

QJsonValue MCRPCService::numListAdd(QString NumList, QString Numbers)
{
    std::string result = opentxs::OTAPI_Wrap::It()->NumList_Add(NumList.toStdString(),
                                                               Numbers.toStdString());
    QJsonObject object{{"NumListAddResult",  QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::numListRemove(QString NumList, QString Numbers)
{
    std::string result = opentxs::OTAPI_Wrap::It()->NumList_Remove(NumList.toStdString(),
                                                               Numbers.toStdString());
    QJsonObject object{{"NumListRemoveResult",  QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::numListVerifyQuery(QString NumList, QString Numbers)
{
    bool result = opentxs::OTAPI_Wrap::It()->NumList_VerifyQuery(NumList.toStdString(),
                                                               Numbers.toStdString());
    QJsonObject object{{"NumListVerifyQueryResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::numListVerifyAll(QString NumList, QString Numbers)
{
    bool result = opentxs::OTAPI_Wrap::It()->NumList_VerifyAll(NumList.toStdString(),
                                                               Numbers.toStdString());
    QJsonObject object{{"NumListVerifyAllResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::numListCount(QString NumList)
{
    int32_t result = opentxs::OTAPI_Wrap::It()->NumList_Count(NumList.toStdString());
    QJsonObject object{{"NumListCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::createNym(int32_t KeySize, QString NymIDSource, QString AltLocation)
{
    std::string result = opentxs::OTAPI_Wrap::It()->CreateNym(KeySize,
                                                              NymIDSource.toStdString(),
                                                              AltLocation.toStdString());
    QJsonObject object{{"CreateNymResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymActiveCronItemIDs(QString NymID, QString NotaryID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_ActiveCronItemIDs(NymID.toStdString(),
                                                                             NotaryID.toStdString());
    QJsonObject object{{"ActiveCronItemIDs", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getActiveCronItem(QString NotaryID, int64_t TransNum)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetActiveCronItem(NotaryID.toStdString(),
                                                                      TransNum);
    QJsonObject object{{"ActiveCronItem", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymSourceForID(QString NymID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_SourceForID(NymID.toStdString());
    QJsonObject object{{"NymSourceForID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymAltSourceLocation(QString NymID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_AltSourceLocation(NymID.toStdString());
    QJsonObject object{{"NymAltSourceLocation", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymCredentialCount(QString NymID)
{
    int32_t result = opentxs::OTAPI_Wrap::It()->GetNym_CredentialCount(NymID.toStdString());
    QJsonObject object{{"NymCredentialCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymCredentialID(QString NymID, int32_t Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_CredentialID(NymID.toStdString(),
                                                              Index);
    QJsonObject object{{"NymCredentialID", QString(result.c_str())}};
    return QJsonValue(object);
}


QJsonValue MCRPCService::getNymCredentialContents(QString NymID, QString CredentialID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_CredentialContents(NymID.toStdString(),
                                                              CredentialID.toStdString());
    QJsonObject object{{"NymCredentialContents", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymRevokedCount(QString NymID)
{
    int32_t result = opentxs::OTAPI_Wrap::It()->GetNym_RevokedCredCount(NymID.toStdString());
    QJsonObject object{{"NymRevokedCredCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymRevokedCredID(QString NymID, int32_t Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_RevokedCredID(NymID.toStdString(),
                                                              Index);
    QJsonObject object{{"NymRevokedCredID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymRevokedCredContents(QString NymID, QString CredentialID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_RevokedCredContents(NymID.toStdString(),
                                                              CredentialID.toStdString());
    QJsonObject object{{"NymRevokedCredContents", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymSubCredentialCount(QString NymID, QString MasterCredID)
{
    int32_t result = opentxs::OTAPI_Wrap::It()->GetNym_SubcredentialCount(NymID.toStdString(),
                                                              MasterCredID.toStdString());
    QJsonObject object{{"NymSubCredentialCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymSubCredentialID(QString NymID, QString MasterCredID, int32_t Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_SubCredentialID(NymID.toStdString(),
                                                              MasterCredID.toStdString(),
                                                              Index);
    QJsonObject object{{"NymSubCredentialID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymSubCredentialContents(QString NymID, QString MasterCredID, QString SubCredID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_SubCredentialContents(NymID.toStdString(),
                                                                        MasterCredID.toStdString(),
                                                                        SubCredID.toStdString());
    QJsonObject object{{"NymSubCredentialContents", QString(result.c_str())}};
    return QJsonValue(object);
}


QJsonValue MCRPCService::addSubCredential(QString NymID, QString MasterCredID, int32_t KeySize)
{
    std::string result = opentxs::OTAPI_Wrap::It()->AddSubcredential(NymID.toStdString(),
                                                              MasterCredID.toStdString(),
                                                              KeySize);
    QJsonObject object{{"AddSubCredentialResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::revokeSubcredential(QString NymID, QString MasterCredID, QString SubCredID)
{
    bool result = opentxs::OTAPI_Wrap::It()->RevokeSubcredential(NymID.toStdString(),
                                                                        MasterCredID.toStdString(),
                                                                        SubCredID.toStdString());
    QJsonObject object{{"RevokeSubcredentialResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getSignerNymID(QString Contract)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetSignerNymID(Contract.toStdString());
    QJsonObject object{{"SignerNymID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::calculateAssetContractID(QString Contract)
{
    std::string result = opentxs::OTAPI_Wrap::It()->CalculateAssetContractID(Contract.toStdString());
    QJsonObject object{{"AssetContractID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::calculateServerContractID(QString Contract)
{
    std::string result = opentxs::OTAPI_Wrap::It()->CalculateServerContractID(Contract.toStdString());
    QJsonObject object{{"ServerContractID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::calculateContractID(QString Contract)
{
    std::string result = opentxs::OTAPI_Wrap::It()->CalculateContractID(Contract.toStdString());
    QJsonObject object{{"ContractID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::createServerContract(QString NymID, QString XMLContents)
{
    std::string result = opentxs::OTAPI_Wrap::It()->CreateServerContract(NymID.toStdString(),
                                                                        XMLContents.toStdString());
    QJsonObject object{{"CreateServerContractResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::createAssetContract(QString NymID, QString XMLContents)
{
    std::string result = opentxs::OTAPI_Wrap::It()->CreateAssetContract(NymID.toStdString(),
                                                                        XMLContents.toStdString());
    QJsonObject object{{"CreateAssetContractResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getServerContract(QString NotaryID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetServer_Contract(NotaryID.toStdString());
    QJsonObject object{{"ServerContract", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getCurrencyFactor(QString InstrumentDefinitionID)
{
    int32_t result = opentxs::OTAPI_Wrap::It()->GetCurrencyFactor(InstrumentDefinitionID.toStdString());
    QJsonObject object{{"CurrencyFactor", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getCurrencyDecimalPower(QString InstrumentDefinitionID)
{
    int32_t result = opentxs::OTAPI_Wrap::It()->GetCurrencyDecimalPower(InstrumentDefinitionID.toStdString());
    QJsonObject object{{"CurrencyDecimalPower", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getCurrencyTLA(QString InstrumentDefinitionID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetCurrencyTLA(InstrumentDefinitionID.toStdString());
    QJsonObject object{{"CurrencyTLA", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getCurrencySymbol(QString InstrumentDefinitionID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetCurrencySymbol(InstrumentDefinitionID.toStdString());
    QJsonObject object{{"CurrencySymbol", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::stringToAmountLocale(QString InstrumentDefinitionID, QString Input,
                                                   QString ThousandsSep, QString DecimalPoint)
{
    int64_t result = opentxs::OTAPI_Wrap::It()->StringToAmountLocale(InstrumentDefinitionID.toStdString(),
                                                                                    Input.toStdString(),
                                                                                    ThousandsSep.toStdString(),
                                                                                    DecimalPoint.toStdString());
    QJsonObject object{{"StringToAmountLocaleResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::formatAmountLocale(QString InstrumentDefinitionID, int64_t Amount,
                                                   QString ThousandsSep, QString DecimalPoint)
{
    std::string result = opentxs::OTAPI_Wrap::It()->FormatAmountLocale(InstrumentDefinitionID.toStdString(),
                                                                                    Amount,
                                                                                    ThousandsSep.toStdString(),
                                                                                    DecimalPoint.toStdString());
    QJsonObject object{{"FormatAmountLocaleResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::formatAmountWithoutSymbolLocale(QString InstrumentDefinitionID, int64_t Amount,
                                                   QString ThousandsSep, QString DecimalPoint)
{
    std::string result = opentxs::OTAPI_Wrap::It()->FormatAmountWithoutSymbolLocale(InstrumentDefinitionID.toStdString(),
                                                                                    Amount,
                                                                                    ThousandsSep.toStdString(),
                                                                                    DecimalPoint.toStdString());
    QJsonObject object{{"FormatAmountWithoutSymbolLocaleResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::stringToAmount(QString InstrumentDefinitionID,
                                                   QString Input)
{
    int64_t result = opentxs::OTAPI_Wrap::It()->StringToAmount(InstrumentDefinitionID.toStdString(),
                                                                       Input.toStdString());
    QJsonObject object{{"StringToAmountResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::formatAmount(QString InstrumentDefinitionID,
                                                   int64_t Amount)
{
    std::string result = opentxs::OTAPI_Wrap::It()->FormatAmount(InstrumentDefinitionID.toStdString(),
                                                                       Amount);
    QJsonObject object{{"FormatAmountResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::formatAmountWithoutSymbol(QString InstrumentDefinitionID,
                                                   int64_t Amount)
{
    std::string result = opentxs::OTAPI_Wrap::It()->FormatAmountWithoutSymbol(InstrumentDefinitionID.toStdString(),
                                                                       Amount);
    QJsonObject object{{"FormatAmountWithoutSymbolResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAssetTypeContract(QString InstrumentDefinitionID)
{
    std::string contract = opentxs::OTAPI_Wrap::It()->GetAssetType_Contract(InstrumentDefinitionID.toStdString());
    QJsonObject object{{"AssetTypeContract", QString(contract.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::addServerContract(QString Contract)
{
    int l_count = opentxs::OTAPI_Wrap::It()->AddServerContract(Contract.toStdString());
    QJsonObject object{{"AddServerContractResult", l_count}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::addAssetContract(QString Contract)
{
    int l_count = opentxs::OTAPI_Wrap::It()->AddAssetContract(Contract.toStdString());
    QJsonObject object{{"AddAssetContractResult", l_count}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymCount()
{
    int l_count = opentxs::OTAPI_Wrap::It()->GetNymCount();
    QJsonObject object{{"NymCount", l_count}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getServerCount()
{
    int l_count = opentxs::OTAPI_Wrap::It()->GetServerCount();
    QJsonObject object{{"ServerCount", l_count}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAssetTypeCount()
{
    int l_count = opentxs::OTAPI_Wrap::It()->GetAssetTypeCount();
    QJsonObject object{{"AssetTypeCount", l_count}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAccountCount()
{
    int l_count = opentxs::OTAPI_Wrap::It()->GetAccountCount();
    QJsonObject object{{"AccountCount", l_count}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletCanRemoveServer(QString NotaryID)
{
    bool result = opentxs::OTAPI_Wrap::It()->Wallet_CanRemoveServer(NotaryID.toStdString());
    QJsonObject object{{"WalletCanRemoveServerResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletRemoveServer(QString NotaryID)
{
    bool result = opentxs::OTAPI_Wrap::It()->Wallet_RemoveServer(NotaryID.toStdString());
    QJsonObject object{{"WalletRemoveServerResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletCanRemoveAssetType(QString InstrumentDefinitionID)
{
    bool result = opentxs::OTAPI_Wrap::It()->Wallet_CanRemoveAssetType(InstrumentDefinitionID.toStdString());
    QJsonObject object{{"WalletCanRemoveAssetTypeResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletRemoveAssetType(QString InstrumentDefinitionID)
{
    bool result = opentxs::OTAPI_Wrap::It()->Wallet_RemoveAssetType(InstrumentDefinitionID.toStdString());
    QJsonObject object{{"WalletRemoveAssetTypeResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletCanRemoveNym(QString NymID)
{
    bool result = opentxs::OTAPI_Wrap::It()->Wallet_CanRemoveNym(NymID.toStdString());
    QJsonObject object{{"WalletCanRemoveNymResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletRemoveNym(QString NymID)
{
    bool result = opentxs::OTAPI_Wrap::It()->Wallet_RemoveNym(NymID.toStdString());
    QJsonObject object{{"WalletRemoveNymResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletCanRemoveAccount(QString AccountID)
{
    bool result = opentxs::OTAPI_Wrap::It()->Wallet_CanRemoveAccount(AccountID.toStdString());
    QJsonObject object{{"WalletCanRemoveAccountResult", result}};
    return QJsonValue(object);
}




QString MCRPCService::mcSendDialog(QString Account, QString Recipient,
                                   QString Asset, QString Amount)
{
    /*
    if(Amount.isEmpty())
    {
        return "Error: Amount not defined";
    }
    if(Recipient.isEmpty())
    {
        return "Error: Recipient not defined";
    }
    */

    Moneychanger::It()->mc_rpc_sendfunds_show_dialog(Account, Recipient, Asset, Amount);
    return "Success";
}

QString MCRPCService::mcRequestFundsDialog(QString Account, QString Recipient,
                                           QString Asset, QString Amount)
{
    /*
    if(Amount.isEmpty())
    {
        return "Error: Amount not defined";
    }
    if(Recipient.isEmpty())
    {
        return "Error: Recipient not defined";
    }
    */

    Moneychanger::It()->mc_rpc_requestfunds_show_dialog(Account, Recipient, Asset, Amount);
    return "Success";
}

bool MCRPCService::mcActivateSmartContract()
{
    // This function should take the ID of a smart contract
    // And launch the Smart Contracts wizard accordingly
    return false;
}

QJsonValue MCRPCService::mcListSmartContracts()
{
    QString Contracts("Placeholder");
    QJsonObject object{{"SmartContractsList", Contracts}};

    return object;
}
