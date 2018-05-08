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

#include <opentxs/opentxs.hpp>

#include <core/moneychanger.hpp>
#include <core/handlers/contacthandler.hpp>

MCRPCService::MCRPCService(QObject* parent)
    : QJsonRpcService(parent)  //, m_RecordList(nullptr)
{
    QThreadPool::globalInstance()->setMaxThreadCount(1);
}

MCRPCService::~MCRPCService()
{

    if (m_RecordList != nullptr) delete m_RecordList;
}

QJsonValue MCRPCService::CreateIndividualNym(
    QString Username,
    QString APIKey,
    QString Name,
    QString Seed)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};

        return QJsonValue(object);
    }
    const std::string result = opentxs::OT::App().API().Exec().CreateNymHD(
        opentxs::proto::CITEMTYPE_INDIVIDUAL, Name.toStdString(), Seed.toStdString(), 0);
    QJsonObject object{{"CreateNymHDResult", QString(result.c_str())}};

    return QJsonValue(object);
}

QJsonValue MCRPCService::registerAccount(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString InstrumentDefinitionID)
{

    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    const opentxs::Identifier notaryID{NotaryID.toStdString()}, nymID{NymID.toStdString()}, instrumentDefinitionID{InstrumentDefinitionID.toStdString()};
    auto action = opentxs::OT::App().API().ServerAction().RegisterAccount(
    		nymID, notaryID, instrumentDefinitionID);
    std::string result = action->Run();

    if (!opentxs::OT::App().API().Exec().CheckConnection(NotaryID.toStdString())) {
        //      emit appendToLog(qstrErrorMsg); // TODO!
        qDebug() << "Network failure while trying to create asset account.";

        QJsonObject object{{"Error", "Network failure"}};
        return QJsonValue(object);
    }

    //  std::cout << "result is " << result << std::endl;

    // -1 error, 0 failure, 1 success.
    //
    if (1 != opentxs::VerifyMessageSuccess(result)) {
        const int64_t lUsageCredits =
            Moneychanger::It()->HasUsageCredits(NotaryID, NymID);

        // HasUsageCredits already pops up an error box in the cases of -2 and
        // 0.
        //
        if (((-2) != lUsageCredits) && (0 != lUsageCredits)) {
            QJsonObject object{{"Error", "Out of usage credits."}};
            return QJsonValue(object);
        }
    }
    // ------------------------------------------------------
    // Get the ID of the new account.
    //
    QString qstrID = QString::fromStdString(
        opentxs::OT::App().API().Exec().Message_GetNewAcctID(result));

    if (qstrID.isEmpty()) {
        QJsonObject object{
            {"Error", "Failed to get new account ID from server response."}};
        return QJsonValue(object);
    }
    // ------------------------------------------------------
    QJsonObject object{{"CreateAccountResult", qstrID}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::numListAdd(
    QString Username,
    QString APIKey,
    QString NumList,
    QString Numbers)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().NumList_Add(
        NumList.toStdString(), Numbers.toStdString());
    QJsonObject object{{"NumListAddResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::numListRemove(
    QString Username,
    QString APIKey,
    QString NumList,
    QString Numbers)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().NumList_Remove(
        NumList.toStdString(), Numbers.toStdString());
    QJsonObject object{{"NumListRemoveResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::numListVerifyQuery(
    QString Username,
    QString APIKey,
    QString NumList,
    QString Numbers)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().NumList_VerifyQuery(
        NumList.toStdString(), Numbers.toStdString());
    QJsonObject object{{"NumListVerifyQueryResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::numListVerifyAll(
    QString Username,
    QString APIKey,
    QString NumList,
    QString Numbers)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().NumList_VerifyAll(
        NumList.toStdString(), Numbers.toStdString());
    QJsonObject object{{"NumListVerifyAllResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::numListCount(
    QString Username,
    QString APIKey,
    QString NumList)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int result =
        opentxs::OT::App().API().Exec().NumList_Count(NumList.toStdString());
    QJsonObject object{{"NumListCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::createNymLegacy(
    QString Username,
    QString APIKey,
    int KeySize,
    QString NymIDSource)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    std::string result = opentxs::OT::App().API().Exec().CreateNymLegacy(
        KeySize, NymIDSource.toStdString());
    QJsonObject object{{"CreateNymLegacyResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymActiveCronItemIDs(
    QString Username,
    QString APIKey,
    QString NymID,
    QString NotaryID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().GetNym_ActiveCronItemIDs(
        NymID.toStdString(), NotaryID.toStdString());
    QJsonObject object{{"ActiveCronItemIDs", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getActiveCronItem(
    QString Username,
    QString APIKey,
    QString NotaryID,
    qint64 TransNum)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().GetActiveCronItem(
        NotaryID.toStdString(), TransNum);
    QJsonObject object{{"ActiveCronItem", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymSourceForID(
    QString Username,
    QString APIKey,
    QString NymID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().GetNym_SourceForID(NymID.toStdString());
    QJsonObject object{{"NymSourceForID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymDescription(
    QString Username,
    QString APIKey,
    QString NymID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().GetNym_Description(NymID.toStdString());
    QJsonObject object{{"NymDescription", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymMasterCredentialCount(
    QString Username,
    QString APIKey,
    QString NymID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OT::App().API().Exec().GetNym_MasterCredentialCount(
        NymID.toStdString());
    QJsonObject object{{"NymCredentialCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymMasterCredentialID(
    QString Username,
    QString APIKey,
    QString NymID,
    int Index)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().GetNym_MasterCredentialID(
        NymID.toStdString(), Index);
    QJsonObject object{{"NymCredentialID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymMasterCredentialContents(
    QString Username,
    QString APIKey,
    QString NymID,
    QString CredentialID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().GetNym_MasterCredentialContents(
            NymID.toStdString(), CredentialID.toStdString());
    QJsonObject object{{"NymCredentialContents", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymRevokedCount(
    QString Username,
    QString APIKey,
    QString NymID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OT::App().API().Exec().GetNym_RevokedCredCount(
        NymID.toStdString());
    QJsonObject object{{"NymRevokedCredCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymRevokedCredID(
    QString Username,
    QString APIKey,
    QString NymID,
    int Index)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().GetNym_RevokedCredID(
        NymID.toStdString(), Index);
    QJsonObject object{{"NymRevokedCredID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymRevokedCredContents(
    QString Username,
    QString APIKey,
    QString NymID,
    QString CredentialID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().GetNym_RevokedCredContents(
            NymID.toStdString(), CredentialID.toStdString());
    QJsonObject object{{"NymRevokedCredContents", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymChildCredentialCount(
    QString Username,
    QString APIKey,
    QString NymID,
    QString MasterCredID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(MasterCredID.toStdString())) {
        QJsonObject object{{"Error", "Invalid MasterCredID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OT::App().API().Exec().GetNym_ChildCredentialCount(
        NymID.toStdString(), MasterCredID.toStdString());
    QJsonObject object{{"NymChildCredentialCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymChildCredentialID(
    QString Username,
    QString APIKey,
    QString NymID,
    QString MasterCredID,
    int Index)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(MasterCredID.toStdString())) {
        QJsonObject object{{"Error", "Invalid MasterCredID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().GetNym_ChildCredentialID(
        NymID.toStdString(), MasterCredID.toStdString(), Index);
    QJsonObject object{{"NymChildCredentialID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymChildCredentialContents(
    QString Username,
    QString APIKey,
    QString NymID,
    QString MasterCredID,
    QString SubCredID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(MasterCredID.toStdString())) {
        QJsonObject object{{"Error", "Invalid MasterCredID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SubCredID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SubCredID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().GetNym_ChildCredentialContents(
            NymID.toStdString(),
            MasterCredID.toStdString(),
            SubCredID.toStdString());
    QJsonObject object{{"NymChildCredentialContents", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::revokeChildcredential(
    QString Username,
    QString APIKey,
    QString NymID,
    QString MasterCredID,
    QString SubCredID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(MasterCredID.toStdString())) {
        QJsonObject object{{"Error", "Invalid MasterCredID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SubCredID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SubCredID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().RevokeChildCredential(
        NymID.toStdString(),
        MasterCredID.toStdString(),
        SubCredID.toStdString());
    QJsonObject object{{"RevokeChildCredentialResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getSignerNymID(
    QString Username,
    QString APIKey,
    QString Contract)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().GetSignerNymID(Contract.toStdString());
    QJsonObject object{{"SignerNymID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::calculateContractID(
    QString Username,
    QString APIKey,
    QString Contract)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().CalculateContractID(
        Contract.toStdString());
    QJsonObject object{{"ContractID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::createCurrencyContract(
    QString Username,
    QString APIKey,
    QString NymID,
    QString shortname,
    QString terms,
    QString names,
    QString symbol,
    QString tla,
    QString power,
    QString fraction)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    std::string result = opentxs::OT::App().API().Exec().CreateCurrencyContract(
        NymID.toStdString(),
        shortname.toStdString(),
        terms.toStdString(),
        names.toStdString(),
        symbol.toStdString(),
        tla.toStdString(),
        stoi(power.toStdString()),
        fraction.toStdString());
    QJsonObject object{
        {"CreateCurrencyContractResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getServerContract(
    QString Username,
    QString APIKey,
    QString NotaryID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().GetServer_Contract(NotaryID.toStdString());
    QJsonObject object{{"ServerContract", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getCurrencyDecimalPower(
    QString Username,
    QString APIKey,
    QString InstrumentDefinitionID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OT::App().API().Exec().GetCurrencyDecimalPower(
        InstrumentDefinitionID.toStdString());
    QJsonObject object{{"CurrencyDecimalPower", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getCurrencyTLA(
    QString Username,
    QString APIKey,
    QString InstrumentDefinitionID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().GetCurrencyTLA(
        InstrumentDefinitionID.toStdString());
    QJsonObject object{{"CurrencyTLA", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getCurrencySymbol(
    QString Username,
    QString APIKey,
    QString InstrumentDefinitionID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().GetCurrencySymbol(
        InstrumentDefinitionID.toStdString());
    QJsonObject object{{"CurrencySymbol", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::stringToAmountLocale(
    QString Username,
    QString APIKey,
    QString InstrumentDefinitionID,
    QString Input,
    QString ThousandsSep,
    QString DecimalPoint)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    qint64 result = opentxs::OT::App().API().Exec().StringToAmountLocale(
        InstrumentDefinitionID.toStdString(),
        Input.toStdString(),
        ThousandsSep.toStdString(),
        DecimalPoint.toStdString());
    QJsonObject object{{"StringToAmountLocaleResult",
                        QString(std::to_string(result).c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::formatAmountLocale(
    QString Username,
    QString APIKey,
    QString InstrumentDefinitionID,
    qint64 Amount,
    QString ThousandsSep,
    QString DecimalPoint)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().FormatAmountLocale(
        InstrumentDefinitionID.toStdString(),
        Amount,
        ThousandsSep.toStdString(),
        DecimalPoint.toStdString());
    QJsonObject object{{"FormatAmountLocaleResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::formatAmountWithoutSymbolLocale(
    QString Username,
    QString APIKey,
    QString InstrumentDefinitionID,
    qint64 Amount,
    QString ThousandsSep,
    QString DecimalPoint)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().FormatAmountWithoutSymbolLocale(
            InstrumentDefinitionID.toStdString(),
            Amount,
            ThousandsSep.toStdString(),
            DecimalPoint.toStdString());
    QJsonObject object{
        {"FormatAmountWithoutSymbolLocaleResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::stringToAmount(
    QString Username,
    QString APIKey,
    QString InstrumentDefinitionID,
    QString Input)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    qint64 result = opentxs::OT::App().API().Exec().StringToAmount(
        InstrumentDefinitionID.toStdString(), Input.toStdString());
    QJsonObject object{
        {"StringToAmountResult", QString(std::to_string(result).c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::formatAmount(
    QString Username,
    QString APIKey,
    QString InstrumentDefinitionID,
    qint64 Amount)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().FormatAmount(
        InstrumentDefinitionID.toStdString(), Amount);
    QJsonObject object{{"FormatAmountResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::formatAmountWithoutSymbol(
    QString Username,
    QString APIKey,
    QString InstrumentDefinitionID,
    qint64 Amount)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().FormatAmountWithoutSymbol(
        InstrumentDefinitionID.toStdString(), Amount);
    QJsonObject object{
        {"FormatAmountWithoutSymbolResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAssetTypeContract(
    QString Username,
    QString APIKey,
    QString InstrumentDefinitionID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    std::string contract = opentxs::OT::App().API().Exec().GetAssetType_Contract(
        InstrumentDefinitionID.toStdString());
    QJsonObject object{{"AssetTypeContract", QString(contract.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::addServerContract(
    QString Username,
    QString APIKey,
    QString Contract)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string l_count =
        opentxs::OT::App().API().Exec().AddServerContract(Contract.toStdString());
    QJsonObject object{{"AddServerContractResult", l_count.c_str()}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::addUnitDefinition(
    QString Username,
    QString APIKey,
    QString Contract)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string newID =
        opentxs::OT::App().API().Exec().AddUnitDefinition(Contract.toStdString());
    QJsonObject object{{"AddUnitDefinitionResult", newID.c_str()}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymCount(QString Username, QString APIKey)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int l_count = opentxs::OT::App().API().Exec().GetNymCount();
    QJsonObject object{{"NymCount", l_count}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getServerCount(QString Username, QString APIKey)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int l_count = opentxs::OT::App().API().Exec().GetServerCount();
    QJsonObject object{{"ServerCount", l_count}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAssetTypeCount(QString Username, QString APIKey)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int l_count = opentxs::OT::App().API().Exec().GetAssetTypeCount();
    QJsonObject object{{"AssetTypeCount", l_count}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAccountCount(QString Username, QString APIKey)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int l_count = opentxs::OT::App().API().Exec().GetAccountCount();
    QJsonObject object{{"AccountCount", l_count}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletCanRemoveServer(
    QString Username,
    QString APIKey,
    QString NotaryID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().Wallet_CanRemoveServer(
        NotaryID.toStdString());
    QJsonObject object{{"WalletCanRemoveServerResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletRemoveServer(
    QString Username,
    QString APIKey,
    QString NotaryID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().Wallet_RemoveServer(
        NotaryID.toStdString());
    QJsonObject object{{"WalletRemoveServerResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletCanRemoveAssetType(
    QString Username,
    QString APIKey,
    QString InstrumentDefinitionID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().Wallet_CanRemoveAssetType(
        InstrumentDefinitionID.toStdString());
    QJsonObject object{{"WalletCanRemoveAssetTypeResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletRemoveAssetType(
    QString Username,
    QString APIKey,
    QString InstrumentDefinitionID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().Wallet_RemoveAssetType(
        InstrumentDefinitionID.toStdString());
    QJsonObject object{{"WalletRemoveAssetTypeResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletCanRemoveNym(
    QString Username,
    QString APIKey,
    QString NymID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    bool result =
        opentxs::OT::App().API().Exec().Wallet_CanRemoveNym(NymID.toStdString());
    QJsonObject object{{"WalletCanRemoveNymResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletRemoveNym(
    QString Username,
    QString APIKey,
    QString NymID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    bool result =
        opentxs::OT::App().API().Exec().Wallet_RemoveNym(NymID.toStdString());
    QJsonObject object{{"WalletRemoveNymResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletCanRemoveAccount(
    QString Username,
    QString APIKey,
    QString AccountID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().Wallet_CanRemoveAccount(
        AccountID.toStdString());
    QJsonObject object{{"WalletCanRemoveAccountResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::doesBoxReceiptExist(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID,
    int BoxType,
    qint64 TransactionNumber)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().DoesBoxReceiptExist(
        NotaryID.toStdString(),
        NymID.toStdString(),
        AccountID.toStdString(),
        BoxType,
        TransactionNumber);
    QJsonObject object{{"DoesBoxReceiptExistResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletExportNym(
    QString Username,
    QString APIKey,
    QString NymID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().Wallet_ExportNym(NymID.toStdString());
    QJsonObject object{{"ExportedNym", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletImportNym(
    QString Username,
    QString APIKey,
    QString FileContents)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Wallet_ImportNym(
        FileContents.toStdString());
    QJsonObject object{{"WalletImportNymResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletGetNymIdFromPartial(
    QString Username,
    QString APIKey,
    QString PartialID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    /* Not putting ID check in here for the moment */

    std::string result =
        opentxs::OT::App().API().Exec().Wallet_GetNymIDFromPartial(
            PartialID.toStdString());
    QJsonObject object{{"NymIdFromPartial", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletGetNotaryIdFromPartial(
    QString Username,
    QString APIKey,
    QString PartialID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().Wallet_GetNotaryIDFromPartial(
            PartialID.toStdString());
    QJsonObject object{{"NotaryIdFromPartial", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletGetInstrumentDefinitionIdFromPartial(
    QString Username,
    QString APIKey,
    QString PartialID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec()
                             .Wallet_GetInstrumentDefinitionIDFromPartial(
                                 PartialID.toStdString());
    QJsonObject object{
        {"InstrumentDefinitionIdFromPartial", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletGetAccountIdFromPartial(
    QString Username,
    QString APIKey,
    QString PartialID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().Wallet_GetAccountIDFromPartial(
            PartialID.toStdString());
    QJsonObject object{{"AccountIdFromPartial", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymID(QString Username, QString APIKey, int Index)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().GetNym_ID(Index);
    QJsonObject object{{"NymID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymName(
    QString Username,
    QString APIKey,
    QString NymID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().GetNym_Name(NymID.toStdString());
    QJsonObject object{{"NymName", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::isNymRegisteredAtServer(
    QString Username,
    QString APIKey,
    QString NymID,
    QString NotaryID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().IsNym_RegisteredAtServer(
        NymID.toStdString(), NotaryID.toStdString());
    QJsonObject object{{"IsNymRegisteredAtServerResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymStats(
    QString Username,
    QString APIKey,
    QString NymID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().GetNym_Stats(NymID.toStdString());
    QJsonObject object{{"NymStats", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymNymboxHash(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().GetNym_NymboxHash(
        NotaryID.toStdString(), NymID.toStdString());
    QJsonObject object{{"NymNymboxHash", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymRecentHash(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().GetNym_RecentHash(
        NotaryID.toStdString(), NymID.toStdString());
    QJsonObject object{{"NymRecentHash", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymInboxHash(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().GetNym_InboxHash(
        NotaryID.toStdString(), NymID.toStdString());
    QJsonObject object{{"NymInboxHash", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymOutboxHash(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().GetNym_OutboxHash(
        NotaryID.toStdString(), NymID.toStdString());
    QJsonObject object{{"NymOutboxHash", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymOutpaymentsCount(
    QString Username,
    QString APIKey,
    QString NymID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OT::App().API().Exec().GetNym_OutpaymentsCount(
        NymID.toStdString());
    QJsonObject object{{"NymOutpaymentsCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymOutpaymentsContentsByIndex(
    QString Username,
    QString APIKey,
    QString NymID,
    int Index)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().GetNym_OutpaymentsContentsByIndex(
            NymID.toStdString(), Index);
    QJsonObject object{
        {"NymOutpaymentsContentsByIndex", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymOutpaymentsRecipientIDByIndex(
    QString Username,
    QString APIKey,
    QString NymID,
    int Index)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().GetNym_OutpaymentsRecipientIDByIndex(
            NymID.toStdString(), Index);
    QJsonObject object{
        {"NymOutpaymentsRecipientIDByIndex", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymOutpaymentsNotaryIDByIndex(
    QString Username,
    QString APIKey,
    QString NymID,
    int Index)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().GetNym_OutpaymentsNotaryIDByIndex(
            NymID.toStdString(), Index);
    QJsonObject object{
        {"NymOutpaymentsNotaryIDByIndex", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::nymRemoveOutpaymentsByIndex(
    QString Username,
    QString APIKey,
    QString NymID,
    int Index)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().Nym_RemoveOutpaymentsByIndex(
        NymID.toStdString(), Index);
    QJsonObject object{{"NymRemoveOutpaymentsByIndexResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::nymVerifyOutpaymentsByIndex(
    QString Username,
    QString APIKey,
    QString NymID,
    int Index)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().Nym_VerifyOutpaymentsByIndex(
        NymID.toStdString(), Index);
    QJsonObject object{{"NymVerifyOutpaymentsByIndexResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetAmount(
    QString Username,
    QString APIKey,
    QString Instrument)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int result = opentxs::OT::App().API().Exec().Instrmnt_GetAmount(
        Instrument.toStdString());
    QJsonObject object{{"InstrumentAmount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetTransactionNumber(
    QString Username,
    QString APIKey,
    QString Instrument)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int result = opentxs::OT::App().API().Exec().Instrmnt_GetTransNum(
        Instrument.toStdString());
    QJsonObject object{{"InstrumentTransactionNumber", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetValidFrom(
    QString Username,
    QString APIKey,
    QString Instrument)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    time64_t result = opentxs::OT::App().API().Exec().Instrmnt_GetValidFrom(
        Instrument.toStdString());
    QJsonObject object{
        {"InstrumentValidFrom", QString(std::to_string(result).c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetValidTo(
    QString Username,
    QString APIKey,
    QString Instrument)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    time64_t result = opentxs::OT::App().API().Exec().Instrmnt_GetValidTo(
        Instrument.toStdString());
    QJsonObject object{
        {"InstrumentValidTo", QString(std::to_string(result).c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetType(
    QString Username,
    QString APIKey,
    QString Instrument)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().Instrmnt_GetType(Instrument.toStdString());
    QJsonObject object{{"InstrumentType", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetMemo(
    QString Username,
    QString APIKey,
    QString Instrument)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().Instrmnt_GetMemo(Instrument.toStdString());
    QJsonObject object{{"InstrumentMemo", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetNotaryID(
    QString Username,
    QString APIKey,
    QString Instrument)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Instrmnt_GetNotaryID(
        Instrument.toStdString());
    QJsonObject object{{"InstrumentNotaryID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetInstrumentDefinitionID(
    QString Username,
    QString APIKey,
    QString Instrument)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().Instrmnt_GetInstrumentDefinitionID(
            Instrument.toStdString());
    QJsonObject object{{"InstrumentDefinitionID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetRemitterNymID(
    QString Username,
    QString APIKey,
    QString Instrument)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Instrmnt_GetRemitterNymID(
        Instrument.toStdString());
    QJsonObject object{{"InstrumentRemitterNymID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetRemitterAccountID(
    QString Username,
    QString APIKey,
    QString Instrument)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().Instrmnt_GetRemitterAcctID(
            Instrument.toStdString());
    QJsonObject object{
        {"InstrumentRemitterAccountID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetSenderNymID(
    QString Username,
    QString APIKey,
    QString Instrument)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Instrmnt_GetSenderNymID(
        Instrument.toStdString());
    QJsonObject object{{"InstrumentSenderNymID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetSenderAccountID(
    QString Username,
    QString APIKey,
    QString Instrument)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Instrmnt_GetSenderAcctID(
        Instrument.toStdString());
    QJsonObject object{{"InstrumentSenderAccountID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetRecipientNymID(
    QString Username,
    QString APIKey,
    QString Instrument)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().Instrmnt_GetRecipientNymID(
            Instrument.toStdString());
    QJsonObject object{{"InstrumentRecipientNymID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetRecipientAccountID(
    QString Username,
    QString APIKey,
    QString Instrument)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().Instrmnt_GetRecipientAcctID(
            Instrument.toStdString());
    QJsonObject object{
        {"InstrumentRecipientAccountID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::setNymName(
    QString Username,
    QString APIKey,
    QString NymID,
    QString SignerNymID,
    QString NewName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    if (!opentxs::OT::App().API().Exec().IsValidID(SignerNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().SetNym_Alias(
        NymID.toStdString(), SignerNymID.toStdString(), NewName.toStdString());
    QJsonObject object{{"SetNymNameResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::setServerName(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NewName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().SetServer_Name(
        NotaryID.toStdString(), NewName.toStdString());
    QJsonObject object{{"SetServerNameResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::setAssetTypeName(
    QString Username,
    QString APIKey,
    QString InstrumentDefinitionID,
    QString NewName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().SetAssetType_Name(
        InstrumentDefinitionID.toStdString(), NewName.toStdString());
    QJsonObject object{{"SetAssetTypeNameResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymTransactionNumberCount(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OT::App().API().Exec().GetNym_TransactionNumCount(
        NotaryID.toStdString(), NymID.toStdString());
    QJsonObject object{{"NymTransactionNumberCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getServerID(
    QString Username,
    QString APIKey,
    int Index)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().GetServer_ID(Index);
    QJsonObject object{{"ServerID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getServerName(
    QString Username,
    QString APIKey,
    QString ServerID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(ServerID.toStdString())) {
        QJsonObject object{{"Error", "Invalid ServerID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().GetServer_Name(ServerID.toStdString());
    QJsonObject object{{"ServerID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAssetTypeID(
    QString Username,
    QString APIKey,
    int Index)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().GetAssetType_ID(Index);
    QJsonObject object{{"AssetTypeID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAssetTypeName(
    QString Username,
    QString APIKey,
    QString AssetTypeID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AssetTypeID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AssetTypeID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().GetAssetType_Name(
        AssetTypeID.toStdString());
    QJsonObject object{{"AssetTypeName", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAssetTypeTLA(
    QString Username,
    QString APIKey,
    QString AssetTypeID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AssetTypeID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AssetTypeID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().GetAssetType_TLA(
        AssetTypeID.toStdString());
    QJsonObject object{{"AssetTypeTLA", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAccountWalletID(
    QString Username,
    QString APIKey,
    int Index)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().GetAccountWallet_ID(Index);
    QJsonObject object{{"AccountWalletID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAccountWalletName(
    QString Username,
    QString APIKey,
    QString AccountWalletID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            AccountWalletID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountWalletID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().GetAccountWallet_Name(
        AccountWalletID.toStdString());
    QJsonObject object{{"AccountWalletName", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAccountWalletInboxHash(
    QString Username,
    QString APIKey,
    QString AccountWalletID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            AccountWalletID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountWalletID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().GetAccountWallet_InboxHash(
            AccountWalletID.toStdString());
    QJsonObject object{{"AccountWalletInboxHash", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAccountWalletOutboxHash(
    QString Username,
    QString APIKey,
    QString AccountWalletID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            AccountWalletID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountWalletID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().GetAccountWallet_OutboxHash(
            AccountWalletID.toStdString());
    QJsonObject object{{"AccountWalletOutboxHash", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getTime(QString Username, QString APIKey)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int result = opentxs::OT::App().API().Exec().GetTime();
    QJsonObject object{{"Time", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::encode(
    QString Username,
    QString APIKey,
    QString Plaintext,
    bool LineBreaks)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Encode(
        Plaintext.toStdString(), LineBreaks);
    QJsonObject object{{"EncodedText", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::decode(
    QString Username,
    QString APIKey,
    QString Plaintext,
    bool LineBreaks)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Decode(
        Plaintext.toStdString(), LineBreaks);
    QJsonObject object{{"DecodedText", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::encrypt(
    QString Username,
    QString APIKey,
    QString RecipientNymID,
    QString Plaintext)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(RecipientNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid RecipientNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Encrypt(
        RecipientNymID.toStdString(), Plaintext.toStdString());
    QJsonObject object{{"EncryptedText", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::decrypt(
    QString Username,
    QString APIKey,
    QString RecipientNymID,
    QString CipherText)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(RecipientNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid RecipientNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Decrypt(
        RecipientNymID.toStdString(), CipherText.toStdString());
    QJsonObject object{{"DecryptedText", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::createSymmetricKey(QString Username, QString APIKey)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().CreateSymmetricKey();
    QJsonObject object{{"SymmetricKey", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::symmetricEncrypt(
    QString Username,
    QString APIKey,
    QString SymmetricKey,
    QString Plaintext)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().SymmetricEncrypt(
        SymmetricKey.toStdString(), Plaintext.toStdString());
    QJsonObject object{{"EncryptedText", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::symmetricDecrypt(
    QString Username,
    QString APIKey,
    QString SymmetricKey,
    QString CipherTextEnvelope)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().SymmetricDecrypt(
        SymmetricKey.toStdString(), CipherTextEnvelope.toStdString());
    QJsonObject object{{"DecryptedText", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::signContract(
    QString Username,
    QString APIKey,
    QString SignerNymID,
    QString Contract)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SignerNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().SignContract(
        SignerNymID.toStdString(), Contract.toStdString());
    QJsonObject object{{"SignedContract", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::flatSign(
    QString Username,
    QString APIKey,
    QString SignerNymID,
    QString Input,
    QString ContractType)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SignerNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().FlatSign(
        SignerNymID.toStdString(),
        Input.toStdString(),
        ContractType.toStdString());
    QJsonObject object{{"FlatSignedText", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::addSignature(
    QString Username,
    QString APIKey,
    QString SignerNymID,
    QString Contract)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SignerNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().AddSignature(
        SignerNymID.toStdString(), Contract.toStdString());
    QJsonObject object{{"SignedContract", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::verifySignature(
    QString Username,
    QString APIKey,
    QString SignerNymID,
    QString Contract)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SignerNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().VerifySignature(
        SignerNymID.toStdString(), Contract.toStdString());
    QJsonObject object{{"VerifySignatureResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::verifyAndRetrieveXMLContents(
    QString Username,
    QString APIKey,
    QString Contract,
    QString SignerID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SignerID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SignerID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().VerifyAndRetrieveXMLContents(
            Contract.toStdString(), SignerID.toStdString());
    QJsonObject object{{"ContractXMLContents", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::verifyAccountReceipt(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().VerifyAccountReceipt(
        NotaryID.toStdString(), NymID.toStdString(), AccountID.toStdString());
    QJsonObject object{{"VerifyAccountReceiptResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::setAccountWalletName(
    QString Username,
    QString APIKey,
    QString AccountID,
    QString SignerNymID,
    QString AccountName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SignerNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().SetAccountWallet_Name(
        AccountID.toStdString(),
        SignerNymID.toStdString(),
        AccountName.toStdString());
    QJsonObject object{{"SetAccountWalletNameResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAccountWalletBalance(
    QString Username,
    QString APIKey,
    QString AccountWalletID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            AccountWalletID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountWalletID"}};
        return QJsonValue(object);
    }

    qint64 result = opentxs::OT::App().API().Exec().GetAccountWallet_Balance(
        AccountWalletID.toStdString());
    QJsonObject object{
        {"AccountWalletBalance", QString(std::to_string(result).c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAccountWalletType(
    QString Username,
    QString APIKey,
    QString AccountWalletID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            AccountWalletID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountWalletID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().GetAccountWallet_Type(
        AccountWalletID.toStdString());
    QJsonObject object{{"AccountWalletType", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAccountWalletInstrumentDefinitionID(
    QString Username,
    QString APIKey,
    QString AccountWalletID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            AccountWalletID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountWalletID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().GetAccountWallet_InstrumentDefinitionID(
            AccountWalletID.toStdString());
    QJsonObject object{
        {"AccountWalletInstrumentDefinitionID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAccountWalletNotaryID(
    QString Username,
    QString APIKey,
    QString AccountWalletID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            AccountWalletID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountWalletID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().GetAccountWallet_NotaryID(
        AccountWalletID.toStdString());
    QJsonObject object{{"AccountWalletNotaryID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAccountWalletNymID(
    QString Username,
    QString APIKey,
    QString AccountWalletID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            AccountWalletID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountWalletID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().GetAccountWallet_NymID(
        AccountWalletID.toStdString());
    QJsonObject object{{"AccountWalletNymID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::writeCheque(
    QString Username,
    QString APIKey,
    QString NotaryID,
    qint64 ChequeAmount,
    time64_t ValidFrom,
    time64_t ValidTo,
    QString SenderAccountID,
    QString SenderNymID,
    QString ChequeMemo,
    QString RecipientNymID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            SenderAccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SenderAccountID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SenderNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SenderNymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(RecipientNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid RecipientNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().WriteCheque(
        NotaryID.toStdString(),
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

QJsonValue MCRPCService::discardCheque(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID,
    QString Cheque)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().DiscardCheque(
        NotaryID.toStdString(),
        NymID.toStdString(),
        AccountID.toStdString(),
        Cheque.toStdString());
    QJsonObject object{{"DiscardChequeResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::proposePaymentPlan(
    QString Username,
    QString APIKey,
    QString NotaryID,
    time64_t ValidFrom,
    time64_t ValidTo,
    QString SenderAccountID,
    QString SenderNymID,
    QString PlanConsideration,
    QString RecipientAccountID,
    QString RecipientNymID,
    qint64 InitialPaymentAmount,
    time64_t InitialPaymentDelay,
    qint64 PaymentPlanAmount,
    time64_t PaymentPlanDelay,
    time64_t PaymentPlanPeriod,
    time64_t PaymentPlanLength,
    int MaxPayments)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SenderNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SenderNymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            RecipientAccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid RecipientAccountID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(RecipientNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid RecipientNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().ProposePaymentPlan(
        NotaryID.toStdString(),
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

QJsonValue MCRPCService::easyProposePlan(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString DateRange,
    QString SenderAccountID,
    QString SenderNymID,
    QString PlanConsideration,
    QString RecipientAccountID,
    QString RecipientNymID,
    QString InitialPayment,
    QString PaymentPlan,
    QString PlanExpiry)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            SenderAccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SenderAccountID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SenderNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SenderNymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            RecipientAccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid RecipientAccountID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(RecipientNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid RecipientNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().EasyProposePlan(
        NotaryID.toStdString(),
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

QJsonValue MCRPCService::confirmPaymentPlan(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString SenderNymID,
    QString SenderAccountID,
    QString RecipientNymID,
    QString PaymentPlan)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SenderNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SenderNymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            SenderAccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SenderAccountID"}};
        return QJsonValue(object);
    }

    if (!opentxs::OT::App().API().Exec().IsValidID(RecipientNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid RecipientNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().ConfirmPaymentPlan(
        NotaryID.toStdString(),
        SenderNymID.toStdString(),
        SenderAccountID.toStdString(),
        RecipientNymID.toStdString(),
        PaymentPlan.toStdString());
    QJsonObject object{{"ConfirmPaymentPlanResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::createSmartContract(
    QString Username,
    QString APIKey,
    QString SignerNymID,
    time64_t ValidFrom,
    time64_t ValidTo,
    bool SpecifyAssets,
    bool SpecifyParties)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SignerNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Create_SmartContract(
        SignerNymID.toStdString(),
        ValidFrom,
        ValidTo,
        SpecifyAssets,
        SpecifyParties);
    QJsonObject object{{"CreateSmartContractResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractSetDates(
    QString Username,
    QString APIKey,
    QString Contract,
    QString SignerNymID,
    time64_t ValidFrom,
    time64_t ValidTo)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SignerNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().SmartContract_SetDates(
        Contract.toStdString(), SignerNymID.toStdString(), ValidFrom, ValidTo);
    QJsonObject object{
        {"SmartContractSetDatesResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartArePartiesSpecified(
    QString Username,
    QString APIKey,
    QString Contract)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().Smart_ArePartiesSpecified(
        Contract.toStdString());
    QJsonObject object{{"SmartArePartiesSpecifiedResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartAreAssetTypesSpecified(
    QString Username,
    QString APIKey,
    QString Contract)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().Smart_AreAssetTypesSpecified(
        Contract.toStdString());
    QJsonObject object{{"SmartAreAssetTypesSpecifiedResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractAddBylaw(
    QString Username,
    QString APIKey,
    QString Contract,
    QString SignerNymID,
    QString BylawName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SignerNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().SmartContract_AddBylaw(
        Contract.toStdString(),
        SignerNymID.toStdString(),
        BylawName.toStdString());
    QJsonObject object{
        {"SmartContractAddBylawResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractAddClause(
    QString Username,
    QString APIKey,
    QString Contract,
    QString SignerNymID,
    QString BylawName,
    QString ClauseName,
    QString SourceCode)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SignerNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().SmartContract_AddClause(
        Contract.toStdString(),
        SignerNymID.toStdString(),
        BylawName.toStdString(),
        ClauseName.toStdString(),
        SourceCode.toStdString());
    QJsonObject object{
        {"SmartContractAddClauseResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractAddVariable(
    QString Username,
    QString APIKey,
    QString Contract,
    QString SignerNymID,
    QString BylawName,
    QString VarName,
    QString VarAccess,
    QString VarType,
    QString VarValue)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SignerNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().SmartContract_AddVariable(
        Contract.toStdString(),
        SignerNymID.toStdString(),
        BylawName.toStdString(),
        VarName.toStdString(),
        VarAccess.toStdString(),
        VarType.toStdString(),
        VarValue.toStdString());
    QJsonObject object{
        {"SmartContractAddVariableResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractAddCallback(
    QString Username,
    QString APIKey,
    QString Contract,
    QString SignerNymID,
    QString BylawName,
    QString CallbackName,
    QString ClauseName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SignerNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().SmartContract_AddCallback(
        Contract.toStdString(),
        SignerNymID.toStdString(),
        BylawName.toStdString(),
        CallbackName.toStdString(),
        ClauseName.toStdString());
    QJsonObject object{
        {"SmartContractAddCallbackResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractAddHook(
    QString Username,
    QString APIKey,
    QString Contract,
    QString SignerNymID,
    QString BylawName,
    QString HookName,
    QString ClauseName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SignerNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().SmartContract_AddHook(
        Contract.toStdString(),
        SignerNymID.toStdString(),
        BylawName.toStdString(),
        HookName.toStdString(),
        ClauseName.toStdString());
    QJsonObject object{{"SmartContractAddHookResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractAddParty(
    QString Username,
    QString APIKey,
    QString Contract,
    QString SignerNymID,
    QString PartyNymID,
    QString PartyName,
    QString AgentName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SignerNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(PartyNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid PartyNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().SmartContract_AddParty(
        Contract.toStdString(),
        SignerNymID.toStdString(),
        PartyNymID.toStdString(),
        PartyName.toStdString(),
        AgentName.toStdString());
    QJsonObject object{
        {"SmartContractAddPartyResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractAddAccount(
    QString Username,
    QString APIKey,
    QString Contract,
    QString SignerNymID,
    QString PartyName,
    QString AccountName,
    QString InstrumentDefinitionID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SignerNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }
    std::string result = opentxs::OT::App().API().Exec().SmartContract_AddAccount(
        Contract.toStdString(),
        SignerNymID.toStdString(),
        PartyName.toStdString(),
        AccountName.toStdString(),
        InstrumentDefinitionID.toStdString());
    QJsonObject object{
        {"SmartContractAddAccountResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractRemoveBylaw(
    QString Username,
    QString APIKey,
    QString Contract,
    QString SignerNymID,
    QString BylawName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SignerNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().SmartContract_RemoveBylaw(
        Contract.toStdString(),
        SignerNymID.toStdString(),
        BylawName.toStdString());
    QJsonObject object{
        {"SmartContractRemoveBylawResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractUpdateClause(
    QString Username,
    QString APIKey,
    QString Contract,
    QString SignerNymID,
    QString BylawName,
    QString ClauseName,
    QString SourceCode)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SignerNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().SmartContract_UpdateClause(
            Contract.toStdString(),
            SignerNymID.toStdString(),
            BylawName.toStdString(),
            ClauseName.toStdString(),
            SourceCode.toStdString());
    QJsonObject object{
        {"SmartContractUpdateClauseResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractRemoveClause(
    QString Username,
    QString APIKey,
    QString Contract,
    QString SignerNymID,
    QString BylawName,
    QString ClauseName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SignerNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().SmartContract_RemoveClause(
            Contract.toStdString(),
            SignerNymID.toStdString(),
            BylawName.toStdString(),
            ClauseName.toStdString());
    QJsonObject object{
        {"SmartContractRemoveClauseResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractRemoveVariable(
    QString Username,
    QString APIKey,
    QString Contract,
    QString SignerNymID,
    QString BylawName,
    QString VarName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SignerNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().SmartContract_RemoveVariable(
            Contract.toStdString(),
            SignerNymID.toStdString(),
            BylawName.toStdString(),
            VarName.toStdString());
    QJsonObject object{
        {"SmartContractRemoveVariableResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractRemoveCallback(
    QString Username,
    QString APIKey,
    QString Contract,
    QString SignerNymID,
    QString BylawName,
    QString CallbackName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SignerNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().SmartContract_RemoveCallback(
            Contract.toStdString(),
            SignerNymID.toStdString(),
            BylawName.toStdString(),
            CallbackName.toStdString());
    QJsonObject object{
        {"SmartContractRemoveCallbackResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractRemoveHook(
    QString Username,
    QString APIKey,
    QString Contract,
    QString SignerNymID,
    QString BylawName,
    QString HookName,
    QString ClauseName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SignerNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().SmartContract_RemoveHook(
        Contract.toStdString(),
        SignerNymID.toStdString(),
        BylawName.toStdString(),
        HookName.toStdString(),
        ClauseName.toStdString());
    QJsonObject object{
        {"SmartContractRemoveHookResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractRemoveParty(
    QString Username,
    QString APIKey,
    QString Contract,
    QString SignerNymID,
    QString PartyName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SignerNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().SmartContract_RemoveParty(
        Contract.toStdString(),
        SignerNymID.toStdString(),
        PartyName.toStdString());
    QJsonObject object{
        {"SmartContractRemovePartyResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractCountNumbersNeeded(
    QString Username,
    QString APIKey,
    QString Contract,
    QString AgentName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int result = opentxs::OT::App().API().Exec().SmartContract_CountNumsNeeded(
        Contract.toStdString(), AgentName.toStdString());
    QJsonObject object{{"SmartContractNumbersNeeded", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractConfirmAccount(
    QString Username,
    QString APIKey,
    QString Contract,
    QString SignerNymID,
    QString PartyName,
    QString AccountName,
    QString AgentName,
    QString AccountID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SignerNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().SmartContract_ConfirmAccount(
            Contract.toStdString(),
            SignerNymID.toStdString(),
            PartyName.toStdString(),
            AccountName.toStdString(),
            AgentName.toStdString(),
            AccountID.toStdString());
    QJsonObject object{
        {"SmartContractConfirmAccountResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractConfirmParty(
    QString Username,
    QString APIKey,
    QString Contract,
    QString PartyName,
    QString NymID,
    QString NotaryID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().SmartContract_ConfirmParty(
            Contract.toStdString(),
            PartyName.toStdString(),
            NymID.toStdString(),
            NotaryID.toStdString());
    QJsonObject object{
        {"SmartContractConfirmPartyResult", QString(result.c_str())}};

    return QJsonValue(object);
}

QJsonValue MCRPCService::smartAreAllPartiesConfirmed(
    QString Username,
    QString APIKey,
    QString Contract)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().Smart_AreAllPartiesConfirmed(
        Contract.toStdString());
    QJsonObject object{{"SmartAreAllPartiesConfirmedResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartIsPartyConfirmed(
    QString Username,
    QString APIKey,
    QString Contract,
    QString PartyName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().Smart_IsPartyConfirmed(
        Contract.toStdString(), PartyName.toStdString());
    QJsonObject object{{"SmartIsPartyConfirmedResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartGetPartyCount(
    QString Username,
    QString APIKey,
    QString Contract)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int result = opentxs::OT::App().API().Exec().Smart_GetPartyCount(
        Contract.toStdString());
    QJsonObject object{{"SmartPartyCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartGetBylawCount(
    QString Username,
    QString APIKey,
    QString Contract)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int result = opentxs::OT::App().API().Exec().Smart_GetBylawCount(
        Contract.toStdString());
    QJsonObject object{{"SmartBylawCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartGetPartyByIndex(
    QString Username,
    QString APIKey,
    QString Contract,
    int Index)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Smart_GetPartyByIndex(
        Contract.toStdString(), Index);
    QJsonObject object{{"SmartGetPartyByIndexResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartGetBylawByIndex(
    QString Username,
    QString APIKey,
    QString Contract,
    int Index)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Smart_GetBylawByIndex(
        Contract.toStdString(), Index);
    QJsonObject object{{"SmartGetBylawByIndexResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::bylawGetLanguage(
    QString Username,
    QString APIKey,
    QString Contract,
    QString BylawName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Bylaw_GetLanguage(
        Contract.toStdString(), BylawName.toStdString());
    QJsonObject object{{"BylawLanguage", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::bylawGetClauseCount(
    QString Username,
    QString APIKey,
    QString Contract,
    QString BylawName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int result = opentxs::OT::App().API().Exec().Bylaw_GetClauseCount(
        Contract.toStdString(), BylawName.toStdString());
    QJsonObject object{{"BylawClauseCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::bylawGetVariableCount(
    QString Username,
    QString APIKey,
    QString Contract,
    QString BylawName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int result = opentxs::OT::App().API().Exec().Bylaw_GetVariableCount(
        Contract.toStdString(), BylawName.toStdString());
    QJsonObject object{{"BylawVariableCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::bylawGetHookCount(
    QString Username,
    QString APIKey,
    QString Contract,
    QString BylawName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int result = opentxs::OT::App().API().Exec().Bylaw_GetHookCount(
        Contract.toStdString(), BylawName.toStdString());
    QJsonObject object{{"BylawHookCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::bylawGetCallbackCount(
    QString Username,
    QString APIKey,
    QString Contract,
    QString BylawName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int result = opentxs::OT::App().API().Exec().Bylaw_GetCallbackCount(
        Contract.toStdString(), BylawName.toStdString());
    QJsonObject object{{"BylawCallbackCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::clauseGetNameByIndex(
    QString Username,
    QString APIKey,
    QString Contract,
    QString BylawName,
    int Index)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Clause_GetNameByIndex(
        Contract.toStdString(), BylawName.toStdString(), Index);
    QJsonObject object{{"ClauseGetNameByIndexResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::clauseGetContents(
    QString Username,
    QString APIKey,
    QString Contract,
    QString BylawName,
    QString ClauseName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Clause_GetContents(
        Contract.toStdString(),
        BylawName.toStdString(),
        ClauseName.toStdString());
    QJsonObject object{{"ClauseContents", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::variableGetNameByIndex(
    QString Username,
    QString APIKey,
    QString Contract,
    QString BylawName,
    int Index)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Variable_GetNameByIndex(
        Contract.toStdString(), BylawName.toStdString(), Index);
    QJsonObject object{
        {"VariableGetNameByIndexResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::variableGetType(
    QString Username,
    QString APIKey,
    QString Contract,
    QString BylawName,
    QString VariableName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Variable_GetType(
        Contract.toStdString(),
        BylawName.toStdString(),
        VariableName.toStdString());
    QJsonObject object{{"VariableType", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::variableGetAccess(
    QString Username,
    QString APIKey,
    QString Contract,
    QString BylawName,
    QString VariableName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Variable_GetAccess(
        Contract.toStdString(),
        BylawName.toStdString(),
        VariableName.toStdString());
    QJsonObject object{{"VariableAccess", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::variableGetContents(
    QString Username,
    QString APIKey,
    QString Contract,
    QString BylawName,
    QString VariableName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Variable_GetContents(
        Contract.toStdString(),
        BylawName.toStdString(),
        VariableName.toStdString());
    QJsonObject object{{"VariableContents", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::hookGetNameByIndex(
    QString Username,
    QString APIKey,
    QString Contract,
    QString BylawName,
    int Index)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Hook_GetNameByIndex(
        Contract.toStdString(), BylawName.toStdString(), Index);
    QJsonObject object{{"HookGetNameByIndexResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::hookGetClauseCount(
    QString Username,
    QString APIKey,
    QString Contract,
    QString BylawName,
    QString HookName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int result = opentxs::OT::App().API().Exec().Hook_GetClauseCount(
        Contract.toStdString(),
        BylawName.toStdString(),
        HookName.toStdString());
    QJsonObject object{{"HookClauseCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::hookGetClauseAtIndex(
    QString Username,
    QString APIKey,
    QString Contract,
    QString BylawName,
    QString HookName,
    int Index)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Hook_GetClauseAtIndex(
        Contract.toStdString(),
        BylawName.toStdString(),
        HookName.toStdString(),
        Index);
    QJsonObject object{{"HookGetClauseAtIndexResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::callbackGetNameByIndex(
    QString Username,
    QString APIKey,
    QString Contract,
    QString BylawName,
    int Index)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Callback_GetNameByIndex(
        Contract.toStdString(), BylawName.toStdString(), Index);
    QJsonObject object{
        {"CallbackGetNameByIndexResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::callbackGetClause(
    QString Username,
    QString APIKey,
    QString Contract,
    QString BylawName,
    QString ClauseName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Callback_GetClause(
        Contract.toStdString(),
        BylawName.toStdString(),
        ClauseName.toStdString());
    QJsonObject object{{"CallbackGetClauseResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::partyGetAccountCount(
    QString Username,
    QString APIKey,
    QString Contract,
    QString PartyName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int result = opentxs::OT::App().API().Exec().Party_GetAcctCount(
        Contract.toStdString(), PartyName.toStdString());
    QJsonObject object{{"PartyAccountCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::partyGetAgentCount(
    QString Username,
    QString APIKey,
    QString Contract,
    QString PartyName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int result = opentxs::OT::App().API().Exec().Party_GetAgentCount(
        Contract.toStdString(), PartyName.toStdString());
    QJsonObject object{{"PartyAgentCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::partyGetID(
    QString Username,
    QString APIKey,
    QString Contract,
    QString PartyName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Party_GetID(
        Contract.toStdString(), PartyName.toStdString());
    QJsonObject object{{"PartyID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::partyGetAccountNameByIndex(
    QString Username,
    QString APIKey,
    QString Contract,
    QString PartyName,
    int Index)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Party_GetAcctNameByIndex(
        Contract.toStdString(), PartyName.toStdString(), Index);
    QJsonObject object{
        {"PartyGetAccountNameByIndexResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::partyGetAccountID(
    QString Username,
    QString APIKey,
    QString Contract,
    QString PartyName,
    QString AccountName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Party_GetAcctID(
        Contract.toStdString(),
        PartyName.toStdString(),
        AccountName.toStdString());
    QJsonObject object{{"PartyGetAccountIDResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::partyGetAccountInstrumentDefinitionID(
    QString Username,
    QString APIKey,
    QString Contract,
    QString PartyName,
    QString AccountName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().Party_GetAcctInstrumentDefinitionID(
            Contract.toStdString(),
            PartyName.toStdString(),
            AccountName.toStdString());
    QJsonObject object{{"PartyGetAccountInstrumentDefinitionIDResult",
                        QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::partyGetAccountAgentName(
    QString Username,
    QString APIKey,
    QString Contract,
    QString PartyName,
    QString AccountName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Party_GetAcctAgentName(
        Contract.toStdString(),
        PartyName.toStdString(),
        AccountName.toStdString());
    QJsonObject object{{"PartyAccountAgentName", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::partyGetAgentNameByIndex(
    QString Username,
    QString APIKey,
    QString Contract,
    QString PartyName,
    int Index)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Party_GetAgentNameByIndex(
        Contract.toStdString(), PartyName.toStdString(), Index);
    QJsonObject object{
        {"PartyGetAgentNameByIndexResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::partyGetAgentID(
    QString Username,
    QString APIKey,
    QString Contract,
    QString PartyName,
    QString AgentName)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Party_GetAgentID(
        Contract.toStdString(),
        PartyName.toStdString(),
        AgentName.toStdString());
    QJsonObject object{{"PartyGetAgentIDResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageHarvestTransactionNumbers(
    QString Username,
    QString APIKey,
    QString Message,
    QString NymID,
    bool HarvestingForRetry,
    bool ReplyWasSuccess,
    bool ReplyWasFailure,
    bool TransactionWasSuccess,
    bool TransactionWasFailure)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().Msg_HarvestTransactionNumbers(
        Message.toStdString(),
        NymID.toStdString(),
        HarvestingForRetry,
        ReplyWasSuccess,
        ReplyWasFailure,
        TransactionWasSuccess,
        TransactionWasFailure);
    QJsonObject object{{"MessageHarvestTransactionNumbersResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadPubkeyEncryption(
    QString Username,
    QString APIKey,
    QString NymID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().LoadPubkey_Encryption(NymID.toStdString());
    QJsonObject object{{"LoadPubkeyEncryptionResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadPubkeySigning(
    QString Username,
    QString APIKey,
    QString NymID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().LoadPubkey_Signing(NymID.toStdString());
    QJsonObject object{{"LoadPubkeySigningResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadUserPubkeyEncryption(
    QString Username,
    QString APIKey,
    QString NymID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().LoadUserPubkey_Encryption(
        NymID.toStdString());
    QJsonObject object{
        {"LoadUserPubkeyEncryptionResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadUserPubkeySigning(
    QString Username,
    QString APIKey,
    QString NymID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().LoadUserPubkey_Signing(
        NymID.toStdString());
    QJsonObject object{
        {"LoadUserPubkeySigningResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::verifyUserPrivateKey(
    QString Username,
    QString APIKey,
    QString NymID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    bool result =
        opentxs::OT::App().API().Exec().VerifyUserPrivateKey(NymID.toStdString());
    QJsonObject object{{"VerifyUserPrivateKeyResult", result}};
    return QJsonValue(object);
}

#if OT_CASH
QJsonValue MCRPCService::mintIsStillGood(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString InstrumentDefinitionID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().Mint_IsStillGood(
        NotaryID.toStdString(), InstrumentDefinitionID.toStdString());
    QJsonObject object{{"MintIsStillGoodResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadMint(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString InstrumentDefinitionID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().LoadMint(
        NotaryID.toStdString(), InstrumentDefinitionID.toStdString());
    QJsonObject object{{"LoadMintResult", QString(result.c_str())}};
    return QJsonValue(object);
}
#endif  // OT_CASH

QJsonValue MCRPCService::loadServerContract(
    QString Username,
    QString APIKey,
    QString NotaryID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().LoadServerContract(NotaryID.toStdString());
    QJsonObject object{{"LoadServerContractResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadAssetAccount(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().LoadAssetAccount(
        NotaryID.toStdString(), NymID.toStdString(), AccountID.toStdString());
    QJsonObject object{{"LoadAssetAccountResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::nymboxGetReplyNotice(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    qint64 RequestNumber)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Nymbox_GetReplyNotice(
        NotaryID.toStdString(), NymID.toStdString(), RequestNumber);
    QJsonObject object{{"NymboxGetReplyNoticeResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::haveAlreadySeenReply(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    qint64 RequestNumber)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().HaveAlreadySeenReply(
        NotaryID.toStdString(), NymID.toStdString(), RequestNumber);
    QJsonObject object{{"HaveAlreadySeenReplyResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadInbox(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().LoadInbox(
        NotaryID.toStdString(), NymID.toStdString(), AccountID.toStdString());
    QJsonObject object{{"LoadInboxResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadInboxNoVerify(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().LoadInboxNoVerify(
        NotaryID.toStdString(), NymID.toStdString(), AccountID.toStdString());
    QJsonObject object{{"LoadInboxResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadOutbox(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().LoadOutbox(
        NotaryID.toStdString(), NymID.toStdString(), AccountID.toStdString());
    QJsonObject object{{"LoadOutboxResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadOutboxNoVerify(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().LoadOutboxNoVerify(
        NotaryID.toStdString(), NymID.toStdString(), AccountID.toStdString());
    QJsonObject object{{"LoadOutboxNoVerifyResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadPaymentInbox(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().LoadPaymentInbox(
        NotaryID.toStdString(), NymID.toStdString());
    QJsonObject object{{"LoadPaymentInboxResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadPaymentInboxNoVerify(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().LoadPaymentInboxNoVerify(
        NotaryID.toStdString(), NymID.toStdString());
    QJsonObject object{
        {"LoadPaymentInboxNoVerifyResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadRecordBox(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().LoadRecordBox(
        NotaryID.toStdString(), NymID.toStdString(), AccountID.toStdString());
    QJsonObject object{{"LoadRecordBoxResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadRecordBoxNoVerify(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().LoadRecordBoxNoVerify(
        NotaryID.toStdString(), NymID.toStdString(), AccountID.toStdString());
    QJsonObject object{
        {"LoadRecordBoxNoVerifyResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadExpiredBox(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().LoadExpiredBox(
        NotaryID.toStdString(), NymID.toStdString());
    QJsonObject object{{"LoadExpiredBoxResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadExpiredBoxNoVerify(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().LoadExpiredBoxNoVerify(
        NotaryID.toStdString(), NymID.toStdString());
    QJsonObject object{
        {"LoadExpiredBoxNoVerifyResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::recordPayment(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    bool IsInbox,
    int Index,
    bool SaveCopy)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().RecordPayment(
        NotaryID.toStdString(), NymID.toStdString(), IsInbox, Index, SaveCopy);
    QJsonObject object{{"RecordPaymentResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::clearRecord(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID,
    int Index,
    bool ClearAll)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().ClearRecord(
        NotaryID.toStdString(),
        NymID.toStdString(),
        AccountID.toStdString(),
        Index,
        ClearAll);
    QJsonObject object{{"ClearRecordResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::clearExpired(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    int Index,
    bool ClearAll)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().ClearExpired(
        NotaryID.toStdString(), NymID.toStdString(), Index, ClearAll);
    QJsonObject object{{"ClearExpiredResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::ledgerGetCount(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID,
    QString Ledger)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OT::App().API().Exec().Ledger_GetCount(
        NotaryID.toStdString(),
        NymID.toStdString(),
        AccountID.toStdString(),
        Ledger.toStdString());
    QJsonObject object{{"LedgerCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::ledgerCreateResponse(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Ledger_CreateResponse(
        NotaryID.toStdString(),
        NymID.toStdString(),
        AccountID.toStdString());
    QJsonObject object{{"LedgerResponse", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::ledgerGetTransactionByIndex(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID,
    QString Ledger,
    int Index)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().Ledger_GetTransactionByIndex(
            NotaryID.toStdString(),
            NymID.toStdString(),
            AccountID.toStdString(),
            Ledger.toStdString(),
            Index);
    QJsonObject object{
        {"LedgerGetTransactionByIndexResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::ledgerGetTransactionByID(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID,
    QString Ledger,
    qint64 TransactionNumber)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().Ledger_GetTransactionByIndex(
            NotaryID.toStdString(),
            NymID.toStdString(),
            AccountID.toStdString(),
            Ledger.toStdString(),
            TransactionNumber);
    QJsonObject object{
        {"LedgerGetTransactionByIDResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::ledgerGetInstrument(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID,
    QString Ledger,
    int Index)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Ledger_GetInstrument(
        NotaryID.toStdString(),
        NymID.toStdString(),
        AccountID.toStdString(),
        Ledger.toStdString(),
        Index);
    QJsonObject object{{"LedgerGetInstrumentResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::ledgerGetTransactionIDByIndex(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID,
    QString Ledger,
    int Index)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    qint64 result = opentxs::OT::App().API().Exec().Ledger_GetTransactionIDByIndex(
        NotaryID.toStdString(),
        NymID.toStdString(),
        AccountID.toStdString(),
        Ledger.toStdString(),
        Index);
    QJsonObject object{{"LedgerGetTransactionIDByIndexResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::ledgerAddTransaction(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID,
    QString Ledger,
    QString Transaction)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Ledger_AddTransaction(
        NotaryID.toStdString(),
        NymID.toStdString(),
        AccountID.toStdString(),
        Ledger.toStdString(),
        Transaction.toStdString());
    QJsonObject object{{"LedgerAddTransactionResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionCreateResponse(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID,
    QString Ledger,
    QString Transaction,
    bool DoIAccept)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().Transaction_CreateResponse(
            NotaryID.toStdString(),
            NymID.toStdString(),
            AccountID.toStdString(),
            Ledger.toStdString(),
            Transaction.toStdString(),
            DoIAccept);
    QJsonObject object{
        {"TransactionCreateResponseResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::ledgerFinalizeResponse(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID,
    QString Ledger)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Ledger_FinalizeResponse(
        NotaryID.toStdString(),
        NymID.toStdString(),
        AccountID.toStdString(),
        Ledger.toStdString());
    QJsonObject object{
        {"LedgerFinalizeResponseResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionGetVoucher(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID,
    QString Transaction)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Transaction_GetVoucher(
        NotaryID.toStdString(),
        NymID.toStdString(),
        AccountID.toStdString(),
        Transaction.toStdString());
    QJsonObject object{{"TransactionVoucher", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionGetSenderNymID(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID,
    QString Transaction)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().Transaction_GetSenderNymID(
            NotaryID.toStdString(),
            NymID.toStdString(),
            AccountID.toStdString(),
            Transaction.toStdString());
    QJsonObject object{{"TransactionSenderNymID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionGetRecipientNymID(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID,
    QString Transaction)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().Transaction_GetRecipientNymID(
            NotaryID.toStdString(),
            NymID.toStdString(),
            AccountID.toStdString(),
            Transaction.toStdString());
    QJsonObject object{{"TransactionRecipientNymID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionGetSenderAccountID(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID,
    QString Transaction)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().Transaction_GetSenderAcctID(
            NotaryID.toStdString(),
            NymID.toStdString(),
            AccountID.toStdString(),
            Transaction.toStdString());
    QJsonObject object{{"TransactionSenderAccountID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionGetRecipientAccountID(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID,
    QString Transaction)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().Transaction_GetRecipientAcctID(
            NotaryID.toStdString(),
            NymID.toStdString(),
            AccountID.toStdString(),
            Transaction.toStdString());
    QJsonObject object{
        {"TransactionRecipientAccountID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::pendingGetNote(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID,
    QString Transaction)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Pending_GetNote(
        NotaryID.toStdString(),
        NymID.toStdString(),
        AccountID.toStdString(),
        Transaction.toStdString());
    QJsonObject object{{"PendingNote", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionGetAmount(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID,
    QString Transaction)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    qint64 result = opentxs::OT::App().API().Exec().Transaction_GetAmount(
        NotaryID.toStdString(),
        NymID.toStdString(),
        AccountID.toStdString(),
        Transaction.toStdString());
    QJsonObject object{{"TransactionAmount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionGetDisplayReferenceToNumber(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID,
    QString Transaction)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    qint64 result =
        opentxs::OT::App().API().Exec().Transaction_GetDisplayReferenceToNum(
            NotaryID.toStdString(),
            NymID.toStdString(),
            AccountID.toStdString(),
            Transaction.toStdString());
    QJsonObject object{{"TransactionDisplayReferenceToNumber", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionGetType(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID,
    QString Transaction)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Transaction_GetType(
        NotaryID.toStdString(),
        NymID.toStdString(),
        AccountID.toStdString(),
        Transaction.toStdString());
    QJsonObject object{{"TransactionType", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::replyNoticeGetRequestNumber(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString Transaction)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    qint64 result = opentxs::OT::App().API().Exec().ReplyNotice_GetRequestNum(
        NotaryID.toStdString(), NymID.toStdString(), Transaction.toStdString());
    QJsonObject object{{"ReplyNoticeRequestNumber", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionGetDateSigned(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID,
    QString Transaction)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    time64_t result = opentxs::OT::App().API().Exec().Transaction_GetDateSigned(
        NotaryID.toStdString(),
        NymID.toStdString(),
        AccountID.toStdString(),
        Transaction.toStdString());
    QJsonObject object{
        {"TransactionDateSigned", QString(std::to_string(result).c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionGetSuccess(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID,
    QString Transaction)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().Transaction_GetSuccess(
        NotaryID.toStdString(),
        NymID.toStdString(),
        AccountID.toStdString(),
        Transaction.toStdString());
    QJsonObject object{{"TransactionSuccess", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionIsCanceled(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID,
    QString Transaction)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().Transaction_IsCanceled(
        NotaryID.toStdString(),
        NymID.toStdString(),
        AccountID.toStdString(),
        Transaction.toStdString());
    QJsonObject object{{"TransactionIsCanceled", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionGetBalanceAgreementSuccess(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID,
    QString Transaction)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    bool result =
        opentxs::OT::App().API().Exec().Transaction_GetBalanceAgreementSuccess(
            NotaryID.toStdString(),
            NymID.toStdString(),
            AccountID.toStdString(),
            Transaction.toStdString());
    QJsonObject object{{"TransactionBalanceAgreementSuccess", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetBalanceAgreementSuccess(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID,
    QString Message)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    bool result =
        opentxs::OT::App().API().Exec().Message_GetBalanceAgreementSuccess(
            NotaryID.toStdString(),
            NymID.toStdString(),
            AccountID.toStdString(),
            Message.toStdString());
    QJsonObject object{{"MessageBalanceAgreementSuccess", result}};
    return QJsonValue(object);
}

#if OT_CASH
QJsonValue MCRPCService::savePurse(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString InstrumentDefinitionID,
    QString NymID,
    QString Purse)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().SavePurse(
        NotaryID.toStdString(),
        InstrumentDefinitionID.toStdString(),
        NymID.toStdString(),
        Purse.toStdString());
    QJsonObject object{{"SavePurseResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadPurse(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString InstrumentDefinitionID,
    QString NymID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().LoadPurse(
        NotaryID.toStdString(),
        InstrumentDefinitionID.toStdString(),
        NymID.toStdString());
    QJsonObject object{{"Purse", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::purseGetTotalValue(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString InstrumentDefinitionID,
    QString Purse)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    qint64 result = opentxs::OT::App().API().Exec().Purse_GetTotalValue(
        NotaryID.toStdString(),
        InstrumentDefinitionID.toStdString(),
        Purse.toStdString());
    QJsonObject object{{"PurseTotalValue", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::purseCount(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString InstrumentDefinitionID,
    QString Purse)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OT::App().API().Exec().Purse_Count(
        NotaryID.toStdString(),
        InstrumentDefinitionID.toStdString(),
        Purse.toStdString());
    QJsonObject object{{"PurseCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::purseHasPassword(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString Purse)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().Purse_HasPassword(
        NotaryID.toStdString(), Purse.toStdString());
    QJsonObject object{{"PurseHasPassword", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::createPurse(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString InstrumentDefinitionID,
    QString OwnerID,
    QString SignerID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(OwnerID.toStdString())) {
        QJsonObject object{{"Error", "Invalid OwnerID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SignerID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SignerID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().CreatePurse(
        NotaryID.toStdString(),
        InstrumentDefinitionID.toStdString(),
        OwnerID.toStdString(),
        SignerID.toStdString());
    QJsonObject object{{"Purse", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::createPursePassphrase(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString InstrumentDefinitionID,
    QString SignerID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SignerID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SignerID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().CreatePurse_Passphrase(
        NotaryID.toStdString(),
        InstrumentDefinitionID.toStdString(),
        SignerID.toStdString());
    QJsonObject object{{"PursePassphrase", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::pursePeek(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString InstrumentDefinitionID,
    QString OwnerID,
    QString Purse)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(OwnerID.toStdString())) {
        QJsonObject object{{"Error", "Invalid OwnerID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Purse_Peek(
        NotaryID.toStdString(),
        InstrumentDefinitionID.toStdString(),
        OwnerID.toStdString(),
        Purse.toStdString());
    QJsonObject object{{"PursePeek", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::pursePop(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString InstrumentDefinitionID,
    QString OwnerOrSignerID,
    QString Purse)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            OwnerOrSignerID.toStdString())) {
        QJsonObject object{{"Error", "Invalid OwnerID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Purse_Pop(
        NotaryID.toStdString(),
        InstrumentDefinitionID.toStdString(),
        OwnerOrSignerID.toStdString(),
        Purse.toStdString());
    QJsonObject object{{"PursePop", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::purseEmpty(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString InstrumentDefinitionID,
    QString SignerID,
    QString Purse)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SignerID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SignerID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Purse_Empty(
        NotaryID.toStdString(),
        InstrumentDefinitionID.toStdString(),
        SignerID.toStdString(),
        Purse.toStdString());
    QJsonObject object{{"PurseEmpty", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::pursePush(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString InstrumentDefinitionID,
    QString SignerID,
    QString OwnerID,
    QString Purse,
    QString Token)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SignerID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SignerID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(OwnerID.toStdString())) {
        QJsonObject object{{"Error", "Invalid OwnerID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Purse_Push(
        NotaryID.toStdString(),
        InstrumentDefinitionID.toStdString(),
        SignerID.toStdString(),
        OwnerID.toStdString(),
        Purse.toStdString(),
        Token.toStdString());
    QJsonObject object{{"PursePurse", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletImportPurse(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString InstrumentDefinitionID,
    QString NymID,
    QString Purse)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().Wallet_ImportPurse(
        NotaryID.toStdString(),
        InstrumentDefinitionID.toStdString(),
        NymID.toStdString(),
        Purse.toStdString());
    QJsonObject object{{"WalletImportPurseResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::tokenChangeOwner(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString InstrumentDefinitionID,
    QString Token,
    QString SignerNymID,
    QString OldOwner,
    QString NewOwner)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(SignerNymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Token_ChangeOwner(
        NotaryID.toStdString(),
        InstrumentDefinitionID.toStdString(),
        Token.toStdString(),
        SignerNymID.toStdString(),
        OldOwner.toStdString(),
        NewOwner.toStdString());
    QJsonObject object{{"TokenChangerOwnerResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::tokenGetID(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString InstrumentDefinitionID,
    QString Token)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Token_GetID(
        NotaryID.toStdString(),
        InstrumentDefinitionID.toStdString(),
        Token.toStdString());
    QJsonObject object{{"TokenID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::tokenGetDenomination(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString InstrumentDefinitionID,
    QString Token)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    qint64 result = opentxs::OT::App().API().Exec().Token_GetDenomination(
        NotaryID.toStdString(),
        InstrumentDefinitionID.toStdString(),
        Token.toStdString());
    QJsonObject object{{"TokenDenomination", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::tokenGetSeries(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString InstrumentDefinitionID,
    QString Token)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OT::App().API().Exec().Token_GetSeries(
        NotaryID.toStdString(),
        InstrumentDefinitionID.toStdString(),
        Token.toStdString());
    QJsonObject object{{"TokenSeries", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::tokenGetValidFrom(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString InstrumentDefinitionID,
    QString Token)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    time64_t result = opentxs::OT::App().API().Exec().Token_GetValidFrom(
        NotaryID.toStdString(),
        InstrumentDefinitionID.toStdString(),
        Token.toStdString());
    QJsonObject object{
        {"TokenValidFrom", QString(std::to_string(result).c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::tokenGetValidTo(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString InstrumentDefinitionID,
    QString Token)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    time64_t result = opentxs::OT::App().API().Exec().Token_GetValidTo(
        NotaryID.toStdString(),
        InstrumentDefinitionID.toStdString(),
        Token.toStdString());
    QJsonObject object{
        {"TokenValidTo", QString(std::to_string(result).c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::tokenGetInstrumentDefinitionID(
    QString Username,
    QString APIKey,
    QString Token)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().Token_GetInstrumentDefinitionID(
            Token.toStdString());
    QJsonObject object{
        {"TokenInstrumentDefinitionID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::tokenGetNotaryID(
    QString Username,
    QString APIKey,
    QString Token)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().Token_GetNotaryID(Token.toStdString());
    QJsonObject object{{"TokenNotaryID", QString(result.c_str())}};
    return QJsonValue(object);
}
#endif  // OT_CASH

QJsonValue MCRPCService::isBasketCurrency(
    QString Username,
    QString APIKey,
    QString InstrumentDefinitionID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().IsBasketCurrency(
        InstrumentDefinitionID.toStdString());
    QJsonObject object{{"IsBasketCurrency", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::basketGetMemberCount(
    QString Username,
    QString APIKey,
    QString InstrumentDefinitionID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().Basket_GetMemberCount(
        InstrumentDefinitionID.toStdString());
    QJsonObject object{{"BasketMemberCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::basketGetMemberType(
    QString Username,
    QString APIKey,
    QString BasketInstrumentDefinitionID,
    int Index)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            BasketInstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid BasketInstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Basket_GetMemberType(
        BasketInstrumentDefinitionID.toStdString(), Index);
    QJsonObject object{{"BasketMemberType", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::basketGetMinimumTransferAmount(
    QString Username,
    QString APIKey,
    QString BasketInstrumentDefinitionID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            BasketInstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid BasketInstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    qint64 result =
        opentxs::OT::App().API().Exec().Basket_GetMinimumTransferAmount(
            BasketInstrumentDefinitionID.toStdString());
    QJsonObject object{{"BasketMinimumTransferAmount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::basketGetMemberMinimumTransferAmount(
    QString Username,
    QString APIKey,
    QString BasketInstrumentDefinitionID,
    int Index)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            BasketInstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid BasketInstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    qint64 result =
        opentxs::OT::App().API().Exec().Basket_GetMemberMinimumTransferAmount(
            BasketInstrumentDefinitionID.toStdString(), Index);
    QJsonObject object{{"BasketMemberMinimumTransferAmount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetUsageCredits(
    QString Username,
    QString APIKey,
    QString Message)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    qint64 result = opentxs::OT::App().API().Exec().Message_GetUsageCredits(
        Message.toStdString());
    QJsonObject object{{"MessageUsageCredits", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::generateBasketCreation(
    QString Username,
    QString APIKey,
    QString NymID,
    QString Shortname,
    QString Name,
    QString Symbol,
    QString Terms,
    qint64 Weight)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().GenerateBasketCreation(
        NymID.toStdString(),
        Shortname.toStdString(),
        Name.toStdString(),
        Symbol.toStdString(),
        Terms.toStdString(),
        Weight);
    QJsonObject object{{"BasketCreation", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::addBasketCreationItem(
    QString Username,
    QString APIKey,
    QString Basket,
    QString InstrumentDefinitionID,
    qint64 MinimumTransfer)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().AddBasketCreationItem(
        Basket.toStdString(),
        InstrumentDefinitionID.toStdString(),
        MinimumTransfer);
    QJsonObject object{
        {"AddBasketCreationItemResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::generateBasketExchange(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString BasketInstrumentDefinitionID,
    QString BasketAssetAccountID,
    int TransferMultiple)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            BasketInstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid BasketInstrumentDefinitionID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            BasketAssetAccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid BasketAssetAccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().GenerateBasketExchange(
        NotaryID.toStdString(),
        NymID.toStdString(),
        BasketInstrumentDefinitionID.toStdString(),
        BasketAssetAccountID.toStdString(),
        TransferMultiple);
    QJsonObject object{{"BasketExchange", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::addBasketExchangeItem(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString Basket,
    QString InstrumentDefinitionID,
    QString AssetAccountID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(
            InstrumentDefinitionID.toStdString())) {
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AssetAccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AssetAccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().AddBasketExchangeItem(
        NotaryID.toStdString(),
        NymID.toStdString(),
        Basket.toStdString(),
        InstrumentDefinitionID.toStdString(),
        AssetAccountID.toStdString());
    QJsonObject object{
        {"AddBasketExchangeItemResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getSentMessage(
    QString Username,
    QString APIKey,
    qint64 RequestNumber,
    QString NotaryID,
    QString NymID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().GetSentMessage(
        RequestNumber, NotaryID.toStdString(), NymID.toStdString());
    QJsonObject object{{"SentMessage", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::removeSentMessage(
    QString Username,
    QString APIKey,
    qint64 RequestNumber,
    QString NotaryID,
    QString NymID)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().RemoveSentMessage(
        RequestNumber, NotaryID.toStdString(), NymID.toStdString());
    QJsonObject object{{"RemoveSentMessageResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::sleep(
    QString Username,
    QString APIKey,
    qint64 Milliseconds)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    opentxs::Log::Sleep(std::chrono::milliseconds(Milliseconds));
    QJsonObject object{{"Sleep", Milliseconds}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::resyncNymWithServer(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString Message)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().ResyncNymWithServer(
        NotaryID.toStdString(), NymID.toStdString(), Message.toStdString());
    QJsonObject object{{"ResyncNymWithServerResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetPayload(
    QString Username,
    QString APIKey,
    QString Message)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().Message_GetPayload(Message.toStdString());
    QJsonObject object{{"MessagePayload", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetCommand(
    QString Username,
    QString APIKey,
    QString Message)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().Message_GetCommand(Message.toStdString());
    QJsonObject object{{"MessageCommand", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetLedger(
    QString Username,
    QString APIKey,
    QString Message)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().Message_GetLedger(Message.toStdString());
    QJsonObject object{{"MessageLedger", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetNewInstrumentDefinitionID(
    QString Username,
    QString APIKey,
    QString Message)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().Message_GetNewInstrumentDefinitionID(
            Message.toStdString());
    QJsonObject object{
        {"MessageNewInstrumentDefinitionID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetNewIssuerAccountID(
    QString Username,
    QString APIKey,
    QString Message)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result =
        opentxs::OT::App().API().Exec().Message_GetNewIssuerAcctID(
            Message.toStdString());
    QJsonObject object{{"MessageNewIssuerAccountID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetNewAccountID(
    QString Username,
    QString APIKey,
    QString Message)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Message_GetNewAcctID(
        Message.toStdString());
    QJsonObject object{{"MessageNewAccountID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetNymboxHash(
    QString Username,
    QString APIKey,
    QString Message)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OT::App().API().Exec().Message_GetNymboxHash(
        Message.toStdString());
    QJsonObject object{{"MessageNymboxHash", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetSuccess(
    QString Username,
    QString APIKey,
    QString Message)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    bool result =
        opentxs::OT::App().API().Exec().Message_GetSuccess(Message.toStdString());
    QJsonObject object{{"MessageNymboxHash", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetDepth(
    QString Username,
    QString APIKey,
    QString Message)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int result =
        opentxs::OT::App().API().Exec().Message_GetDepth(Message.toStdString());
    QJsonObject object{{"MessageDepth", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageIsTransactionCanceled(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID,
    QString Message)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().Message_IsTransactionCanceled(
        NotaryID.toStdString(),
        NymID.toStdString(),
        AccountID.toStdString(),
        Message.toStdString());
    QJsonObject object{{"MessageIsTransactionCanceledResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetTransactionSuccess(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString NymID,
    QString AccountID,
    QString Message)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NotaryID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(NymID.toStdString())) {
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(AccountID.toStdString())) {
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OT::App().API().Exec().Message_GetTransactionSuccess(
        NotaryID.toStdString(),
        NymID.toStdString(),
        AccountID.toStdString(),
        Message.toStdString());
    QJsonObject object{{"MessageTransactionSuccess", result}};
    return QJsonValue(object);
}

// Moneychanger::It() methods

QString MCRPCService::mcSendDialog(
    QString Username,
    QString APIKey,
    QString Account,
    QString Recipient,
    QString Asset,
    QString Amount)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QString("Error: Invalid API Key");
    }
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

    Moneychanger::It()->mc_rpc_sendfunds_show_dialog(
        Account, Recipient, Asset, Amount);
    return "Success";
}

QString MCRPCService::mcRequestFundsDialog(
    QString Username,
    QString APIKey,
    QString Account,
    QString Recipient,
    QString Asset,
    QString Amount)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QString("Error: Invalid API Key");
    }

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

    Moneychanger::It()->mc_rpc_requestfunds_show_dialog(
        Account, Recipient, Asset, Amount);
    return "Success";
}

QString MCRPCService::mcMessagesDialog(QString Username, QString APIKey)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QString("Error: Invalid API Key");
    }

    Moneychanger::It()->mc_rpc_messages_show_dialog();

    return "Success";
}

QString MCRPCService::mcExchangeDialog(QString Username, QString APIKey)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QString("Error: Invalid API Key");
    }

    Moneychanger::It()->mc_rpc_exchange_show_dialog();
    return "Success";
}

QString MCRPCService::mcPaymentsDialog(QString Username, QString APIKey)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QString("Error: Invalid API Key");
    }

    return "Not yet implemented";
}

QString MCRPCService::mcManageAccountsDialog(QString Username, QString APIKey)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QString("Error: Invalid API Key");
    }

    Moneychanger::It()->mc_rpc_manage_accounts_show_dialog();
    return "Success";
}

QString MCRPCService::mcManageNymsDialog(QString Username, QString APIKey)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QString("Error: Invalid API Key");
    }

    Moneychanger::It()->mc_rpc_manage_nyms_show_dialog();
    return "Success";
}

QString MCRPCService::mcManageAssetsDialog(QString Username, QString APIKey)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QString("Error: Invalid API Key");
    }

    Moneychanger::It()->mc_rpc_manage_assets_show_dialog();
    return "Success";
}

QString MCRPCService::mcManageSmartContractsDialog(
    QString Username,
    QString APIKey)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QString("Error: Invalid API Key");
    }

    Moneychanger::It()->mc_rpc_manage_smartcontracts_show_dialog();
    return "Success";
}

bool MCRPCService::mcActivateSmartContract(QString Username, QString APIKey)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return false;
    }
    // This function should take the ID of a smart contract
    // And launch the Smart Contracts wizard accordingly
    return false;
}

QJsonValue MCRPCService::mcListSmartContracts(QString Username, QString APIKey)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    QString Contracts("Placeholder");
    QJsonObject object{{"SmartContractsList", Contracts}};

    return QJsonValue(object);
}

// RecordList Methods

QJsonValue MCRPCService::recordListPopulate(QString Username, QString APIKey)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    if (m_RecordList == nullptr) createRecordList(Username, APIKey);

    int nServerCount = opentxs::OT::App().API().Exec().GetServerCount();
    int nAssetCount = opentxs::OT::App().API().Exec().GetAssetTypeCount();
    int nNymCount = opentxs::OT::App().API().Exec().GetNymCount();
    int nAccountCount = opentxs::OT::App().API().Exec().GetAccountCount();
    // ----------------------------------------------------
    for (int ii = 0; ii < nServerCount; ++ii) {
        std::string NotaryID = opentxs::OT::App().API().Exec().GetServer_ID(ii);
        m_RecordList->AddNotaryID(NotaryID);
    }
    // ----------------------------------------------------
    for (int ii = 0; ii < nAssetCount; ++ii) {
        std::string InstrumentDefinitionID =
            opentxs::OT::App().API().Exec().GetAssetType_ID(ii);
        m_RecordList->AddInstrumentDefinitionID(InstrumentDefinitionID);
    }
    // ----------------------------------------------------
    for (int ii = 0; ii < nNymCount; ++ii) {
        std::string nymId = opentxs::OT::App().API().Exec().GetNym_ID(ii);
        m_RecordList->AddNymID(nymId);
    }
    // ----------------------------------------------------
    for (int ii = 0; ii < nAccountCount; ++ii) {
        std::string accountID =
            opentxs::OT::App().API().Exec().GetAccountWallet_ID(ii);
        m_RecordList->AddAccountID(accountID);
    }
    // ----------------------------------------------------
    m_RecordList->AcceptChequesAutomatically(true);
    m_RecordList->AcceptReceiptsAutomatically(true);
    m_RecordList->AcceptTransfersAutomatically(false);

    m_RecordList->Populate();

    QJsonObject object{{"RecordListPopulated", "True"}};

    return object;
}

QJsonValue MCRPCService::recordListCount(QString Username, QString APIKey)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    QJsonObject reply_obj;
    if (m_RecordList == nullptr) {
        qDebug() << QString("Record List Null");
        QJsonValue reply_val = recordListPopulate(Username, APIKey);
        reply_obj.insert("RecordListPopulated", reply_val);
    }

    if (m_RecordList == nullptr) {
        QJsonObject object{{"Error", "Recordlist is a null pointer."}};
        return object;
    }
    /*
    if (QJsonValue("True") != reply_obj.value("RecordListPopulated"))
    {
        qDebug() << QString("Recordlist didn't populate properly:
    %1").arg(reply_obj.value("RecordListPopulated").toString());
        QJsonObject object{{"Error", QString("Recordlist didn't populate
    properly: %1").arg(reply_obj.value("RecordListPopulated").toString())}};
        return object;
    }
*/
    m_RecordList->Populate();
    int count = m_RecordList->size();

    QJsonObject object{{"RecordListCount", count}};

    return object;
}

QJsonValue MCRPCService::recordListRetrieve(
    QString Username,
    QString APIKey,
    int Index)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    // if(m_RecordList == nullptr)
    //    recordListPopulate(Username, APIKey);
    m_RecordList->Populate();
    int count = m_RecordList->size();

    if (Index >= count) Index = count;

    if (Index < 0) Index = 0;

    QJsonObject object;

    QJsonObject record{
        {"AccountID",
         QString(m_RecordList->GetRecord(Index).GetAccountID().c_str())},
        {"Address",
         QString(m_RecordList->GetRecord(Index).GetAddress().c_str())},
        {"Amount", QString(m_RecordList->GetRecord(Index).GetAmount().c_str())},
        {"BoxIndex", m_RecordList->GetRecord(Index).GetBoxIndex()},
        {"Contents",
         QString(m_RecordList->GetRecord(Index).GetContents().c_str())},
        {"CurrencyTLA",
         QString(m_RecordList->GetRecord(Index).GetCurrencyTLA().c_str())},
        {"Date", QString(m_RecordList->GetRecord(Index).GetDate().c_str())},
        {"InitialPaymentAmount",
         qint64(m_RecordList->GetRecord(Index).GetInitialPaymentAmount())},
        {"InitialPaymentDate",
         qint64(m_RecordList->GetRecord(Index).GetInitialPaymentDate())},
        {"UnitType",
         QString(m_RecordList->GetRecord(Index)
                     .GetUnitTypeID()
                     .c_str())},
        {"InstrumentType",
         QString(m_RecordList->GetRecord(Index).GetInstrumentType().c_str())},
        {"MaximumNoPayments",
         m_RecordList->GetRecord(Index).GetMaximumNoPayments()},
        {"Memo", QString(m_RecordList->GetRecord(Index).GetMemo().c_str())},
        {"MethodID", m_RecordList->GetRecord(Index).GetMethodID()},
        {"MesssageID",
         QString(m_RecordList->GetRecord(Index).GetMsgID().c_str())},
        {"MessageType",
         QString(m_RecordList->GetRecord(Index).GetMsgType().c_str())},
        {"MessageTypeDisplay",
         QString(m_RecordList->GetRecord(Index).GetMsgTypeDisplay().c_str())},
        {"Name", QString(m_RecordList->GetRecord(Index).GetName().c_str())},
        {"MsgNotaryID",
         QString(m_RecordList->GetRecord(Index).GetMsgNotaryID().c_str())},
        {"PmntNotaryID",
         QString(m_RecordList->GetRecord(Index).GetPmntNotaryID().c_str())},
        {"NymID", QString(m_RecordList->GetRecord(Index).GetNymID().c_str())},
        {"OtherAccountID",
         QString(m_RecordList->GetRecord(Index).GetOtherAccountID().c_str())},
        {"OtherAddress",
         QString(m_RecordList->GetRecord(Index).GetOtherAddress().c_str())},
        {"OtherNymID",
         QString(m_RecordList->GetRecord(Index).GetOtherNymID().c_str())},
        {"PaymentPlanAmount",
         qint64(m_RecordList->GetRecord(Index).GetPaymentPlanAmount())},
        {"PaymentPlanStartDate",
         qint64(m_RecordList->GetRecord(Index).GetPaymentPlanStartDate())},
        {"RecordType", m_RecordList->GetRecord(Index).GetRecordType()},
        {"TimeBetweenPayments",
         qint64(m_RecordList->GetRecord(Index).GetTimeBetweenPayments())},
        {"TransactionNum",
         qint64(m_RecordList->GetRecord(Index).GetTransactionNum())},
        {"TransNumForDisplay",
         qint64(m_RecordList->GetRecord(Index).GetTransNumForDisplay())},
        {"ValidFrom", qint64(m_RecordList->GetRecord(Index).GetValidFrom())},
        {"ValidTo", qint64(m_RecordList->GetRecord(Index).GetValidTo())}};

    object.insert(QString(Index), record);

    return object;
}

QJsonValue MCRPCService::recordListRetrieve(
    QString Username,
    QString APIKey,
    int BeginIndex,
    int EndIndex)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    // enum OTRecordType { Mail = 0, Transfer, Receipt, Instrument,ErrorState };

    QJsonObject reply_obj;
    if (m_RecordList == nullptr) {
        QJsonValue reply_val = recordListPopulate(Username, APIKey);
        reply_obj.insert("RecordListPopulated", reply_val);
    }

    if (m_RecordList == nullptr) {
        QJsonObject object{{"Error", "Recordlist is a null pointer."}};
        return object;
    }

    /*if (QJsonValue("True") != reply_obj.value("RecordListPopulated"))
    {
        qDebug() << QString("Recordlist didn't populate properly:
    %1").arg(reply_obj.value("RecordListPopulated").toString());
        QJsonObject object{{"Error", QString("Recordlist didn't populate
    properly: %1").arg(reply_obj.value("RecordListPopulated").toString())}};
        return object;
    }*/

    m_RecordList->Populate();
    int count = m_RecordList->size();

    // Swap if Begin > End
    if (BeginIndex > EndIndex) {
        BeginIndex ^= EndIndex;
        EndIndex ^= BeginIndex;
        BeginIndex ^= EndIndex;
    }

    if (BeginIndex < 0) BeginIndex = 0;

    if (BeginIndex > count) {
        QJsonObject object{{"Error", "Out of Bound Request"}};
        return object;
    }

    if (EndIndex > count) EndIndex = count;

    QJsonObject object;
    for (auto x = BeginIndex; x < EndIndex; x++) {

        QJsonObject record{
            {"AccountID",
             QString(m_RecordList->GetRecord(x).GetAccountID().c_str())},
            {"Address",
             QString(m_RecordList->GetRecord(x).GetAddress().c_str())},
            {"Amount", QString(m_RecordList->GetRecord(x).GetAmount().c_str())},
            {"BoxIndex", m_RecordList->GetRecord(x).GetBoxIndex()},
            {"Contents",
             QString(m_RecordList->GetRecord(x).GetContents().c_str())},
            {"CurrencyTLA",
             QString(m_RecordList->GetRecord(x).GetCurrencyTLA().c_str())},
            {"Date", QString(m_RecordList->GetRecord(x).GetDate().c_str())},
            {"InitialPaymentAmount",
             qint64(m_RecordList->GetRecord(x).GetInitialPaymentAmount())},
            {"InitialPaymentDate",
             qint64(m_RecordList->GetRecord(x).GetInitialPaymentDate())},
            {"UnitTypeID",
             QString(m_RecordList->GetRecord(x)
                         .GetUnitTypeID()
                         .c_str())},
            {"InstrumentType",
             QString(m_RecordList->GetRecord(x).GetInstrumentType().c_str())},
            {"MaximumNoPayments",
             m_RecordList->GetRecord(x).GetMaximumNoPayments()},
            {"Memo", QString(m_RecordList->GetRecord(x).GetMemo().c_str())},
            {"MethodID", m_RecordList->GetRecord(x).GetMethodID()},
            {"MesssageID",
             QString(m_RecordList->GetRecord(x).GetMsgID().c_str())},
            {"MessageType",
             QString(m_RecordList->GetRecord(x).GetMsgType().c_str())},
            {"MessageTypeDisplay",
             QString(m_RecordList->GetRecord(x).GetMsgTypeDisplay().c_str())},
            {"Name", QString(m_RecordList->GetRecord(x).GetName().c_str())},
            {"MsgNotaryID",
             QString(m_RecordList->GetRecord(x).GetMsgNotaryID().c_str())},
            {"PmntNotaryID",
             QString(m_RecordList->GetRecord(x).GetPmntNotaryID().c_str())},
            {"NymID", QString(m_RecordList->GetRecord(x).GetNymID().c_str())},
            {"OtherAccountID",
             QString(m_RecordList->GetRecord(x).GetOtherAccountID().c_str())},
            {"OtherAddress",
             QString(m_RecordList->GetRecord(x).GetOtherAddress().c_str())},
            {"OtherNymID",
             QString(m_RecordList->GetRecord(x).GetOtherNymID().c_str())},
            {"PaymentPlanAmount",
             qint64(m_RecordList->GetRecord(x).GetPaymentPlanAmount())},
            {"PaymentPlanStartDate",
             qint64(m_RecordList->GetRecord(x).GetPaymentPlanStartDate())},
            {"RecordType", m_RecordList->GetRecord(x).GetRecordType()},
            {"TimeBetweenPayments",
             qint64(m_RecordList->GetRecord(x).GetTimeBetweenPayments())},
            {"TransactionNum",
             qint64(m_RecordList->GetRecord(x).GetTransactionNum())},
            {"TransNumForDisplay",
             qint64(m_RecordList->GetRecord(x).GetTransNumForDisplay())},
            {"ValidFrom", qint64(m_RecordList->GetRecord(x).GetValidFrom())},
            {"ValidTo", qint64(m_RecordList->GetRecord(x).GetValidTo())}};

        object.insert(QString(x), record);
    }

    return object;
}

bool MCRPCService::createRecordList(QString Username, QString APIKey)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return false;
    }

    if (m_RecordList == nullptr)
        m_RecordList = new opentxs::OTRecordList(*(new MTNameLookupQT));
    qDebug() << QString("Record List Created");
    return true;
}

QJsonValue MCRPCService::setDefaultNym(
    QString Username,
    QString APIKey,
    QString NymID,
    QString NymName)
{

    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QString("Error: Invalid API Key");
    }

    Moneychanger::It()->setDefaultNym(NymID, NymName);
    return "Success";
}

QJsonValue MCRPCService::getDefaultNym(QString Username, QString APIKey)
{

    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QString("Error: Invalid API Key");
    }

    QJsonObject record{{"NymID", Moneychanger::It()->getDefaultNymID()},
                       {"NymName", Moneychanger::It()->getDefaultNymName()}};

    return record;
}

QJsonValue MCRPCService::setDefaultAccount(
    QString Username,
    QString APIKey,
    QString AccountID,
    QString AccountName)
{

    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QString("Error: Invalid API Key");
    }

    Moneychanger::It()->setDefaultAccount(AccountID, AccountName);
    return "Success";
}

QJsonValue MCRPCService::getDefaultAccount(QString Username, QString APIKey)
{

    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QString("Error: Invalid API Key");
    }

    QJsonObject record{
        {"AccountID", Moneychanger::It()->getDefaultAccountID()},
        {"AccountName", Moneychanger::It()->getDefaultAccountName()}};

    return record;
}

QJsonValue MCRPCService::setDefaultServer(
    QString Username,
    QString APIKey,
    QString NotaryID,
    QString ServerName)
{

    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QString("Error: Invalid API Key");
    }

    Moneychanger::It()->setDefaultServer(NotaryID, ServerName);
    return "Success";
}

QJsonValue MCRPCService::getDefaultServer(QString Username, QString APIKey)
{

    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QString("Error: Invalid API Key");
    }

    QJsonObject record{
        {"NotaryID", Moneychanger::It()->getDefaultNotaryID()},
        {"ServerName", Moneychanger::It()->getDefaultServerName()}};

    return record;
}

QJsonValue MCRPCService::setDefaultAsset(
    QString Username,
    QString APIKey,
    QString AssetID,
    QString AssetName)
{

    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QString("Error: Invalid API Key");
    }

    Moneychanger::It()->setDefaultServer(AssetID, AssetName);
    return "Success";
}

QJsonValue MCRPCService::getDefaultAsset(QString Username, QString APIKey)
{

    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QString("Error: Invalid API Key");
    }

    QJsonObject record{
        {"AssetID", Moneychanger::It()->getDefaultAssetID()},
        {"AssetName", Moneychanger::It()->getDefaultAssetName()}};
    return record;
}

QJsonValue MCRPCService::isValidID(QString ID, QString Username, QString APIKey)
{
    if (!validateAPIKey(Username, APIKey)) {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if (!opentxs::OT::App().API().Exec().IsValidID(ID.toStdString())) {
        QJsonObject object{{"IsValidID", "False"}};
        return QJsonValue(object);
    } else {
        QJsonObject object{{"IsValidID", "True"}};
        return QJsonValue(object);
    }
}

QJsonValue MCRPCService::userLogin(QString Username, QString PlaintextPassword)
{
    if (m_userManager.activateUserAccount(Username, PlaintextPassword)) {
        QString userKey;
        userKey = m_userManager.getAPIKey(Username);
        QJsonObject object{{"Success", "User Activated"}, {"APIKey", userKey}};
        return object;
    } else {
        QJsonObject object{{"Error", "User Login Failed"}};
        return object;
    }
}

QJsonValue MCRPCService::userLogout(QString Username, QString PlaintextPassword)
{
    if (m_userManager.validateUserInDatabase(Username, PlaintextPassword)) {
        m_userManager.deactivateUserAccount(Username);
        QJsonObject object{{"Success", "User Logged Out"}};
        return object;
    } else {
        QJsonObject object{{"Error", "User Credentials Not Validated"}};
        return object;
    }
}

QJsonValue MCRPCService::refreshAPIKey(
    QString Username,
    QString PlaintextPassword)
{
    if (!m_userManager.checkUserActivated(Username)) {
        QJsonObject object{{"Error", "User Not Logged In"}};
        return object;
    } else {
        if (m_userManager.validateUserInDatabase(Username, PlaintextPassword)) {
            QString l_apiKey = m_userManager.getAPIKey(Username);
            QJsonObject object{{"Username", Username}, {"APIKey", l_apiKey}};
            return object;
        } else {
            QJsonObject object{{"Error", "User Credentials Invalid"}};
            return object;
        }
    }
}

bool MCRPCService::validateAPIKey(QString Username, QString APIKey)
{
    if (!m_userManager.checkUserActivated(Username)) {
        return false;
    } else {
        if (m_userManager.validateAPIKey(Username, APIKey)) {
            return true;
        } else {
            return false;
        }
    }
}
