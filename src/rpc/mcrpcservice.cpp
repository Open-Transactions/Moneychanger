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
    int result = opentxs::OTAPI_Wrap::It()->NumList_Count(NumList.toStdString());
    QJsonObject object{{"NumListCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::createNym(int KeySize, QString NymIDSource,
                                   QString AltLocation)
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

QJsonValue MCRPCService::getActiveCronItem(QString NotaryID, qint64 TransNum)
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
    int result = opentxs::OTAPI_Wrap::It()->GetNym_CredentialCount(NymID.toStdString());
    QJsonObject object{{"NymCredentialCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymCredentialID(QString NymID, int Index)
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
    int result = opentxs::OTAPI_Wrap::It()->GetNym_RevokedCredCount(NymID.toStdString());
    QJsonObject object{{"NymRevokedCredCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymRevokedCredID(QString NymID, int Index)
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
    int result = opentxs::OTAPI_Wrap::It()->GetNym_SubcredentialCount(NymID.toStdString(),
                                                                          MasterCredID.toStdString());
    QJsonObject object{{"NymSubCredentialCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymSubCredentialID(QString NymID, QString MasterCredID,
                                               int Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_SubCredentialID(NymID.toStdString(),
                                                                           MasterCredID.toStdString(),
                                                              Index);
    QJsonObject object{{"NymSubCredentialID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymSubCredentialContents(QString NymID, QString MasterCredID,
                                                     QString SubCredID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_SubCredentialContents(NymID.toStdString(),
                                                                                 MasterCredID.toStdString(),
                                                                                 SubCredID.toStdString());
    QJsonObject object{{"NymSubCredentialContents", QString(result.c_str())}};
    return QJsonValue(object);
}


QJsonValue MCRPCService::addSubCredential(QString NymID, QString MasterCredID,
                                          int KeySize)
{
    std::string result = opentxs::OTAPI_Wrap::It()->AddSubcredential(NymID.toStdString(),
                                                                     MasterCredID.toStdString(),
                                                                     KeySize);
    QJsonObject object{{"AddSubCredentialResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::revokeSubcredential(QString NymID, QString MasterCredID,
                                             QString SubCredID)
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
    int result = opentxs::OTAPI_Wrap::It()->GetCurrencyFactor(InstrumentDefinitionID.toStdString());
    QJsonObject object{{"CurrencyFactor", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getCurrencyDecimalPower(QString InstrumentDefinitionID)
{
    int result = opentxs::OTAPI_Wrap::It()->GetCurrencyDecimalPower(InstrumentDefinitionID.toStdString());
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
    qint64 result = opentxs::OTAPI_Wrap::It()->StringToAmountLocale(InstrumentDefinitionID.toStdString(),
                                                                                    Input.toStdString(),
                                                                                    ThousandsSep.toStdString(),
                                                                                    DecimalPoint.toStdString());
    QJsonObject object{{"StringToAmountLocaleResult", QString(std::to_string(result).c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::formatAmountLocale(QString InstrumentDefinitionID, qint64 Amount,
                                            QString ThousandsSep, QString DecimalPoint)
{
    std::string result = opentxs::OTAPI_Wrap::It()->FormatAmountLocale(InstrumentDefinitionID.toStdString(),
                                                                                    Amount,
                                                                                    ThousandsSep.toStdString(),
                                                                                    DecimalPoint.toStdString());
    QJsonObject object{{"FormatAmountLocaleResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::formatAmountWithoutSymbolLocale(QString InstrumentDefinitionID, qint64 Amount,
                                                         QString ThousandsSep, QString DecimalPoint)
{
    std::string result = opentxs::OTAPI_Wrap::It()->FormatAmountWithoutSymbolLocale(InstrumentDefinitionID.toStdString(),
                                                                                    Amount,
                                                                                    ThousandsSep.toStdString(),
                                                                                    DecimalPoint.toStdString());
    QJsonObject object{{"FormatAmountWithoutSymbolLocaleResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::stringToAmount(QString InstrumentDefinitionID, QString Input)
{
    qint64 result = opentxs::OTAPI_Wrap::It()->StringToAmount(InstrumentDefinitionID.toStdString(),
                                                               Input.toStdString());
    QJsonObject object{{"StringToAmountResult", QString(std::to_string(result).c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::formatAmount(QString InstrumentDefinitionID, qint64 Amount)
{
    std::string result = opentxs::OTAPI_Wrap::It()->FormatAmount(InstrumentDefinitionID.toStdString(),
                                                                 Amount);
    QJsonObject object{{"FormatAmountResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::formatAmountWithoutSymbol(QString InstrumentDefinitionID, qint64 Amount)
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
                                             QString AccountID, int BoxType,
                                             qint64 TransactionNumber)
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
                                       QString AccountID, int BoxType,
                                       qint64 TransactionNumber)
{
    int result = opentxs::OTAPI_Wrap::It()->getBoxReceipt(NotaryID.toStdString(),
                                                                 NymID.toStdString(),
                                                                 AccountID.toStdString(),
                                                                 BoxType,
                                                                 TransactionNumber);
    QJsonObject object{{"BoxReceipt", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::deleteAssetAccount(QString NotaryID, QString NymID,
                                            QString AccountID)
{
    int result = opentxs::OTAPI_Wrap::It()->deleteAssetAccount(NotaryID.toStdString(),
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

QJsonValue MCRPCService::getNymID(int Index)
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
    int result = opentxs::OTAPI_Wrap::It()->GetNym_MailCount(NymID.toStdString());
    QJsonObject object{{"NymMailCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymContentsByIndex(QString NymID, int Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_MailContentsByIndex(NymID.toStdString(),
                                                                               Index);
    QJsonObject object{{"NymContentsByIndex", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymMailSenderIDByIndex(QString NymID, int Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_MailSenderIDByIndex(NymID.toStdString(),
                                                                               Index);
    QJsonObject object{{"NymMailSenderIDByIndex", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymMailNotaryIDByIndex(QString NymID, int Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_MailNotaryIDByIndex(NymID.toStdString(),
                                                                               Index);
    QJsonObject object{{"NymMailNotaryIDByIndex", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::nymRemoveMailByIndex(QString NymID, int Index)
{
    bool result = opentxs::OTAPI_Wrap::It()->Nym_RemoveMailByIndex(NymID.toStdString(),
                                                                   Index);
    QJsonObject object{{"NymRemoveMailByIndexResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::nymVerifyMailByIndex(QString NymID, int Index)
{
    bool result = opentxs::OTAPI_Wrap::It()->Nym_VerifyMailByIndex(NymID.toStdString(),
                                                                   Index);
    QJsonObject object{{"NymVerifyMailByIndexResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymOutmailCount(QString NymID)
{
    int result = opentxs::OTAPI_Wrap::It()->GetNym_OutmailCount(NymID.toStdString());
    QJsonObject object{{"NymOutmailCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymOutmailContentsByIndex(QString NymID, int Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_OutmailContentsByIndex(NymID.toStdString(),
                                                                                  Index);
    QJsonObject object{{"NymOutmailContentsIDByIndex", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymOutmailRecipientIDByIndex(QString NymID, int Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_OutmailRecipientIDByIndex(NymID.toStdString(),
                                                                                     Index);
    QJsonObject object{{"NymOutmailRecipientIDByIndex", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymOutmailNotaryIDByIndex(QString NymID, int Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_OutmailNotaryIDByIndex(NymID.toStdString(),
                                                                                  Index);
    QJsonObject object{{"NymOutmailNotaryIDIDByIndex", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::nymRemoveOutmailByIndex(QString NymID, int Index)
{
    bool result = opentxs::OTAPI_Wrap::It()->Nym_RemoveOutmailByIndex(NymID.toStdString(),
                                                                      Index);
    QJsonObject object{{"NymRemoveOutmailByIndexResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::nymVerifyOutmailByIndex(QString NymID, int Index)
{
    bool result = opentxs::OTAPI_Wrap::It()->Nym_VerifyOutmailByIndex(NymID.toStdString(),
                                                                      Index);
    QJsonObject object{{"NymVerifyOutmailByIndexResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymOutpaymentsCount(QString NymID)
{
    int result = opentxs::OTAPI_Wrap::It()->GetNym_OutpaymentsCount(NymID.toStdString());
    QJsonObject object{{"NymOutpaymentsCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymOutpaymentsContentsByIndex(QString NymID, int Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_OutpaymentsContentsByIndex(NymID.toStdString(),
                                                                                      Index);
    QJsonObject object{{"NymOutpaymentsContentsByIndex", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymOutpaymentsRecipientIDByIndex(QString NymID, int Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_OutpaymentsRecipientIDByIndex(NymID.toStdString(),
                                                                                         Index);
    QJsonObject object{{"NymOutpaymentsRecipientIDByIndex", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymOutpaymentsNotaryIDByIndex(QString NymID, int Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_OutpaymentsNotaryIDByIndex(NymID.toStdString(),
                                                                                      Index);
    QJsonObject object{{"NymOutpaymentsNotaryIDByIndex", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::nymRemoveOutpaymentsByIndex(QString NymID, int Index)
{
    bool result = opentxs::OTAPI_Wrap::It()->Nym_RemoveOutpaymentsByIndex(NymID.toStdString(),
                                                                          Index);
    QJsonObject object{{"NymRemoveOutpaymentsByIndexResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::nymVerifyOutpaymentsByIndex(QString NymID, int Index)
{
    bool result = opentxs::OTAPI_Wrap::It()->Nym_VerifyOutpaymentsByIndex(NymID.toStdString(),
                                                                          Index);
    QJsonObject object{{"NymVerifyOutpaymentsByIndexResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetAmount(QString Instrument)
{
    int result = opentxs::OTAPI_Wrap::It()->Instrmnt_GetAmount(Instrument.toStdString());
    QJsonObject object{{"InstrumentAmount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetTransactionNumber(QString Instrument)
{
    int result = opentxs::OTAPI_Wrap::It()->Instrmnt_GetTransNum(Instrument.toStdString());
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

QJsonValue MCRPCService::setNymName(QString NymID, QString SignerNymID,
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
    int result = opentxs::OTAPI_Wrap::It()->GetNym_TransactionNumCount(NotaryID.toStdString(), NymID.toStdString());
    QJsonObject object{{"NymTransactionNumberCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getServerID(int Index)
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

QJsonValue MCRPCService::getAssetTypeID(int Index)
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

QJsonValue MCRPCService::getAccountWalletID(int Index)
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
    int result = opentxs::OTAPI_Wrap::It()->GetTime();
    QJsonObject object{{"Time", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::encode(QString Plaintext, bool LineBreaks)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Encode(Plaintext.toStdString(),
                                                           LineBreaks);
    QJsonObject object{{"EncodedText", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::decode(QString Plaintext, bool LineBreaks)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Decode(Plaintext.toStdString(),
                                                           LineBreaks);
    QJsonObject object{{"DecodedText", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::encrypt(QString RecipientNymID, QString Plaintext)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Encrypt(RecipientNymID.toStdString(),
                                                            Plaintext.toStdString());
    QJsonObject object{{"EncryptedText", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::decrypt(QString RecipientNymID, QString CipherText)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Decrypt(RecipientNymID.toStdString(),
                                                            CipherText.toStdString());
    QJsonObject object{{"DecryptedText", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::createSymmetricKey(void)
{
    std::string result = opentxs::OTAPI_Wrap::It()->CreateSymmetricKey();
    QJsonObject object{{"SymmetricKey", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::symmetricEncrypt(QString SymmetricKey, QString Plaintext)
{
    std::string result = opentxs::OTAPI_Wrap::It()->SymmetricEncrypt(SymmetricKey.toStdString(),
                                                                     Plaintext.toStdString());
    QJsonObject object{{"EncryptedText", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::symmetricDecrypt(QString SymmetricKey, QString CipherTextEnvelope)
{
    std::string result = opentxs::OTAPI_Wrap::It()->SymmetricDecrypt(SymmetricKey.toStdString(),
                                                                     CipherTextEnvelope.toStdString());
    QJsonObject object{{"DecryptedText", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::signContract(QString SignerNymID, QString Contract)
{
    std::string result = opentxs::OTAPI_Wrap::It()->SignContract(SignerNymID.toStdString(),
                                                                 Contract.toStdString());
    QJsonObject object{{"SignedContract", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::flatSign(QString SignerNymID, QString Input,
                                  QString ContractType)
{
    std::string result = opentxs::OTAPI_Wrap::It()->FlatSign(SignerNymID.toStdString(),
                                                             Input.toStdString(),
                                                             ContractType.toStdString());
    QJsonObject object{{"FlatSignedText", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::addSignature(QString SignerNymID, QString Contract)
{
    std::string result = opentxs::OTAPI_Wrap::It()->AddSignature(SignerNymID.toStdString(),
                                                                 Contract.toStdString());
    QJsonObject object{{"SignedContract", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::verifySignature(QString SignerNymID, QString Contract)
{
    bool result = opentxs::OTAPI_Wrap::It()->VerifySignature(SignerNymID.toStdString(),
                                                             Contract.toStdString());
    QJsonObject object{{"VerifySignatureResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::verifyAndRetrieveXMLContents(QString Contract, QString SignerID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->VerifyAndRetrieveXMLContents(Contract.toStdString(),
                                                                                 SignerID.toStdString());
    QJsonObject object{{"ContractXMLContents", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::verifyAccountReceipt(QString NotaryID, QString NymID,
                                              QString AccountID)
{
    bool result = opentxs::OTAPI_Wrap::It()->VerifyAccountReceipt(NotaryID.toStdString(),
                                                                  NymID.toStdString(),
                                                                  AccountID.toStdString());
    QJsonObject object{{"VerifyAccountReceiptResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::setAccountWalletName(QString AccountID, QString SignerNymID,
                                              QString AccountName)
{
    bool result = opentxs::OTAPI_Wrap::It()->SetAccountWallet_Name(AccountID.toStdString(),
                                                                   SignerNymID.toStdString(),
                                                                   AccountName.toStdString());
    QJsonObject object{{"SetAccountWalletNameResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAccountWalletBalance(QString AccountWalletID)
{
    qint64 result = opentxs::OTAPI_Wrap::It()->GetAccountWallet_Balance(AccountWalletID.toStdString());
    QJsonObject object{{"AccountWalletBalance", QString(std::to_string(result).c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAccountWalletType(QString AccountWalletID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetAccountWallet_Type(AccountWalletID.toStdString());
    QJsonObject object{{"AccountWalletType", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAccountWalletInstrumentDefinitionID(QString AccountWalletID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetAccountWallet_InstrumentDefinitionID(AccountWalletID.toStdString());
    QJsonObject object{{"AccountWalletInstrumentDefinitionID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAccountWalletNotaryID(QString AccountWalletID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetAccountWallet_NotaryID(AccountWalletID.toStdString());
    QJsonObject object{{"AccountWalletNotaryID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAccountWalletNymID(QString AccountWalletID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetAccountWallet_NymID(AccountWalletID.toStdString());
    QJsonObject object{{"AccountWalletNymID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::writeCheque(QString NotaryID, qint64 ChequeAmount,
                                     time64_t ValidFrom, time64_t ValidTo,
                                     QString SenderAccountID, QString SenderNymID,
                                     QString ChequeMemo, QString RecipientNymID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->WriteCheque(NotaryID.toStdString(),
                                                                ChequeAmount,
                                                                ValidFrom,
                                                                ValidTo,
                                                                SenderAccountID.toStdString(),
                                                                SenderNymID.toStdString(),
                                                                ChequeMemo.toStdString(),
                                                                RecipientNymID.toStdString());
    QJsonObject object{{"Cheque", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::discardCheque(QString NotaryId, QString NymID,
                                       QString AccountID, QString Cheque)
{
    bool result = opentxs::OTAPI_Wrap::It()->DiscardCheque(NotaryId.toStdString(),
                                                                  NymID.toStdString(),
                                                                  AccountID.toStdString(),
                                                                  Cheque.toStdString());
    QJsonObject object{{"DiscardChequeResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::proposePaymentPlan(QString NotaryID, time64_t ValidFrom,
                                            time64_t ValidTo, QString SenderAccountID,
                                            QString SenderNymID, QString PlanConsideration,
                                            QString RecipientAccountID, QString RecipientNymID,
                                            qint64 InitialPaymentAmount, time64_t InitialPaymentDelay,
                                            qint64 PaymentPlanAmount, time64_t PaymentPlanDelay,
                                            time64_t PaymentPlanPeriod, time64_t PaymentPlanLength,
                                            int MaxPayments)
{
    std::string result = opentxs::OTAPI_Wrap::It()->ProposePaymentPlan(NotaryID.toStdString(),
                                                                       ValidFrom,
                                                                       ValidTo,
                                                                       SenderAccountID.toStdString(),
                                                                       SenderNymID.toStdString(),
                                                                       PlanConsideration.toStdString(),
                                                                       RecipientAccountID.toStdString(),
                                                                       RecipientNymID.toStdString(),
                                                                       InitialPaymentAmount,
                                                                       InitialPaymentDelay,
                                                                       PaymentPlanAmount,
                                                                       PaymentPlanDelay,
                                                                       PaymentPlanPeriod,
                                                                       PaymentPlanLength,
                                                                       MaxPayments);
    QJsonObject object{{"ProposePaymentPlanResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::easyProposePlan(QString NotaryID, QString DateRange,
                                         QString SenderAccountID, QString SenderNymID,
                                         QString PlanConsideration, QString RecipientAccountID,
                                         QString RecipientNymID, QString InitialPayment,
                                         QString PaymentPlan, QString PlanExpiry)
{
    std::string result = opentxs::OTAPI_Wrap::It()->EasyProposePlan(NotaryID.toStdString(),
                                                                    DateRange.toStdString(),
                                                                    SenderAccountID.toStdString(),
                                                                    SenderNymID.toStdString(),
                                                                    PlanConsideration.toStdString(),
                                                                    RecipientAccountID.toStdString(),
                                                                    RecipientNymID.toStdString(),
                                                                    InitialPayment.toStdString(),
                                                                    PaymentPlan.toStdString(),
                                                                    PlanExpiry.toStdString());
    QJsonObject object{{"EasyProposePlanResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::confirmPaymentPlan(QString NotaryID, QString SenderNymID,
                                            QString SenderAccountID, QString RecipientNymID,
                                            QString PaymentPlan)
{
    std::string result = opentxs::OTAPI_Wrap::It()->ConfirmPaymentPlan(NotaryID.toStdString(),
                                                                       SenderNymID.toStdString(),
                                                                       SenderAccountID.toStdString(),
                                                                       RecipientNymID.toStdString(),
                                                                       PaymentPlan.toStdString());
    QJsonObject object{{"ConfirmPaymentPlanResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::createSmartContract(QString SignerNymID, time64_t ValidFrom,
                                             time64_t ValidTo, bool SpecifyAssets,
                                             bool SpecifyParties)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Create_SmartContract(SignerNymID.toStdString(),
                                                                         ValidFrom,
                                                                         ValidTo,
                                                                         SpecifyAssets,
                                                                         SpecifyParties);
    QJsonObject object{{"CreateSmartContractResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractSetDates(QString Contract, QString SignerNymID,
                                               time64_t ValidFrom, time64_t ValidTo)
{
    std::string result = opentxs::OTAPI_Wrap::It()->SmartContract_SetDates(Contract.toStdString(),
                                                                           SignerNymID.toStdString(),
                                                                           ValidFrom,
                                                                           ValidTo);
    QJsonObject object{{"SmartContractSetDatesResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartArePartiesSpecified(QString Contract)
{
    bool result = opentxs::OTAPI_Wrap::It()->Smart_ArePartiesSpecified(Contract.toStdString());
    QJsonObject object{{"SmartArePartiesSpecifiedResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartAreAssetTypesSpecified(QString Contract)
{
    bool result = opentxs::OTAPI_Wrap::It()->Smart_AreAssetTypesSpecified(Contract.toStdString());
    QJsonObject object{{"SmartAreAssetTypesSpecifiedResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractAddBylaw(QString Contract, QString SignerNymID,
                                               QString BylawName)
{
    std::string result = opentxs::OTAPI_Wrap::It()->SmartContract_AddBylaw(Contract.toStdString(),
                                                                           SignerNymID.toStdString(),
                                                                           BylawName.toStdString());
    QJsonObject object{{"SmartContractAddBylawResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractAddClause(QString Contract, QString SignerNymID,
                                                QString BylawName, QString ClauseName,
                                                QString SourceCode)
{
    std::string result = opentxs::OTAPI_Wrap::It()->SmartContract_AddClause(Contract.toStdString(),
                                                                            SignerNymID.toStdString(),
                                                                            BylawName.toStdString(),
                                                                            ClauseName.toStdString(),
                                                                            SourceCode.toStdString());
    QJsonObject object{{"SmartContractAddClauseResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractAddVariable(QString Contract, QString SignerNymID,
                                                  QString BylawName, QString VarName,
                                                  QString VarAccess, QString VarType,
                                                  QString VarValue)
{
    std::string result = opentxs::OTAPI_Wrap::It()->SmartContract_AddVariable(Contract.toStdString(),
                                                                              SignerNymID.toStdString(),
                                                                              BylawName.toStdString(),
                                                                              VarName.toStdString(),
                                                                              VarAccess.toStdString(),
                                                                              VarType.toStdString(),
                                                                              VarValue.toStdString());
    QJsonObject object{{"SmartContractAddVariableResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractAddCallback(QString Contract, QString SignerNymID,
                                                  QString BylawName, QString CallbackName,
                                                  QString ClauseName)
{
    std::string result = opentxs::OTAPI_Wrap::It()->SmartContract_AddCallback(Contract.toStdString(),
                                                                              SignerNymID.toStdString(),
                                                                              BylawName.toStdString(),
                                                                              CallbackName.toStdString(),
                                                                              ClauseName.toStdString());
    QJsonObject object{{"SmartContractAddCallbackResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractAddHook(QString Contract, QString SignerNymID,
                                              QString BylawName, QString HookName,
                                              QString ClauseName)
{
    std::string result = opentxs::OTAPI_Wrap::It()->SmartContract_AddHook(Contract.toStdString(),
                                                                          SignerNymID.toStdString(),
                                                                          BylawName.toStdString(),
                                                                          HookName.toStdString(),
                                                                          ClauseName.toStdString());
    QJsonObject object{{"SmartContractAddHookResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractAddParty(QString Contract, QString SignerNymID,
                                               QString PartyNymID, QString PartyName,
                                               QString AgentName)
{
    std::string result = opentxs::OTAPI_Wrap::It()->SmartContract_AddParty(Contract.toStdString(),
                                                                           SignerNymID.toStdString(),
                                                                           PartyNymID.toStdString(),
                                                                           PartyName.toStdString(),
                                                                           AgentName.toStdString());
    QJsonObject object{{"SmartContractAddPartyResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractAddAccount(QString Contract, QString SignerNymID,
                                                 QString PartyName, QString AccountName,
                                                 QString InstrumentDefinitionID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->SmartContract_AddAccount(Contract.toStdString(),
                                                                             SignerNymID.toStdString(),
                                                                             PartyName.toStdString(),
                                                                             AccountName.toStdString(),
                                                                             InstrumentDefinitionID.toStdString());
    QJsonObject object{{"SmartContractAddAccountResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractRemoveBylaw(QString Contract, QString SignerNymID,
                                                  QString BylawName)
{
    std::string result = opentxs::OTAPI_Wrap::It()->SmartContract_RemoveBylaw(Contract.toStdString(),
                                                                              SignerNymID.toStdString(),
                                                                              BylawName.toStdString());
    QJsonObject object{{"SmartContractRemoveBylawResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractUpdateClause(QString Contract, QString SignerNymID,
                                                   QString BylawName, QString ClauseName,
                                                   QString SourceCode)
{
    std::string result = opentxs::OTAPI_Wrap::It()->SmartContract_UpdateClause(Contract.toStdString(),
                                                                               SignerNymID.toStdString(),
                                                                               BylawName.toStdString(),
                                                                               ClauseName.toStdString(),
                                                                               SourceCode.toStdString());
    QJsonObject object{{"SmartContractUpdateClauseResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractRemoveClause(QString Contract, QString SignerNymID,
                                                   QString BylawName, QString ClauseName)
{
    std::string result = opentxs::OTAPI_Wrap::It()->SmartContract_RemoveClause(Contract.toStdString(),
                                                                               SignerNymID.toStdString(),
                                                                               BylawName.toStdString(),
                                                                               ClauseName.toStdString());
    QJsonObject object{{"SmartContractRemoveClauseResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractRemoveVariable(QString Contract, QString SignerNymID,
                                                     QString BylawName, QString VarName)
{
    std::string result = opentxs::OTAPI_Wrap::It()->SmartContract_RemoveVariable(Contract.toStdString(),
                                                                                 SignerNymID.toStdString(),
                                                                                 BylawName.toStdString(),
                                                                                 VarName.toStdString());
    QJsonObject object{{"SmartContractRemoveVariableResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractRemoveCallback(QString Contract, QString SignerNymID,
                                                     QString BylawName, QString CallbackName)
{
    std::string result = opentxs::OTAPI_Wrap::It()->SmartContract_RemoveCallback(Contract.toStdString(),
                                                                                 SignerNymID.toStdString(),
                                                                                 BylawName.toStdString(),
                                                                                 CallbackName.toStdString());
    QJsonObject object{{"SmartContractRemoveCallbackResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractRemoveHook(QString Contract, QString SignerNymID,
                                                 QString BylawName, QString HookName,
                                                 QString ClauseName)
{
    std::string result = opentxs::OTAPI_Wrap::It()->SmartContract_RemoveHook(Contract.toStdString(),
                                                                             SignerNymID.toStdString(),
                                                                             BylawName.toStdString(),
                                                                             HookName.toStdString(),
                                                                             ClauseName.toStdString());
    QJsonObject object{{"SmartContractRemoveHookResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractRemoveParty(QString Contract, QString SignerNymID,
                                                  QString PartyName)
{
    std::string result = opentxs::OTAPI_Wrap::It()->SmartContract_RemoveParty(Contract.toStdString(),
                                                                              SignerNymID.toStdString(),
                                                                              PartyName.toStdString());
    QJsonObject object{{"SmartContractRemovePartyResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractCountNumbersNeeded(QString Contract, QString AgentName)
{
    int result = opentxs::OTAPI_Wrap::It()->SmartContract_CountNumsNeeded(Contract.toStdString(),
                                                                              AgentName.toStdString());
    QJsonObject object{{"SmartContractNumbersNeeded", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractConfirmAccount(QString Contract, QString SignerNymID,
                                                     QString PartyName, QString AccountName,
                                                     QString AgentName, QString AccountID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->SmartContract_ConfirmAccount(Contract.toStdString(),
                                                                                 SignerNymID.toStdString(),
                                                                                 PartyName.toStdString(),
                                                                                 AccountName.toStdString(),
                                                                                 AgentName.toStdString(),
                                                                                 AccountID.toStdString());
    QJsonObject object{{"SmartContractConfirmAccountResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractConfirmParty(QString Contract, QString PartyName,
                                                   QString NymID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->SmartContract_ConfirmParty(Contract.toStdString(),
                                                                               PartyName.toStdString(),
                                                                               NymID.toStdString());
    QJsonObject object{{"SmartContractConfirmPartyResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartAreAllPartiesConfirmed(QString Contract)
{
    bool result = opentxs::OTAPI_Wrap::It()->Smart_AreAllPartiesConfirmed(Contract.toStdString());
    QJsonObject object{{"SmartAreAllPartiesConfirmedResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartIsPartyConfirmed(QString Contract, QString PartyName)
{
    bool result = opentxs::OTAPI_Wrap::It()->Smart_IsPartyConfirmed(Contract.toStdString(),
                                                                    PartyName.toStdString());
    QJsonObject object{{"SmartIsPartyConfirmedResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartGetPartyCount(QString Contract)
{
    int result = opentxs::OTAPI_Wrap::It()->Smart_GetPartyCount(Contract.toStdString());
    QJsonObject object{{"SmartPartyCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartGetBylawCount(QString Contract)
{
    int result = opentxs::OTAPI_Wrap::It()->Smart_GetBylawCount(Contract.toStdString());
    QJsonObject object{{"SmartBylawCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartGetPartyByIndex(QString Contract, int Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Smart_GetPartyByIndex(Contract.toStdString(),
                                                                          Index);
    QJsonObject object{{"SmartGetPartyByIndexResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartGetBylawByIndex(QString Contract, int Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Smart_GetBylawByIndex(Contract.toStdString(),
                                                                          Index);
    QJsonObject object{{"SmartGetBylawByIndexResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::bylawGetLanguage(QString Contract, QString BylawName)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Bylaw_GetLanguage(Contract.toStdString(),
                                                                      BylawName.toStdString());
    QJsonObject object{{"BylawLanguage", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::bylawGetClauseCount(QString Contract, QString BylawName)
{
    int result = opentxs::OTAPI_Wrap::It()->Bylaw_GetClauseCount(Contract.toStdString(),
                                                                     BylawName.toStdString());
    QJsonObject object{{"BylawClauseCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::bylawGetVariableCount(QString Contract, QString BylawName)
{
    int result = opentxs::OTAPI_Wrap::It()->Bylaw_GetVariableCount(Contract.toStdString(),
                                                                       BylawName.toStdString());
    QJsonObject object{{"BylawVariableCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::bylawGetHookCount(QString Contract, QString BylawName)
{
    int result = opentxs::OTAPI_Wrap::It()->Bylaw_GetHookCount(Contract.toStdString(),
                                                                   BylawName.toStdString());
    QJsonObject object{{"BylawHookCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::bylawGetCallbackCount(QString Contract, QString BylawName)
{
    int result = opentxs::OTAPI_Wrap::It()->Bylaw_GetCallbackCount(Contract.toStdString(),
                                                                       BylawName.toStdString());
    QJsonObject object{{"BylawCallbackCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::clauseGetNameByIndex(QString Contract, QString BylawName,
                                              int Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Clause_GetNameByIndex(Contract.toStdString(),
                                                                          BylawName.toStdString(),
                                                                          Index);
    QJsonObject object{{"ClauseGetNameByIndexResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::clauseGetContents(QString Contract, QString BylawName,
                                           QString ClauseName)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Clause_GetContents(Contract.toStdString(),
                                                                       BylawName.toStdString(),
                                                                       ClauseName.toStdString());
    QJsonObject object{{"ClauseContents", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::variableGetNameByIndex(QString Contract, QString BylawName,
                                                int Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Variable_GetNameByIndex(Contract.toStdString(),
                                                                            BylawName.toStdString(),
                                                                            Index);
    QJsonObject object{{"VariableGetNameByIndexResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::variableGetType(QString Contract, QString BylawName,
                                         QString VariableName)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Variable_GetType(Contract.toStdString(),
                                                                     BylawName.toStdString(),
                                                                     VariableName.toStdString());
    QJsonObject object{{"VariableType", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::variableGetAccess(QString Contract, QString BylawName,
                                           QString VariableName)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Variable_GetAccess(Contract.toStdString(),
                                                                       BylawName.toStdString(),
                                                                       VariableName.toStdString());
    QJsonObject object{{"VariableAccess", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::variableGetContents(QString Contract, QString BylawName,
                                             QString VariableName)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Variable_GetContents(Contract.toStdString(),
                                                                         BylawName.toStdString(),
                                                                         VariableName.toStdString());
    QJsonObject object{{"VariableContents", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::hookGetNameByIndex(QString Contract, QString BylawName,
                                            int Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Hook_GetNameByIndex(Contract.toStdString(),
                                                                        BylawName.toStdString(),
                                                                        Index);
    QJsonObject object{{"HookGetNameByIndexResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::hookGetClauseCount(QString Contract, QString BylawName,
                                            QString HookName)
{
    int result = opentxs::OTAPI_Wrap::It()->Hook_GetClauseCount(Contract.toStdString(),
                                                                    BylawName.toStdString(),
                                                                    HookName.toStdString());
    QJsonObject object{{"HookClauseCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::hookGetClauseAtIndex(QString Contract, QString BylawName,
                                              QString HookName, int Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Hook_GetClauseAtIndex(Contract.toStdString(),
                                                                          BylawName.toStdString(),
                                                                          HookName.toStdString(),
                                                                          Index);
    QJsonObject object{{"HookGetClauseAtIndexResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::callbackGetNameByIndex(QString Contract, QString BylawName,
                                                int Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Callback_GetNameByIndex(Contract.toStdString(),
                                                                            BylawName.toStdString(),
                                                                            Index);
    QJsonObject object{{"CallbackGetNameByIndexResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::callbackGetClause(QString Contract, QString BylawName,
                                           QString ClauseName)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Callback_GetClause(Contract.toStdString(),
                                                                       BylawName.toStdString(),
                                                                       ClauseName.toStdString());
    QJsonObject object{{"CallbackGetClauseResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::partyGetAccountCount(QString Contract, QString PartyName)
{
    int result = opentxs::OTAPI_Wrap::It()->Party_GetAcctCount(Contract.toStdString(),
                                                                   PartyName.toStdString());
    QJsonObject object{{"PartyAccountCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::partyGetAgentCount(QString Contract, QString PartyName)
{
    int result = opentxs::OTAPI_Wrap::It()->Party_GetAgentCount(Contract.toStdString(),
                                                                   PartyName.toStdString());
    QJsonObject object{{"PartyAgentCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::partyGetID(QString Contract, QString PartyName)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Party_GetID(Contract.toStdString(),
                                                                PartyName.toStdString());
    QJsonObject object{{"PartyID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::partyGetAccountNameByIndex(QString Contract, QString PartyName,
                                                    int Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Party_GetAcctNameByIndex(Contract.toStdString(),
                                                                             PartyName.toStdString(),
                                                                             Index);
    QJsonObject object{{"PartyGetAccountNameByIndexResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::partyGetAccountID(QString Contract, QString PartyName,
                                           QString AccountName)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Party_GetAcctID(Contract.toStdString(),
                                                                    PartyName.toStdString(),
                                                                    AccountName.toStdString());
    QJsonObject object{{"PartyGetAccountIDResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::partyGetAccountInstrumentDefinitionID(QString Contract, QString PartyName,
                                                               QString AccountName)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Party_GetAcctInstrumentDefinitionID(Contract.toStdString(),
                                                                                        PartyName.toStdString(),
                                                                                        AccountName.toStdString());
    QJsonObject object{{"PartyGetAccountInstrumentDefinitionIDResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::partyGetAccountAgentName(QString Contract, QString PartyName,
                                                  QString AccountName)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Party_GetAcctAgentName(Contract.toStdString(),
                                                                           PartyName.toStdString(),
                                                                           AccountName.toStdString());
    QJsonObject object{{"PartyAccountAgentName", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::partyGetAgentNameByIndex(QString Contract, QString PartyName,
                                                  int Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Party_GetAgentNameByIndex(Contract.toStdString(),
                                                                              PartyName.toStdString(),
                                                                              Index);
    QJsonObject object{{"PartyGetAgentNameByIndexResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::partyGetAgentID(QString Contract, QString PartyName,
                                         QString AgentName)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Party_GetAgentID(Contract.toStdString(),
                                                                     PartyName.toStdString(),
                                                                     AgentName.toStdString());
    QJsonObject object{{"PartyGetAgentIDResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::activateSmartContract(QString NotaryID, QString NymID,
                                               QString SmartContract)
{
    int result = opentxs::OTAPI_Wrap::It()->activateSmartContract(NotaryID.toStdString(),
                                                                  NymID.toStdString(),
                                                                  SmartContract.toStdString());
    QJsonObject object{{"ActivateSmartContractResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::triggerClause(QString NotaryID, QString NymID,
                                       qint64 TransactionNumber, QString ClauseName,
                                       QString Parameter)
{
    int result = opentxs::OTAPI_Wrap::It()->triggerClause(NotaryID.toStdString(),
                                                              NymID.toStdString(),
                                                              TransactionNumber,
                                                              ClauseName.toStdString(),
                                                              Parameter.toStdString());
    QJsonObject object{{"TriggerClauseResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageHarvestTransactionNumbers(QString Message, QString NymID,
                                                          bool HarvestingForRetry, bool ReplyWasSuccess,
                                                          bool ReplyWasFailure, bool TransactionWasSuccess,
                                                          bool TransactionWasFailure)
{
    bool result = opentxs::OTAPI_Wrap::It()->Msg_HarvestTransactionNumbers(Message.toStdString(),
                                                                           NymID.toStdString(),
                                                                           HarvestingForRetry,
                                                                           ReplyWasSuccess,
                                                                           ReplyWasFailure,
                                                                           TransactionWasSuccess,
                                                                           TransactionWasFailure);
    QJsonObject object{{"MessageHarvestTransactionNumbersResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadPubkeyEncryption(QString NymID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->LoadPubkey_Encryption(NymID.toStdString());
    QJsonObject object{{"LoadPubkeyEncryptionResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadPubkeySigning(QString NymID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->LoadPubkey_Signing(NymID.toStdString());
    QJsonObject object{{"LoadPubkeySigningResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadUserPubkeyEncryption(QString NymID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->LoadUserPubkey_Encryption(NymID.toStdString());
    QJsonObject object{{"LoadUserPubkeyEncryptionResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadUserPubkeySigning(QString NymID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->LoadUserPubkey_Signing(NymID.toStdString());
    QJsonObject object{{"LoadUserPubkeySigningResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::verifyUserPrivateKey(QString NymID)
{
    bool result = opentxs::OTAPI_Wrap::It()->VerifyUserPrivateKey(NymID.toStdString());
    QJsonObject object{{"VerifyUserPrivateKeyResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::mintIsStillGood(QString NotaryID, QString InstrumentDefinitionID)
{
    bool result = opentxs::OTAPI_Wrap::It()->Mint_IsStillGood(NotaryID.toStdString(),
                                                              InstrumentDefinitionID.toStdString());
    QJsonObject object{{"MintIsStillGoodResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadMint(QString NotaryID, QString InstrumentDefinitionID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->LoadMint(NotaryID.toStdString(),
                                                             InstrumentDefinitionID.toStdString());
    QJsonObject object{{"LoadMintResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadAssetContract(QString InstrumentDefinitionID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->LoadAssetContract(InstrumentDefinitionID.toStdString());
    QJsonObject object{{"LoadAssetContractResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadServerContract(QString NotaryID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->LoadServerContract(NotaryID.toStdString());
    QJsonObject object{{"LoadServerContractResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadAssetAccount(QString NotaryID, QString NymID,
                                          QString AccountID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->LoadAssetAccount(NotaryID.toStdString(),
                                                                     NymID.toStdString(),
                                                                     AccountID.toStdString());
    QJsonObject object{{"LoadAssetAccountResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::nymboxGetReplyNotice(QString NotaryID, QString NymID,
                                              qint64 RequestNumber)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Nymbox_GetReplyNotice(NotaryID.toStdString(),
                                                                          NymID.toStdString(),
                                                                          RequestNumber);
    QJsonObject object{{"NymboxGetReplyNoticeResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::haveAlreadySeenReply(QString NotaryID, QString NymID,
                                              qint64 RequestNumber)
{
    bool result = opentxs::OTAPI_Wrap::It()->HaveAlreadySeenReply(NotaryID.toStdString(),
                                                                  NymID.toStdString(),
                                                                  RequestNumber);
    QJsonObject object{{"HaveAlreadySeenReplyResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadNymbox(QString NotaryID, QString NymID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->LoadNymbox(NotaryID.toStdString(),
                                                               NymID.toStdString());
    QJsonObject object{{"LoadNymboxResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadNymboxNoVerify(QString NotaryID, QString NymID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->LoadNymboxNoVerify(NotaryID.toStdString(),
                                                                       NymID.toStdString());
    QJsonObject object{{"LoadNymboxNoVerifyResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadInbox(QString NotaryID, QString NymID,
                                   QString AccountID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->LoadInbox(NotaryID.toStdString(),
                                                              NymID.toStdString(),
                                                              AccountID.toStdString());
    QJsonObject object{{"LoadInboxResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadInboxNoVerify(QString NotaryID, QString NymID,
                                           QString AccountID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->LoadInboxNoVerify(NotaryID.toStdString(),
                                                                      NymID.toStdString(),
                                                                      AccountID.toStdString());
    QJsonObject object{{"LoadInboxResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadOutbox(QString NotaryID, QString NymID,
                                   QString AccountID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->LoadOutbox(NotaryID.toStdString(),
                                                               NymID.toStdString(),
                                                               AccountID.toStdString());
    QJsonObject object{{"LoadOutboxResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadOutboxNoVerify(QString NotaryID, QString NymID,
                                           QString AccountID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->LoadOutboxNoVerify(NotaryID.toStdString(),
                                                                       NymID.toStdString(),
                                                                       AccountID.toStdString());
    QJsonObject object{{"LoadOutboxNoVerifyResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadPaymentInbox(QString NotaryID, QString NymID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->LoadPaymentInbox(NotaryID.toStdString(),
                                                                     NymID.toStdString());
    QJsonObject object{{"LoadPaymentInboxResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadPaymentInboxNoVerify(QString NotaryID, QString NymID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->LoadPaymentInboxNoVerify(NotaryID.toStdString(),
                                                                             NymID.toStdString());
    QJsonObject object{{"LoadPaymentInboxNoVerifyResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadRecordBox(QString NotaryID, QString NymID,
                                       QString AccountID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->LoadRecordBox(NotaryID.toStdString(),
                                                                  NymID.toStdString(),
                                                                  AccountID.toStdString());
    QJsonObject object{{"LoadRecordBoxResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadRecordBoxNoVerify(QString NotaryID, QString NymID,
                                               QString AccountID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->LoadRecordBoxNoVerify(NotaryID.toStdString(),
                                                                          NymID.toStdString(),
                                                                          AccountID.toStdString());
    QJsonObject object{{"LoadRecordBoxNoVerifyResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadExpiredBox(QString NotaryID, QString NymID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->LoadExpiredBox(NotaryID.toStdString(),
                                                                   NymID.toStdString());
    QJsonObject object{{"LoadExpiredBoxResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadExpiredBoxNoVerify(QString NotaryID, QString NymID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->LoadExpiredBoxNoVerify(NotaryID.toStdString(),
                                                                           NymID.toStdString());
    QJsonObject object{{"LoadExpiredBoxNoVerifyResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::recordPayment(QString NotaryID, QString NymID,
                                       bool IsInbox, int Index,
                                       bool SaveCopy)
{
    bool result = opentxs::OTAPI_Wrap::It()->RecordPayment(NotaryID.toStdString(),
                                                                  NymID.toStdString(),
                                                                  IsInbox,
                                                                  Index,
                                                                  SaveCopy);
    QJsonObject object{{"RecordPaymentResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::clearRecord(QString NotaryID, QString NymID,
                                     QString AccountID, int Index,
                                     bool ClearAll)
{
    bool result = opentxs::OTAPI_Wrap::It()->ClearRecord(NotaryID.toStdString(),
                                                         NymID.toStdString(),
                                                         AccountID.toStdString(),
                                                         Index,
                                                         ClearAll);
    QJsonObject object{{"ClearRecordResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::clearExpired(QString NotaryID, QString NymID,
                                      int Index, bool ClearAll)
{
    bool result = opentxs::OTAPI_Wrap::It()->ClearExpired(NotaryID.toStdString(),
                                                          NymID.toStdString(),
                                                          Index,
                                                          ClearAll);
    QJsonObject object{{"ClearExpiredResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::ledgerGetCount(QString NotaryID, QString NymID,
                                        QString AccountID, QString Ledger)
{
    int result = opentxs::OTAPI_Wrap::It()->Ledger_GetCount(NotaryID.toStdString(),
                                                            NymID.toStdString(),
                                                            AccountID.toStdString(),
                                                            Ledger.toStdString());
    QJsonObject object{{"LedgerCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::ledgerCreateResponse(QString NotaryID, QString NymID,
                                              QString AccountID, QString OriginalLedger)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Ledger_CreateResponse(NotaryID.toStdString(),
                                                                          NymID.toStdString(),
                                                                          AccountID.toStdString(),
                                                                          OriginalLedger.toStdString());
    QJsonObject object{{"LedgerResponse", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::ledgerGetTransactionByIndex(QString NotaryID, QString NymID,
                                                     QString AccountID, QString Ledger,
                                                     int Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Ledger_GetTransactionByIndex(NotaryID.toStdString(),
                                                                                 NymID.toStdString(),
                                                                                 AccountID.toStdString(),
                                                                                 Ledger.toStdString(),
                                                                                 Index);
    QJsonObject object{{"LedgerGetTransactionByIndexResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::ledgerGetTransactionByID(QString NotaryID, QString NymID,
                                                  QString AccountID, QString Ledger,
                                                  qint64 TransactionNumber)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Ledger_GetTransactionByIndex(NotaryID.toStdString(),
                                                                                 NymID.toStdString(),
                                                                                 AccountID.toStdString(),
                                                                                 Ledger.toStdString(),
                                                                                 TransactionNumber);
    QJsonObject object{{"LedgerGetTransactionByIDResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::ledgerGetInstrument(QString NotaryID, QString NymID,
                                             QString AccountID, QString Ledger,
                                             int Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Ledger_GetInstrument(NotaryID.toStdString(),
                                                                         NymID.toStdString(),
                                                                         AccountID.toStdString(),
                                                                         Ledger.toStdString(),
                                                                         Index);
    QJsonObject object{{"LedgerGetInstrumentResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::ledgerGetTransactionIDByIndex(QString NotaryID, QString NymID,
                                                       QString AccountID, QString Ledger,
                                                       int Index)
{
    qint64 result = opentxs::OTAPI_Wrap::It()->Ledger_GetTransactionIDByIndex(NotaryID.toStdString(),
                                                                              NymID.toStdString(),
                                                                              AccountID.toStdString(),
                                                                              Ledger.toStdString(),
                                                                              Index);
    QJsonObject object{{"LedgerGetTransactionIDByIndexResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::ledgerAddTransaction(QString NotaryID, QString NymID,
                                              QString AccountID, QString Ledger,
                                              QString Transaction)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Ledger_AddTransaction(NotaryID.toStdString(),
                                                                          NymID.toStdString(),
                                                                          AccountID.toStdString(),
                                                                          Ledger.toStdString(),
                                                                          Transaction.toStdString());
    QJsonObject object{{"LedgerAddTransactionResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionCreateResponse(QString NotaryID, QString NymID,
                                                   QString AccountID, QString Ledger,
                                                   QString Transaction, bool DoIAccept)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Transaction_CreateResponse(NotaryID.toStdString(),
                                                                               NymID.toStdString(),
                                                                               AccountID.toStdString(),
                                                                               Ledger.toStdString(),
                                                                               Transaction.toStdString(),
                                                                               DoIAccept);
    QJsonObject object{{"TransactionCreateResponseResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::ledgerFinalizeResponse(QString NotaryID, QString NymID,
                                                QString AccountID, QString Ledger)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Ledger_FinalizeResponse(NotaryID.toStdString(),
                                                                            NymID.toStdString(),
                                                                            AccountID.toStdString(),
                                                                            Ledger.toStdString());
    QJsonObject object{{"LedgerFinalizeResponseResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionGetVoucher(QString NotaryID, QString NymID,
                                               QString AccountID, QString Transaction)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Transaction_GetVoucher(NotaryID.toStdString(),
                                                                           NymID.toStdString(),
                                                                           AccountID.toStdString(),
                                                                           Transaction.toStdString());
    QJsonObject object{{"TransactionVoucher", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionGetSenderNymID(QString NotaryID, QString NymID,
                                                   QString AccountID, QString Transaction)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Transaction_GetSenderNymID(NotaryID.toStdString(),
                                                                               NymID.toStdString(),
                                                                               AccountID.toStdString(),
                                                                               Transaction.toStdString());
    QJsonObject object{{"TransactionSenderNymID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionGetRecipientNymID(QString NotaryID, QString NymID,
                                                      QString AccountID, QString Transaction)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Transaction_GetRecipientNymID(NotaryID.toStdString(),
                                                                                  NymID.toStdString(),
                                                                                  AccountID.toStdString(),
                                                                                  Transaction.toStdString());
    QJsonObject object{{"TransactionRecipientNymID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionGetSenderAccountID(QString NotaryID, QString NymID,
                                                       QString AccountID, QString Transaction)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Transaction_GetSenderAcctID(NotaryID.toStdString(),
                                                                                NymID.toStdString(),
                                                                                AccountID.toStdString(),
                                                                                Transaction.toStdString());
    QJsonObject object{{"TransactionSenderAccountID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionGetRecipientAccountID(QString NotaryID, QString NymID,
                                                          QString AccountID, QString Transaction)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Transaction_GetRecipientAcctID(NotaryID.toStdString(),
                                                                                   NymID.toStdString(),
                                                                                   AccountID.toStdString(),
                                                                                   Transaction.toStdString());
    QJsonObject object{{"TransactionRecipientAccountID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::pendingGetNote(QString NotaryID, QString NymID,
                                        QString AccountID, QString Transaction)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Pending_GetNote(NotaryID.toStdString(),
                                                                    NymID.toStdString(),
                                                                    AccountID.toStdString(),
                                                                    Transaction.toStdString());
    QJsonObject object{{"PendingNote", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionGetAmount(QString NotaryID, QString NymID,
                                              QString AccountID, QString Transaction)
{
    qint64 result = opentxs::OTAPI_Wrap::It()->Transaction_GetAmount(NotaryID.toStdString(),
                                                                     NymID.toStdString(),
                                                                     AccountID.toStdString(),
                                                                     Transaction.toStdString());
    QJsonObject object{{"TransactionAmount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionGetDisplayReferenceToNumber(QString NotaryID, QString NymID,
                                                                QString AccountID, QString Transaction)
{
    qint64 result = opentxs::OTAPI_Wrap::It()->Transaction_GetDisplayReferenceToNum(NotaryID.toStdString(),
                                                                                    NymID.toStdString(),
                                                                                    AccountID.toStdString(),
                                                                                    Transaction.toStdString());
    QJsonObject object{{"TransactionDisplayReferenceToNumber", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionGetType(QString NotaryID, QString NymID,
                                            QString AccountID, QString Transaction)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Transaction_GetType(NotaryID.toStdString(),
                                                                        NymID.toStdString(),
                                                                        AccountID.toStdString(),
                                                                        Transaction.toStdString());
    QJsonObject object{{"TransactionType", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::replyNoticeGetRequestNumber(QString NotaryID, QString NymID,
                                                     QString Transaction)
{
    qint64 result = opentxs::OTAPI_Wrap::It()->ReplyNotice_GetRequestNum(NotaryID.toStdString(),
                                                                         NymID.toStdString(),
                                                                         Transaction.toStdString());
    QJsonObject object{{"ReplyNoticeRequestNumber", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionGetDateSigned(QString NotaryID, QString NymID,
                                                  QString AccountID, QString Transaction)
{
    time64_t result = opentxs::OTAPI_Wrap::It()->Transaction_GetDateSigned(NotaryID.toStdString(),
                                                                           NymID.toStdString(),
                                                                           AccountID.toStdString(),
                                                                           Transaction.toStdString());
    QJsonObject object{{"TransactionDateSigned", QString(std::to_string(result).c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionGetSuccess(QString NotaryID, QString NymID,
                                               QString AccountID, QString Transaction)
{
    bool result = opentxs::OTAPI_Wrap::It()->Transaction_GetSuccess(NotaryID.toStdString(),
                                                                    NymID.toStdString(),
                                                                    AccountID.toStdString(),
                                                                    Transaction.toStdString());
    QJsonObject object{{"TransactionSuccess", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionIsCanceled(QString NotaryID, QString NymID,
                                               QString AccountID, QString Transaction)
{
    bool result = opentxs::OTAPI_Wrap::It()->Transaction_IsCanceled(NotaryID.toStdString(),
                                                                    NymID.toStdString(),
                                                                    AccountID.toStdString(),
                                                                    Transaction.toStdString());
    QJsonObject object{{"TransactionIsCanceled", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionGetBalanceAgreementSuccess(QString NotaryID, QString NymID,
                                                               QString AccountID, QString Transaction)
{
    bool result = opentxs::OTAPI_Wrap::It()->Transaction_GetBalanceAgreementSuccess(NotaryID.toStdString(),
                                                                                    NymID.toStdString(),
                                                                                    AccountID.toStdString(),
                                                                                    Transaction.toStdString());
    QJsonObject object{{"TransactionBalanceAgreementSuccess", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetBalanceAgreementSuccess(QString NotaryID, QString NymID,
                                                           QString AccountID, QString Message)
{
    bool result = opentxs::OTAPI_Wrap::It()->Message_GetBalanceAgreementSuccess(NotaryID.toStdString(),
                                                                                NymID.toStdString(),
                                                                                AccountID.toStdString(),
                                                                                Message.toStdString());
    QJsonObject object{{"MessageBalanceAgreementSuccess", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::savePurse(QString NotaryID, QString InstrumentDefinitionID,
                                   QString NymID, QString Purse)
{
    bool result = opentxs::OTAPI_Wrap::It()->SavePurse(NotaryID.toStdString(),
                                                       InstrumentDefinitionID.toStdString(),
                                                       NymID.toStdString(),
                                                       Purse.toStdString());
    QJsonObject object{{"SavePurseResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadPurse(QString NotaryID, QString InstrumentDefinitionID,
                                   QString NymID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->LoadPurse(NotaryID.toStdString(),
                                                       InstrumentDefinitionID.toStdString(),
                                                       NymID.toStdString());
    QJsonObject object{{"Purse", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::purseGetTotalValue(QString NotaryID, QString InstrumentDefinitionID,
                                            QString Purse)
{
    qint64 result = opentxs::OTAPI_Wrap::It()->Purse_GetTotalValue(NotaryID.toStdString(),
                                                                   InstrumentDefinitionID.toStdString(),
                                                                   Purse.toStdString());
    QJsonObject object{{"PurseTotalValue", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::purseCount(QString NotaryID, QString InstrumentDefinitionID,
                                    QString Purse)
{
    int result = opentxs::OTAPI_Wrap::It()->Purse_Count(NotaryID.toStdString(),
                                                           InstrumentDefinitionID.toStdString(),
                                                           Purse.toStdString());
    QJsonObject object{{"PurseCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::purseHasPassword(QString NotaryID, QString Purse)
{
    bool result = opentxs::OTAPI_Wrap::It()->Purse_HasPassword(NotaryID.toStdString(),
                                                               Purse.toStdString());
    QJsonObject object{{"PurseHasPassword", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::createPurse(QString NotaryID, QString InstrumentDefinitionID,
                                     QString OwnerID, QString SignerID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->CreatePurse(NotaryID.toStdString(),
                                                                InstrumentDefinitionID.toStdString(),
                                                                OwnerID.toStdString(),
                                                                SignerID.toStdString());
    QJsonObject object{{"Purse", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::createPursePassphrase(QString NotaryID, QString InstrumentDefinitionID,
                                               QString SignerID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->CreatePurse_Passphrase(NotaryID.toStdString(),
                                                                           InstrumentDefinitionID.toStdString(),
                                                                           SignerID.toStdString());
    QJsonObject object{{"PursePassphrase", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::pursePeek(QString NotaryID, QString InstrumentDefinitionID,
                                   QString OwnerID, QString Purse)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Purse_Peek(NotaryID.toStdString(),
                                                               InstrumentDefinitionID.toStdString(),
                                                               OwnerID.toStdString(),
                                                               Purse.toStdString());
    QJsonObject object{{"PursePeek", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::pursePop(QString NotaryID, QString InstrumentDefinitionID,
                                  QString OwnerOrSignerID, QString Purse)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Purse_Pop(NotaryID.toStdString(),
                                                              InstrumentDefinitionID.toStdString(),
                                                              OwnerOrSignerID.toStdString(),
                                                              Purse.toStdString());
    QJsonObject object{{"PursePop", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::purseEmpty(QString NotaryID, QString InstrumentDefinitionID,
                                    QString SignerID, QString Purse)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Purse_Empty(NotaryID.toStdString(),
                                                                InstrumentDefinitionID.toStdString(),
                                                                SignerID.toStdString(),
                                                                Purse.toStdString());
    QJsonObject object{{"PurseEmpty", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::pursePush(QString NotaryID, QString InstrumentDefinitionID,
                                   QString SignerID, QString OwnerID,
                                   QString Purse, QString Token)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Purse_Push(NotaryID.toStdString(),
                                                               InstrumentDefinitionID.toStdString(),
                                                               SignerID.toStdString(),
                                                               OwnerID.toStdString(),
                                                               Purse.toStdString(),
                                                               Token.toStdString());
    QJsonObject object{{"PursePurse", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletImportPurse(QString NotaryID, QString InstrumentDefinitionID,
                                           QString NymID, QString Purse)
{
    bool result = opentxs::OTAPI_Wrap::It()->Wallet_ImportPurse(NotaryID.toStdString(),
                                                                InstrumentDefinitionID.toStdString(),
                                                                NymID.toStdString(),
                                                                Purse.toStdString());
    QJsonObject object{{"WalletImportPurseResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::exchangePurse(QString NotaryID, QString InstrumentDefinitionID,
                                       QString NymID, QString Purse)
{
    int result = opentxs::OTAPI_Wrap::It()->exchangePurse(NotaryID.toStdString(),
                                                          InstrumentDefinitionID.toStdString(),
                                                          NymID.toStdString(),
                                                          Purse.toStdString());
    QJsonObject object{{"ExchangePurseResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::tokenChangeOwner(QString NotaryID, QString InstrumentDefinitionID,
                                          QString Token, QString SignerNymID,
                                          QString OldOwner, QString NewOwner)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Token_ChangeOwner(NotaryID.toStdString(),
                                                                      InstrumentDefinitionID.toStdString(),
                                                                      Token.toStdString(),
                                                                      SignerNymID.toStdString(),
                                                                      OldOwner.toStdString(),
                                                                      NewOwner.toStdString());
    QJsonObject object{{"TokenChangerOwnerResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::tokenGetID(QString NotaryID, QString InstrumentDefinitionID,
                                    QString Token)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Token_GetID(NotaryID.toStdString(),
                                                                InstrumentDefinitionID.toStdString(),
                                                                Token.toStdString());
    QJsonObject object{{"TokenID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::tokenGetDenomination(QString NotaryID, QString InstrumentDefinitionID,
                                              QString Token)
{
    qint64 result = opentxs::OTAPI_Wrap::It()->Token_GetDenomination(NotaryID.toStdString(),
                                                                     InstrumentDefinitionID.toStdString(),
                                                                     Token.toStdString());
    QJsonObject object{{"TokenDenomination", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::tokenGetSeries(QString NotaryID, QString InstrumentDefinitionID,
                                        QString Token)
{
    int result = opentxs::OTAPI_Wrap::It()->Token_GetSeries(NotaryID.toStdString(),
                                                            InstrumentDefinitionID.toStdString(),
                                                            Token.toStdString());
    QJsonObject object{{"TokenSeries", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::tokenGetValidFrom(QString NotaryID, QString InstrumentDefinitionID,
                                           QString Token)
{
    time64_t result = opentxs::OTAPI_Wrap::It()->Token_GetValidFrom(NotaryID.toStdString(),
                                                                    InstrumentDefinitionID.toStdString(),
                                                                    Token.toStdString());
    QJsonObject object{{"TokenValidFrom", QString(std::to_string(result).c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::tokenGetValidTo(QString NotaryID, QString InstrumentDefinitionID,
                                         QString Token)
{
    time64_t result = opentxs::OTAPI_Wrap::It()->Token_GetValidTo(NotaryID.toStdString(),
                                                                  InstrumentDefinitionID.toStdString(),
                                                                  Token.toStdString());
    QJsonObject object{{"TokenValidTo", QString(std::to_string(result).c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::tokenGetInstrumentDefinitionID(QString Token)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Token_GetInstrumentDefinitionID(Token.toStdString());
    QJsonObject object{{"TokenInstrumentDefinitionID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::tokenGetNotaryID(QString Token)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Token_GetNotaryID(Token.toStdString());
    QJsonObject object{{"TokenNotaryID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::isBasketCurrency(QString InstrumentDefinitionID)
{
    bool result = opentxs::OTAPI_Wrap::It()->IsBasketCurrency(InstrumentDefinitionID.toStdString());
    QJsonObject object{{"IsBasketCurrency", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::basketGetMemberCount(QString InstrumentDefinitionID)
{
    bool result = opentxs::OTAPI_Wrap::It()->Basket_GetMemberCount(InstrumentDefinitionID.toStdString());
    QJsonObject object{{"BasketMemberCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::basketGetMemberType(QString BasketInstrumentDefinitionID, int Index)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Basket_GetMemberType(BasketInstrumentDefinitionID.toStdString(),
                                                                         Index);
    QJsonObject object{{"BasketMemberType", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::basketGetMinimumTransferAmount(QString BasketInstrumentDefinitionID)
{
    qint64 result = opentxs::OTAPI_Wrap::It()->Basket_GetMinimumTransferAmount(BasketInstrumentDefinitionID.toStdString());
    QJsonObject object{{"BasketMinimumTransferAmount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::basketGetMemberMinimumTransferAmount(QString BasketInstrumentDefinitionID, int Index)
{
    qint64 result = opentxs::OTAPI_Wrap::It()->Basket_GetMemberMinimumTransferAmount(BasketInstrumentDefinitionID.toStdString(),
                                                                                     Index);
    QJsonObject object{{"BasketMemberMinimumTransferAmount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::pingNotary(QString NotaryID, QString NymID)
{
    int result = opentxs::OTAPI_Wrap::It()->pingNotary(NotaryID.toStdString(),
                                                       NymID.toStdString());
    QJsonObject object{{"PingNotaryResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::registerNym(QString NotaryID, QString NymID)
{
    int result = opentxs::OTAPI_Wrap::It()->registerNym(NotaryID.toStdString(),
                                                        NymID.toStdString());
    QJsonObject object{{"RegisterNymResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::unregisterNym(QString NotaryID, QString NymID)
{
    int result = opentxs::OTAPI_Wrap::It()->unregisterNym(NotaryID.toStdString(),
                                                          NymID.toStdString());
    QJsonObject object{{"UnregisterNymResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetUsageCredits(QString Message)
{
    qint64 result = opentxs::OTAPI_Wrap::It()->Message_GetUsageCredits(Message.toStdString());
    QJsonObject object{{"MessageUsageCredits", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::usageCredits(QString NotaryID, QString NymID,
                                      QString NymIDCheck, qint64 Adjustment)
{
    int result = opentxs::OTAPI_Wrap::It()->usageCredits(NotaryID.toStdString(),
                                                         NymID.toStdString(),
                                                         NymIDCheck.toStdString(),
                                                         Adjustment);
    QJsonObject object{{"MessageUsageCredits", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::checkNym(QString NotaryID, QString NymID,
                                  QString NymIDCheck)
{
    int result = opentxs::OTAPI_Wrap::It()->checkNym(NotaryID.toStdString(),
                                                     NymID.toStdString(),
                                                     NymIDCheck.toStdString());
    QJsonObject object{{"CheckNymResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::sendNymMessage(QString NotaryID, QString NymID,
                                        QString NymIDRecipient, QString RecipientPubkey,
                                        QString Message)
{
    int result = opentxs::OTAPI_Wrap::It()->sendNymMessage(NotaryID.toStdString(),
                                                           NymID.toStdString(),
                                                           NymIDRecipient.toStdString(),
                                                           RecipientPubkey.toStdString(),
                                                           Message.toStdString());
    QJsonObject object{{"SendNymMessageResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::sendNymInstrument(QString NotaryID, QString NymID,
                                           QString NymIDRecipient, QString RecipientPubkey,
                                           QString Instrument, QString InstrumentForSender)
{
    int result = opentxs::OTAPI_Wrap::It()->sendNymInstrument(NotaryID.toStdString(),
                                                              NymID.toStdString(),
                                                              NymIDRecipient.toStdString(),
                                                              RecipientPubkey.toStdString(),
                                                              Instrument.toStdString(),
                                                              InstrumentForSender.toStdString());
    QJsonObject object{{"SendNymInstrumentResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getRequestNumber(QString NotaryID, QString NymID)
{
    int result = opentxs::OTAPI_Wrap::It()->getRequestNumber(NotaryID.toStdString(),
                                                             NymID.toStdString());
    QJsonObject object{{"RequestNumber", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::registerInstrumentDefinition(QString NotaryID, QString NymID,
                                                      QString Contract)
{
    int result = opentxs::OTAPI_Wrap::It()->registerInstrumentDefinition(NotaryID.toStdString(),
                                                                         NymID.toStdString(),
                                                                         Contract.toStdString());
    QJsonObject object{{"RegisterInstrumentDefinitionResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getInstrumentDefinition(QString NotaryID, QString NymID,
                                                 QString InstrumentDefinitionID)
{
    int result = opentxs::OTAPI_Wrap::It()->getInstrumentDefinition(NotaryID.toStdString(),
                                                                    NymID.toStdString(),
                                                                    InstrumentDefinitionID.toStdString());
    QJsonObject object{{"InstrumentDefinition", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getMint(QString NotaryID, QString NymID,
                                 QString InstrumentDefinitionID)
{
    int result = opentxs::OTAPI_Wrap::It()->getMint(NotaryID.toStdString(),
                                                    NymID.toStdString(),
                                                    InstrumentDefinitionID.toStdString());
    QJsonObject object{{"Mint", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::registerAccount(QString NotaryID, QString NymID,
                                         QString InstrumentDefinitionID)
{
    int result = opentxs::OTAPI_Wrap::It()->registerAccount(NotaryID.toStdString(),
                                                            NymID.toStdString(),
                                                            InstrumentDefinitionID.toStdString());
    QJsonObject object{{"RegisterAccountResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAccountData(QString NotaryID, QString NymID,
                                        QString AccountID)
{
    int result = opentxs::OTAPI_Wrap::It()->getAccountData(NotaryID.toStdString(),
                                                           NymID.toStdString(),
                                                           AccountID.toStdString());
    QJsonObject object{{"AccountData", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::generateBasketCreation(QString NymID, qint64 MinimumTransfer)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GenerateBasketCreation(NymID.toStdString(),
                                                                           MinimumTransfer);
    QJsonObject object{{"BasketCreation", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::addBasketCreationItem(QString NymID, QString Basket,
                                               QString InstrumentDefinitionID, qint64 MinimumTransfer)
{
    std::string result = opentxs::OTAPI_Wrap::It()->AddBasketCreationItem(NymID.toStdString(),
                                                                          Basket.toStdString(),
                                                                          InstrumentDefinitionID.toStdString(),
                                                                          MinimumTransfer);
    QJsonObject object{{"AddBasketCreationItemResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::issueBasket(QString NotaryID, QString NymID,
                                     QString Basket)
{
    int result = opentxs::OTAPI_Wrap::It()->issueBasket(NotaryID.toStdString(),
                                                        NymID.toStdString(),
                                                        Basket.toStdString());
    QJsonObject object{{"IssueBasketResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::generateBasketExchange(QString NotaryID, QString NymID,
                                                QString BasketInstrumentDefinitionID, QString BasketAssetAccountID,
                                                int TransferMultiple)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GenerateBasketExchange(NotaryID.toStdString(),
                                                                           NymID.toStdString(),
                                                                           BasketInstrumentDefinitionID.toStdString(),
                                                                           BasketAssetAccountID.toStdString(),
                                                                           TransferMultiple);
    QJsonObject object{{"BasketExchange", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::addBasketExchangeItem(QString NotaryID, QString NymID,
                                               QString Basket, QString InstrumentDefinitionID,
                                               QString AssetAccountID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->AddBasketExchangeItem(NotaryID.toStdString(),
                                                                          NymID.toStdString(),
                                                                          Basket.toStdString(),
                                                                          InstrumentDefinitionID.toStdString(),
                                                                          AssetAccountID.toStdString());
    QJsonObject object{{"AddBasketExchangeItemResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::exchangeBasket(QString NotaryID, QString NymID,
                                        QString BasketInstrumentDefinitionID, QString Basket,
                                        bool ExchangeDirection)
{
    int result = opentxs::OTAPI_Wrap::It()->exchangeBasket(NotaryID.toStdString(),
                                                           NymID.toStdString(),
                                                           BasketInstrumentDefinitionID.toStdString(),
                                                           Basket.toStdString(),
                                                           ExchangeDirection);
    QJsonObject object{{"ExchangeBasketResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getTransactionNumbers(QString NotaryID, QString NymID)
{
    int result = opentxs::OTAPI_Wrap::It()->getTransactionNumbers(NotaryID.toStdString(),
                                                                  NymID.toStdString());
    QJsonObject object{{"TransactionNumbers", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::notarizeWithdrawal(QString NotaryID, QString NymID,
                                            QString AccountID, qint64 Amount)
{
    int result = opentxs::OTAPI_Wrap::It()->notarizeWithdrawal(NotaryID.toStdString(),
                                                               NymID.toStdString(),
                                                               AccountID.toStdString(),
                                                               Amount);
    QJsonObject object{{"NotarizeWithdrawalResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::notarizeDeposit(QString NotaryID, QString NymID,
                                         QString AccountID, QString Purse)
{
    int result = opentxs::OTAPI_Wrap::It()->notarizeDeposit(NotaryID.toStdString(),
                                                            NymID.toStdString(),
                                                            AccountID.toStdString(),
                                                            Purse.toStdString());
    QJsonObject object{{"NotarizeDepositResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::notarizeTransfer(QString NotaryID, QString NymID,
                                          QString AccountFrom, QString AccountTo,
                                          qint64 Amount, QString Note)
{
    int result = opentxs::OTAPI_Wrap::It()->notarizeTransfer(NotaryID.toStdString(),
                                                             NymID.toStdString(),
                                                             AccountFrom.toStdString(),
                                                             AccountTo.toStdString(),
                                                             Amount,
                                                             Note.toStdString());
    QJsonObject object{{"NotarizeTransferResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymbox(QString NotaryID, QString NymID)
{
    int result = opentxs::OTAPI_Wrap::It()->getNymbox(NotaryID.toStdString(),
                                                      NymID.toStdString());
    QJsonObject object{{"NymBox", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::processInbox(QString NotaryID, QString NymID,
                                      QString AccountID, QString AccountLedger)
{
    int result = opentxs::OTAPI_Wrap::It()->processInbox(NotaryID.toStdString(),
                                                         NymID.toStdString(),
                                                         AccountID.toStdString(),
                                                         AccountLedger.toStdString());
    QJsonObject object{{"ProcessInboxResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::processNymbox(QString NotaryID, QString NymID)
{
    int result = opentxs::OTAPI_Wrap::It()->processNymbox(NotaryID.toStdString(),
                                                          NymID.toStdString());
    QJsonObject object{{"ProcessNymBoxResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::withdrawVoucher(QString NotaryID, QString NymID,
                                         QString AccountID, QString RecipientNymID,
                                         QString ChequeMemo, qint64 Amount)
{
    int result = opentxs::OTAPI_Wrap::It()->withdrawVoucher(NotaryID.toStdString(),
                                                            NymID.toStdString(),
                                                            AccountID.toStdString(),
                                                            RecipientNymID.toStdString(),
                                                            ChequeMemo.toStdString(),
                                                            Amount);
    QJsonObject object{{"WithdrawVoucherResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::payDividend(QString NotaryID, QString IssuerNymID,
                                     QString DividendFromAccountID, QString SharesInstrumentDefinitionID,
                                     QString DividendMemo, qint64 AmountPerShare)
{
    int result = opentxs::OTAPI_Wrap::It()->payDividend(NotaryID.toStdString(),
                                                        IssuerNymID.toStdString(),
                                                        DividendFromAccountID.toStdString(),
                                                        SharesInstrumentDefinitionID.toStdString(),
                                                        DividendMemo.toStdString(),
                                                        AmountPerShare);
    QJsonObject object{{"PayDividendResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::depositCheque(QString NotaryID, QString NymID,
                                       QString AccountID, QString Cheque)
{
    int result = opentxs::OTAPI_Wrap::It()->depositCheque(NotaryID.toStdString(),
                                                          NymID.toStdString(),
                                                          AccountID.toStdString(),
                                                          Cheque.toStdString());
    QJsonObject object{{"DepositChequeResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::depositPaymentPlan(QString NotaryID, QString NymID,
                                            QString PaymentPlan)
{
    int result = opentxs::OTAPI_Wrap::It()->depositPaymentPlan(NotaryID.toStdString(),
                                                               NymID.toStdString(),
                                                               PaymentPlan.toStdString());
    QJsonObject object{{"DepositPaymentPlanResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::killMarketOffer(QString NotaryID, QString NymID,
                                         QString AssetAccountID, qint64 TransactionNumber)
{
    int result = opentxs::OTAPI_Wrap::It()->killMarketOffer(NotaryID.toStdString(),
                                                            NymID.toStdString(),
                                                            AssetAccountID.toStdString(),
                                                            TransactionNumber);
    QJsonObject object{{"KillMarketOfferResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::killPaymentPlan(QString NotaryID, QString NymID,
                                         QString FromAccountID, qint64 TransactionNumber)
{
    int result = opentxs::OTAPI_Wrap::It()->killPaymentPlan(NotaryID.toStdString(),
                                                            NymID.toStdString(),
                                                            FromAccountID.toStdString(),
                                                            TransactionNumber);
    QJsonObject object{{"KillPaymentPlanResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::issueMarketOffer(QString AssetAccountID, QString CurrencyAccountID,
                                          qint64 MarketScale, qint64 MinimumIncrement,
                                          qint64 TotalAssetsOnOffer, qint64 PriceLimit,
                                          bool BuyingOrSelling, time64_t LifeSpanInSeconds,
                                          QString StopSign, qint64 ActivationPrice)
{
    int result = opentxs::OTAPI_Wrap::It()->issueMarketOffer(AssetAccountID.toStdString(),
                                                             CurrencyAccountID.toStdString(),
                                                             MarketScale,
                                                             MinimumIncrement,
                                                             TotalAssetsOnOffer,
                                                             PriceLimit,
                                                             BuyingOrSelling,
                                                             LifeSpanInSeconds,
                                                             StopSign.toStdString(),
                                                             ActivationPrice);
    QJsonObject object{{"IssueMarketOfferResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getMarketList(QString NotaryID, QString NymID)
{
    int result = opentxs::OTAPI_Wrap::It()->getMarketList(NotaryID.toStdString(),
                                                          NymID.toStdString());
    QJsonObject object{{"GetMarketListResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getMarketOffers(QString NotaryID, QString NymID,
                                         QString MarketID, qint64 MaxDepth)
{
    int result = opentxs::OTAPI_Wrap::It()->getMarketOffers(NotaryID.toStdString(),
                                                            NymID.toStdString(),
                                                            MarketID.toStdString(),
                                                            MaxDepth);
    QJsonObject object{{"GetMarketOffersResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getMarketRecentTrades(QString NotaryID, QString NymID,
                                               QString MarketID)
{
    int result = opentxs::OTAPI_Wrap::It()->getMarketRecentTrades(NotaryID.toStdString(),
                                                                  NymID.toStdString(),
                                                                  MarketID.toStdString());
    QJsonObject object{{"GetMarketRecentTradesResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymMarketOffers(QString NotaryID, QString NymID)
{
    int result = opentxs::OTAPI_Wrap::It()->getNymMarketOffers(NotaryID.toStdString(),
                                                               NymID.toStdString());
    QJsonObject object{{"GetNymMarketOffersResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::popMessageBuffer(qint64 RequestNumber, QString NotaryID,
                                          QString NymID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->PopMessageBuffer(RequestNumber,
                                                                     NotaryID.toStdString(),
                                                                     NymID.toStdString());
    QJsonObject object{{"PopMessageBufferResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::flushMessageBuffer()
{
    opentxs::OTAPI_Wrap::It()->FlushMessageBuffer();
    QJsonObject object{{"MessageBufferFlushed", "True"}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getSentMessage(qint64 RequestNumber, QString NotaryID,
                                        QString NymID)
{
    std::string result = opentxs::OTAPI_Wrap::It()->GetSentMessage(RequestNumber,
                                                                   NotaryID.toStdString(),
                                                                   NymID.toStdString());
    QJsonObject object{{"SentMessage", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::removeSentMessage(qint64 RequestNumber, QString NotaryID,
                                           QString NymID)
{
    bool result = opentxs::OTAPI_Wrap::It()->RemoveSentMessage(RequestNumber,
                                                               NotaryID.toStdString(),
                                                               NymID.toStdString());
    QJsonObject object{{"RemoveSentMessageResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::flushSentMessages(bool HarvestingForRetry, QString NotaryID,
                                           QString NymID, QString NymBox)
{
    opentxs::OTAPI_Wrap::It()->FlushSentMessages(HarvestingForRetry,
                                                 NotaryID.toStdString(),
                                                 NymID.toStdString(),
                                                 NymBox.toStdString());
    QJsonObject object{{"FlushedSentMessages", "True"}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::sleep(qint64 Milliseconds)
{
    opentxs::OTAPI_Wrap::It()->Sleep(Milliseconds);
    QJsonObject object{{"Sleep", Milliseconds}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::resyncNymWithServer(QString NotaryID, QString NymID,
                                             QString Message)
{
    bool result = opentxs::OTAPI_Wrap::It()->ResyncNymWithServer(NotaryID.toStdString(),
                                                                 NymID.toStdString(),
                                                                 Message.toStdString());
    QJsonObject object{{"ResyncNymWithServerResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::queryInstrumentDefinitions(QString NotaryID, QString NymID,
                                                    QString EncodedMap)
{
    int result = opentxs::OTAPI_Wrap::It()->queryInstrumentDefinitions(NotaryID.toStdString(),
                                                                       NymID.toStdString(),
                                                                       EncodedMap.toStdString());
    QJsonObject object{{"QueryInstrumentDefinitionsResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetPayload(QString Message)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Message_GetPayload(Message.toStdString());
    QJsonObject object{{"MessagePayload", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetCommand(QString Message)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Message_GetCommand(Message.toStdString());
    QJsonObject object{{"MessageCommand", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetLedger(QString Message)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Message_GetLedger(Message.toStdString());
    QJsonObject object{{"MessageLedger", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetNewInstrumentDefinitionID(QString Message)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Message_GetNewInstrumentDefinitionID(Message.toStdString());
    QJsonObject object{{"MessageNewInstrumentDefinitionID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetNewIssuerAccountID(QString Message)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Message_GetNewIssuerAcctID(Message.toStdString());
    QJsonObject object{{"MessageNewIssuerAccountID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetNewAccountID(QString Message)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Message_GetNewAcctID(Message.toStdString());
    QJsonObject object{{"MessageNewAccountID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetNymboxHash(QString Message)
{
    std::string result = opentxs::OTAPI_Wrap::It()->Message_GetNymboxHash(Message.toStdString());
    QJsonObject object{{"MessageNymboxHash", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetSuccess(QString Message)
{
    bool result = opentxs::OTAPI_Wrap::It()->Message_GetSuccess(Message.toStdString());
    QJsonObject object{{"MessageNymboxHash", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetDepth(QString Message)
{
    int result = opentxs::OTAPI_Wrap::It()->Message_GetDepth(Message.toStdString());
    QJsonObject object{{"MessageDepth", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageIsTransactionCanceled(QString NotaryID, QString NymID,
                                                      QString AccountID, QString Message)
{
    bool result = opentxs::OTAPI_Wrap::It()->Message_IsTransactionCanceled(NotaryID.toStdString(),
                                                                           NymID.toStdString(),
                                                                           AccountID.toStdString(),
                                                                           Message.toStdString());
    QJsonObject object{{"MessageIsTransactionCanceledResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetTransactionSuccess(QString NotaryID, QString NymID,
                                                      QString AccountID, QString Message)
{
    bool result = opentxs::OTAPI_Wrap::It()->Message_GetTransactionSuccess(NotaryID.toStdString(),
                                                                           NymID.toStdString(),
                                                                           AccountID.toStdString(),
                                                                           Message.toStdString());
    QJsonObject object{{"MessageTransactionSuccess", result}};
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
