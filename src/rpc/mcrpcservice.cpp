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
    QJsonObject object{{"StringToAmountLocaleResult", QString(std::to_string(result).c_str())}};
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
    QJsonObject object{{"StringToAmountResult", QString(std::to_string(result).c_str())}};
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

QJsonValue MCRPCService::doesBoxReceiptExist(QString NotaryID, QString NymID,
                               QString AccountID, int32_t BoxType,
                               int64_t TransactionNumber)
{
    bool result = opentxs::OTAPI_Wrap::It()->DoesBoxReceiptExist(NotaryID.toStdString(),
                                                                 NymID.toStdString(),
                                                                 AccountID.toStdString(),
                                                                 BoxType,
                                                                 TransactionNumber);
    QJsonObject object{{"DoesBoxReceiptExistResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getBoxReceipt(QString NotaryID, QString NymID,
                               QString AccountID, int32_t BoxType,
                               int64_t TransactionNumber)
{
    int32_t result = opentxs::OTAPI_Wrap::It()->getBoxReceipt(NotaryID.toStdString(),
                                                                 NymID.toStdString(),
                                                                 AccountID.toStdString(),
                                                                 BoxType,
                                                                 TransactionNumber);
    QJsonObject object{{"BoxReceipt", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::deleteAssetAccount(QString NotaryID,
                                            QString NymID,
                                            QString AccountID)
{
    int32_t result = opentxs::OTAPI_Wrap::It()->deleteAssetAccount(NotaryID.toStdString(),
                                                                 NymID.toStdString(),
                                                                 AccountID.toStdString());
    QJsonObject object{{"DeleteAssetAccountResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletExportNym(QString NymID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Wallet_ExportNym(NymID.toStdString());
    QJsonObject object{{"ExportedNym", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletExportCert(QString NymID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Wallet_ExportCert(NymID.toStdString());
    QJsonObject object{{"ExportedCert", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletImportNym(QString FileContents)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Wallet_ImportNym(FileContents.toStdString());
    QJsonObject object{{"WalletImportNymResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletImportCert(QString DisplayName, QString FileContents)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Wallet_ImportCert(DisplayName.toStdString(),
                                                                      FileContents.toStdString());
    QJsonObject object{{"WalletImportCertResult", QString(result.c_str())}};
    return QJsonValue(object);
}

/*
QJsonValue MCRPCService::walletChangePassphrase()
{
    std::string result = opentxs::OTAPI_Wrap::It()->Wallet_ChangePassphrase();
    QJsonObject object{{"WalletChangePassphraseResult", QString(result.c_str())}};
    return QJsonValue(object);
}
*/

QJsonValue MCRPCService::walletGetNymIdFromPartial(QString PartialID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Wallet_GetNymIDFromPartial(PartialID.toStdString());
    QJsonObject object{{"NymIdFromPartial", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletGetNotaryIdFromPartial(QString PartialID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Wallet_GetNotaryIDFromPartial(PartialID.toStdString());
    QJsonObject object{{"NotaryIdFromPartial", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletGetInstrumentDefinitionIdFromPartial(QString PartialID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Wallet_GetInstrumentDefinitionIDFromPartial(PartialID.toStdString());
    QJsonObject object{{"InstrumentDefinitionIdFromPartial", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletGetAccountIdFromPartial(QString PartialID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Wallet_GetAccountIDFromPartial(PartialID.toStdString());
    QJsonObject object{{"AccountIdFromPartial", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymID(int32_t Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_ID(Index);
    QJsonObject object{{"NymID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymName(QString NymID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_Name(NymID.toStdString());
    QJsonObject object{{"NymName", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::isNymRegisteredAtServer(QString NymID, QString NotaryID)
{
    bool result = opentxs::OTAPI_Wrap::It()->IsNym_RegisteredAtServer(NymID.toStdString(),
                                                                      NotaryID.toStdString());
    QJsonObject object{{"IsNymRegisteredAtServerResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymStats(QString NymID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_Stats(NymID.toStdString());
    QJsonObject object{{"NymStats", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymNymboxHash(QString NotaryID, QString NymID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_NymboxHash(NotaryID.toStdString(),
                                                                      NymID.toStdString());
    QJsonObject object{{"NymNymboxHash", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymRecentHash(QString NotaryID, QString NymID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_RecentHash(NotaryID.toStdString(),
                                                                      NymID.toStdString());
    QJsonObject object{{"NymRecentHash", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymInboxHash(QString NotaryID, QString NymID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_InboxHash(NotaryID.toStdString(),
                                                                      NymID.toStdString());
    QJsonObject object{{"NymInboxHash", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymOutboxHash(QString NotaryID, QString NymID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_OutboxHash(NotaryID.toStdString(),
                                                                      NymID.toStdString());
    QJsonObject object{{"NymOutboxHash", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymMailCount(QString NymID)
{
    int32_t result = opentxs::OTAPI_Wrap::It()->GetNym_MailCount(NymID.toStdString());
    QJsonObject object{{"NymMailCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymContentsByIndex(QString NymID, int32_t Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_MailContentsByIndex(NymID.toStdString(),
                                                                               Index);
    QJsonObject object{{"NymContentsByIndex", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymMailSenderIDByIndex(QString NymID, int32_t Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_MailSenderIDByIndex(NymID.toStdString(),
                                                                               Index);
    QJsonObject object{{"NymMailSenderIDByIndex", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymMailNotaryIDByIndex(QString NymID, int32_t Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_MailNotaryIDByIndex(NymID.toStdString(),
                                                                               Index);
    QJsonObject object{{"NymMailNotaryIDByIndex", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::nymRemoveMailByIndex(QString NymID, int32_t Index)
{
    bool result = opentxs::OTAPI_Wrap::It()->Nym_RemoveMailByIndex(NymID.toStdString(),
                                                                   Index);
    QJsonObject object{{"NymRemoveMailByIndexResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::nymVerifyMailByIndex(QString NymID, int32_t Index)
{
    bool result = opentxs::OTAPI_Wrap::It()->Nym_VerifyMailByIndex(NymID.toStdString(),
                                                                   Index);
    QJsonObject object{{"NymVerifyMailByIndexResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymOutmailCount(QString NymID)
{
    int32_t result = opentxs::OTAPI_Wrap::It()->GetNym_OutmailCount(NymID.toStdString());
    QJsonObject object{{"NymOutmailCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymOutmailContentsByIndex(QString NymID, int32_t Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_OutmailContentsByIndex(NymID.toStdString(),
                                                                                  Index);
    QJsonObject object{{"NymOutmailContentsIDByIndex", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymOutmailRecipientIDByIndex(QString NymID, int32_t Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_OutmailRecipientIDByIndex(NymID.toStdString(),
                                                                                     Index);
    QJsonObject object{{"NymOutmailRecipientIDByIndex", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymOutmailNotaryIDByIndex(QString NymID, int32_t Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_OutmailNotaryIDByIndex(NymID.toStdString(),
                                                                                  Index);
    QJsonObject object{{"NymOutmailNotaryIDIDByIndex", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::nymRemoveOutmailByIndex(QString NymID, int32_t Index)
{
    bool result = opentxs::OTAPI_Wrap::It()->Nym_RemoveOutmailByIndex(NymID.toStdString(),
                                                                      Index);
    QJsonObject object{{"NymRemoveOutmailByIndexResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::nymVerifyOutmailByIndex(QString NymID, int32_t Index)
{
    bool result = opentxs::OTAPI_Wrap::It()->Nym_VerifyOutmailByIndex(NymID.toStdString(),
                                                                      Index);
    QJsonObject object{{"NymVerifyOutmailByIndexResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymOutpaymentsCount(QString NymID)
{
    int32_t result = opentxs::OTAPI_Wrap::It()->GetNym_OutpaymentsCount(NymID.toStdString());
    QJsonObject object{{"NymOutpaymentsCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymOutpaymentsContentsByIndex(QString NymID, int32_t Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_OutpaymentsContentsByIndex(NymID.toStdString(),
                                                                                      Index);
    QJsonObject object{{"NymOutpaymentsContentsByIndex", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymOutpaymentsRecipientIDByIndex(QString NymID, int32_t Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_OutpaymentsRecipientIDByIndex(NymID.toStdString(),
                                                                                         Index);
    QJsonObject object{{"NymOutpaymentsRecipientIDByIndex", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymOutpaymentsNotaryIDByIndex(QString NymID, int32_t Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_OutpaymentsNotaryIDByIndex(NymID.toStdString(),
                                                                                      Index);
    QJsonObject object{{"NymOutpaymentsNotaryIDByIndex", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::nymRemoveOutpaymentsByIndex(QString NymID, int32_t Index)
{
    bool result = opentxs::OTAPI_Wrap::It()->Nym_RemoveOutpaymentsByIndex(NymID.toStdString(),
                                                                          Index);
    QJsonObject object{{"NymRemoveOutpaymentsByIndexResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::nymVerifyOutpaymentsByIndex(QString NymID, int32_t Index)
{
    bool result = opentxs::OTAPI_Wrap::It()->Nym_VerifyOutpaymentsByIndex(NymID.toStdString(),
                                                                          Index);
    QJsonObject object{{"NymVerifyOutpaymentsByIndexResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetAmount(QString Instrument)
{
    int32_t result = opentxs::OTAPI_Wrap::It()->Instrmnt_GetAmount(Instrument.toStdString());
    QJsonObject object{{"InstrumentAmount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetTransactionNumber(QString Instrument)
{
    int32_t result = opentxs::OTAPI_Wrap::It()->Instrmnt_GetTransNum(Instrument.toStdString());
    QJsonObject object{{"InstrumentTransactionNumber", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetValidFrom(QString Instrument)
{
    time64_t result = opentxs::OTAPI_Wrap::It()->Instrmnt_GetValidFrom(Instrument.toStdString());
    QJsonObject object{{"InstrumentValidFrom", QString(std::to_string(result).c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetValidTo(QString Instrument)
{
    time64_t result = opentxs::OTAPI_Wrap::It()->Instrmnt_GetValidTo(Instrument.toStdString());
    QJsonObject object{{"InstrumentValidTo", QString(std::to_string(result).c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetType(QString Instrument)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Instrmnt_GetType(Instrument.toStdString());
    QJsonObject object{{"InstrumentType", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetMemo(QString Instrument)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Instrmnt_GetMemo(Instrument.toStdString());
    QJsonObject object{{"InstrumentMemo", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetNotaryID(QString Instrument)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Instrmnt_GetNotaryID(Instrument.toStdString());
    QJsonObject object{{"InstrumentNotaryID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetInstrumentDefinitionID(QString Instrument)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Instrmnt_GetInstrumentDefinitionID(Instrument.toStdString());
    QJsonObject object{{"InstrumentDefinitionID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetRemitterNymID(QString Instrument)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Instrmnt_GetRemitterNymID(Instrument.toStdString());
    QJsonObject object{{"InstrumentRemitterNymID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetRemitterAccountID(QString Instrument)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Instrmnt_GetRemitterAcctID(Instrument.toStdString());
    QJsonObject object{{"InstrumentRemitterAccountID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetSenderNymID(QString Instrument)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Instrmnt_GetSenderNymID(Instrument.toStdString());
    QJsonObject object{{"InstrumentSenderNymID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetSenderAccountID(QString Instrument)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Instrmnt_GetSenderAcctID(Instrument.toStdString());
    QJsonObject object{{"InstrumentSenderAccountID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetRecipientNymID(QString Instrument)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Instrmnt_GetRecipientNymID(Instrument.toStdString());
    QJsonObject object{{"InstrumentRecipientNymID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetRecipientAccountID(QString Instrument)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Instrmnt_GetRecipientAcctID(Instrument.toStdString());
    QJsonObject object{{"InstrumentRecipientAccountID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::setNymName(QString NymID,
                                    QString SignerNymID,
                                    QString NewName)
{
    bool result = opentxs::OTAPI_Wrap::It()->SetNym_Name(NymID.toStdString(),
                                                         SignerNymID.toStdString(),
                                                         NewName.toStdString());
    QJsonObject object{{"SetNymNameResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::setServerName(QString NotaryID,
                                       QString NewName)
{
    bool result = opentxs::OTAPI_Wrap::It()->SetServer_Name(NotaryID.toStdString(),
                                                         NewName.toStdString());
    QJsonObject object{{"SetServerNameResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::setAssetTypeName(QString InstrumentDefinitionID,
                                       QString NewName)
{
    bool result = opentxs::OTAPI_Wrap::It()->SetAssetType_Name(InstrumentDefinitionID.toStdString(),
                                                         NewName.toStdString());
    QJsonObject object{{"SetAssetTypeNameResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymTransactionNumberCount(QString NotaryID, QString NymID)
{
    int32_t result = opentxs::OTAPI_Wrap::It()->GetNym_TransactionNumCount(NotaryID.toStdString(), NymID.toStdString());
    QJsonObject object{{"NymTransactionNumberCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getServerID(int32_t Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetServer_ID(Index);
    QJsonObject object{{"ServerID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getServerName(QString ServerID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetServer_Name(ServerID.toStdString());
    QJsonObject object{{"ServerID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAssetTypeID(int32_t Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetAssetType_ID(Index);
    QJsonObject object{{"AssetTypeID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAssetTypeName(QString AssetTypeID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetAssetType_Name(AssetTypeID.toStdString());
    QJsonObject object{{"AssetTypeName", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAssetTypeTLA(QString AssetTypeID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetAssetType_TLA(AssetTypeID.toStdString());
    QJsonObject object{{"AssetTypeTLA", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAccountWalletID(int32_t Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetAccountWallet_ID(Index);
    QJsonObject object{{"AccountWalletID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAccountWalletName(QString AccountWalletID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetAccountWallet_Name(AccountWalletID.toStdString());
    QJsonObject object{{"AccountWalletName", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAccountWalletInboxHash(QString AccountWalletID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetAccountWallet_InboxHash(AccountWalletID.toStdString());
    QJsonObject object{{"AccountWalletInboxHash", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAccountWalletOutboxHash(QString AccountWalletID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetAccountWallet_OutboxHash(AccountWalletID.toStdString());
    QJsonObject object{{"AccountWalletOutboxHash", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getTime(void)
{
    int32_t result = opentxs::OTAPI_Wrap::It()->GetTime();
    QJsonObject object{{"Time", result}};
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
