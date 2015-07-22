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
