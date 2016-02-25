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
#include <opentxs/client/OT_ME.hpp>
#include <opentxs/core/Log.hpp>

#include <core/moneychanger.hpp>
#include <core/handlers/contacthandler.hpp>


MCRPCService::MCRPCService(QObject *parent)
    : QJsonRpcService(parent)//, m_RecordList(nullptr)
{
    QThreadPool::globalInstance()->setMaxThreadCount(1);
}

MCRPCService::~MCRPCService(){

    if(m_RecordList != nullptr)
        delete m_RecordList;

}

QJsonValue MCRPCService::createNymEcdsa(QString Username, QString APIKey,
                                        QString NymIDSource)
{
    if (!validateAPIKey(Username, APIKey))
    {
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    std::string result = opentxs::OTAPI_Wrap::It()->CreateNymECDSA(
                                                              NymIDSource.toStdString());
    QJsonObject object{{"CreateNymEcdsaResult", QString(result.c_str())}};
    return QJsonValue(object);
}


QJsonValue MCRPCService::registerAccount(QString Username, QString APIKey,
                                         QString NotaryID, QString NymID,
                                         QString InstrumentDefinitionID)
{

    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    opentxs::OT_ME madeEasy;

    std::string result = madeEasy.create_asset_acct(NotaryID.toStdString(), NymID.toStdString(), InstrumentDefinitionID.toStdString());

    if (opentxs::OTAPI_Wrap::networkFailure())
    {
//      emit appendToLog(qstrErrorMsg); // TODO!
        qDebug() << "Network failure while trying to create asset account.";

        QJsonObject object{{"Error", "Network failure"}};
        return QJsonValue(object);
    }

//  std::cout << "result is " << result << std::endl;

    // -1 error, 0 failure, 1 success.
    //
    if (1 != madeEasy.VerifyMessageSuccess(result))
    {
        const int64_t lUsageCredits = Moneychanger::It()->HasUsageCredits(NotaryID, NymID);

        // HasUsageCredits already pops up an error box in the cases of -2 and 0.
        //
        if (((-2) != lUsageCredits) && (0 != lUsageCredits))
        {
            QJsonObject object{{"Error", "Out of usage credits."}};
            return QJsonValue(object);
        }
    }
    // ------------------------------------------------------
    // Get the ID of the new account.
    //
    QString qstrID = QString::fromStdString(opentxs::OTAPI_Wrap::It()->Message_GetNewAcctID(result));

    if (qstrID.isEmpty())
    {
        QJsonObject object{{"Error", "Failed to get new account ID from server response."}};
        return QJsonValue(object);
    }
    // ------------------------------------------------------
    QJsonObject object{{"CreateAccountResult", qstrID}};
    return QJsonValue(object);
}


QJsonValue MCRPCService::numListAdd(QString Username, QString APIKey,
                                    QString NumList, QString Numbers)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->NumList_Add(NumList.toStdString(),
                                                                Numbers.toStdString());
    QJsonObject object{{"NumListAddResult",  QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::numListRemove(QString Username, QString APIKey,
                                       QString NumList, QString Numbers)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->NumList_Remove(NumList.toStdString(),
                                                                   Numbers.toStdString());
    QJsonObject object{{"NumListRemoveResult",  QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::numListVerifyQuery(QString Username, QString APIKey,
                                            QString NumList, QString Numbers)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->NumList_VerifyQuery(NumList.toStdString(),
                                                                 Numbers.toStdString());
    QJsonObject object{{"NumListVerifyQueryResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::numListVerifyAll(QString Username, QString APIKey,
                                          QString NumList, QString Numbers)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->NumList_VerifyAll(NumList.toStdString(),
                                                               Numbers.toStdString());
    QJsonObject object{{"NumListVerifyAllResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::numListCount(QString Username, QString APIKey,
                                      QString NumList)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->NumList_Count(NumList.toStdString());
    QJsonObject object{{"NumListCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::createNymLegacy(QString Username, QString APIKey,
                                   int KeySize, QString NymIDSource)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    std::string result = opentxs::OTAPI_Wrap::It()->CreateNymLegacy(KeySize,
                                                              NymIDSource.toStdString());
    QJsonObject object{{"CreateNymLegacyResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymActiveCronItemIDs(QString Username, QString APIKey,
                                                 QString NymID, QString NotaryID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_ActiveCronItemIDs(NymID.toStdString(),
                                                                             NotaryID.toStdString());
    QJsonObject object{{"ActiveCronItemIDs", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getActiveCronItem(QString Username, QString APIKey,
                                           QString NotaryID, qint64 TransNum)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetActiveCronItem(NotaryID.toStdString(),
                                                                      TransNum);
    QJsonObject object{{"ActiveCronItem", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymSourceForID(QString Username, QString APIKey,
                                           QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_SourceForID(NymID.toStdString());
    QJsonObject object{{"NymSourceForID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymDescription(QString Username, QString APIKey,
                                                 QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_Description(NymID.toStdString());
    QJsonObject object{{"NymDescription", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymMasterCredentialCount(QString Username, QString APIKey,
                                               QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->GetNym_MasterCredentialCount(NymID.toStdString());
    QJsonObject object{{"NymCredentialCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymMasterCredentialID(QString Username, QString APIKey,
                                            QString NymID, int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_MasterCredentialID(NymID.toStdString(),
                                                                        Index);
    QJsonObject object{{"NymCredentialID", QString(result.c_str())}};
    return QJsonValue(object);
}


QJsonValue MCRPCService::getNymMasterCredentialContents(QString Username, QString APIKey,
                                                  QString NymID, QString CredentialID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_MasterCredentialContents(NymID.toStdString(),
                                                                              CredentialID.toStdString());
    QJsonObject object{{"NymCredentialContents", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymRevokedCount(QString Username, QString APIKey,
                                            QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->GetNym_RevokedCredCount(NymID.toStdString());
    QJsonObject object{{"NymRevokedCredCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymRevokedCredID(QString Username, QString APIKey,
                                             QString NymID, int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_RevokedCredID(NymID.toStdString(),
                                                                         Index);
    QJsonObject object{{"NymRevokedCredID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymRevokedCredContents(QString Username, QString APIKey,
                                                   QString NymID, QString CredentialID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_RevokedCredContents(NymID.toStdString(),
                                                                               CredentialID.toStdString());
    QJsonObject object{{"NymRevokedCredContents", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymChildCredentialCount(QString Username, QString APIKey,
                                                  QString NymID, QString MasterCredID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(MasterCredID.toStdString())){
        QJsonObject object{{"Error", "Invalid MasterCredID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->GetNym_ChildCredentialCount(NymID.toStdString(),
                                                                      MasterCredID.toStdString());
    QJsonObject object{{"NymChildCredentialCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymChildCredentialID(QString Username, QString APIKey,
                                               QString NymID, QString MasterCredID,
                                               int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(MasterCredID.toStdString())){
        QJsonObject object{{"Error", "Invalid MasterCredID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_ChildCredentialID(NymID.toStdString(),
                                                                           MasterCredID.toStdString(),
                                                                           Index);
    QJsonObject object{{"NymChildCredentialID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymChildCredentialContents(QString Username, QString APIKey,
                                                     QString NymID, QString MasterCredID,
                                                     QString SubCredID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(MasterCredID.toStdString())){
        QJsonObject object{{"Error", "Invalid MasterCredID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SubCredID.toStdString())){
        QJsonObject object{{"Error", "Invalid SubCredID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_ChildCredentialContents(NymID.toStdString(),
                                                                                 MasterCredID.toStdString(),
                                                                                 SubCredID.toStdString());
    QJsonObject object{{"NymChildCredentialContents", QString(result.c_str())}};
    return QJsonValue(object);
}


QJsonValue MCRPCService::revokeChildcredential(QString Username, QString APIKey,
                                             QString NymID, QString MasterCredID,
                                             QString SubCredID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(MasterCredID.toStdString())){
        QJsonObject object{{"Error", "Invalid MasterCredID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SubCredID.toStdString())){
        QJsonObject object{{"Error", "Invalid SubCredID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->RevokeChildCredential(NymID.toStdString(),
                                                                 MasterCredID.toStdString(),
                                                                 SubCredID.toStdString());
    QJsonObject object{{"RevokeChildCredentialResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getSignerNymID(QString Username, QString APIKey,
                                        QString Contract)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetSignerNymID(Contract.toStdString());
    QJsonObject object{{"SignerNymID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::calculateUnitDefinitionID(QString Username, QString APIKey,
                                                  QString Contract)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->CalculateUnitDefinitionID(Contract.toStdString());
    QJsonObject object{{"UnitDefinitionID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::calculateServerContractID(QString Username, QString APIKey,
                                                   QString Contract)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->CalculateServerContractID(Contract.toStdString());
    QJsonObject object{{"ServerContractID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::calculateContractID(QString Username, QString APIKey,
                                             QString Contract)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->CalculateContractID(Contract.toStdString());
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
    QString factor,
    QString power,
    QString fraction)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    std::string result =
        opentxs::OTAPI_Wrap::It()->CreateCurrencyContract(
            NymID.toStdString(),
            shortname.toStdString(),
            terms.toStdString(),
            names.toStdString(),
            symbol.toStdString(),
            tla.toStdString(),
            stoi(factor.toStdString()),
            stoi(power.toStdString()),
            fraction.toStdString());
    QJsonObject object{{"CreateCurrencyContractResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getServerContract(QString Username, QString APIKey,
                                           QString NotaryID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetServer_Contract(NotaryID.toStdString());
    QJsonObject object{{"ServerContract", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getCurrencyFactor(QString Username, QString APIKey,
                                           QString InstrumentDefinitionID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->GetCurrencyFactor(InstrumentDefinitionID.toStdString());
    QJsonObject object{{"CurrencyFactor", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getCurrencyDecimalPower(QString Username, QString APIKey,
                                                 QString InstrumentDefinitionID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->GetCurrencyDecimalPower(InstrumentDefinitionID.toStdString());
    QJsonObject object{{"CurrencyDecimalPower", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getCurrencyTLA(QString Username, QString APIKey,
                                        QString InstrumentDefinitionID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetCurrencyTLA(InstrumentDefinitionID.toStdString());
    QJsonObject object{{"CurrencyTLA", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getCurrencySymbol(QString Username, QString APIKey,
                                           QString InstrumentDefinitionID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetCurrencySymbol(InstrumentDefinitionID.toStdString());
    QJsonObject object{{"CurrencySymbol", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::stringToAmountLocale(QString Username, QString APIKey,
                                              QString InstrumentDefinitionID, QString Input,
                                              QString ThousandsSep, QString DecimalPoint)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    qint64 result = opentxs::OTAPI_Wrap::It()->StringToAmountLocale(InstrumentDefinitionID.toStdString(),
                                                                    Input.toStdString(),
                                                                    ThousandsSep.toStdString(),
                                                                    DecimalPoint.toStdString());
    QJsonObject object{{"StringToAmountLocaleResult", QString(std::to_string(result).c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::formatAmountLocale(QString Username, QString APIKey,
                                            QString InstrumentDefinitionID, qint64 Amount,
                                            QString ThousandsSep, QString DecimalPoint)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->FormatAmountLocale(InstrumentDefinitionID.toStdString(),
                                                                       Amount,
                                                                       ThousandsSep.toStdString(),
                                                                       DecimalPoint.toStdString());
    QJsonObject object{{"FormatAmountLocaleResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::formatAmountWithoutSymbolLocale(QString Username, QString APIKey,
                                                         QString InstrumentDefinitionID, qint64 Amount,
                                                         QString ThousandsSep, QString DecimalPoint)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->FormatAmountWithoutSymbolLocale(InstrumentDefinitionID.toStdString(),
                                                                                    Amount,
                                                                                    ThousandsSep.toStdString(),
                                                                                    DecimalPoint.toStdString());
    QJsonObject object{{"FormatAmountWithoutSymbolLocaleResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::stringToAmount(QString Username, QString APIKey,
                                        QString InstrumentDefinitionID, QString Input)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    qint64 result = opentxs::OTAPI_Wrap::It()->StringToAmount(InstrumentDefinitionID.toStdString(),
                                                              Input.toStdString());
    QJsonObject object{{"StringToAmountResult", QString(std::to_string(result).c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::formatAmount(QString Username, QString APIKey,
                                      QString InstrumentDefinitionID, qint64 Amount)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->FormatAmount(InstrumentDefinitionID.toStdString(),
                                                                 Amount);
    QJsonObject object{{"FormatAmountResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::formatAmountWithoutSymbol(QString Username, QString APIKey,
                                                   QString InstrumentDefinitionID, qint64 Amount)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->FormatAmountWithoutSymbol(InstrumentDefinitionID.toStdString(),
                                                                              Amount);
    QJsonObject object{{"FormatAmountWithoutSymbolResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAssetTypeContract(QString Username, QString APIKey,
                                              QString InstrumentDefinitionID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    std::string contract = opentxs::OTAPI_Wrap::It()->GetAssetType_Contract(InstrumentDefinitionID.toStdString());
    QJsonObject object{{"AssetTypeContract", QString(contract.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::addServerContract(QString Username, QString APIKey,
                                           QString Contract)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int l_count = opentxs::OTAPI_Wrap::It()->AddServerContract(Contract.toStdString());
    QJsonObject object{{"AddServerContractResult", l_count}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::addUnitDefinition(QString Username, QString APIKey,
                                          QString Contract)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int l_count = opentxs::OTAPI_Wrap::It()->AddUnitDefinition(Contract.toStdString());
    QJsonObject object{{"AddUnitDefinitionResult", l_count}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymCount(QString Username, QString APIKey)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int l_count = opentxs::OTAPI_Wrap::It()->GetNymCount();
    QJsonObject object{{"NymCount", l_count}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getServerCount(QString Username, QString APIKey)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int l_count = opentxs::OTAPI_Wrap::It()->GetServerCount();
    QJsonObject object{{"ServerCount", l_count}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAssetTypeCount(QString Username, QString APIKey)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int l_count = opentxs::OTAPI_Wrap::It()->GetAssetTypeCount();
    QJsonObject object{{"AssetTypeCount", l_count}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAccountCount(QString Username, QString APIKey)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int l_count = opentxs::OTAPI_Wrap::It()->GetAccountCount();
    QJsonObject object{{"AccountCount", l_count}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletCanRemoveServer(QString Username, QString APIKey,
                                               QString NotaryID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->Wallet_CanRemoveServer(NotaryID.toStdString());
    QJsonObject object{{"WalletCanRemoveServerResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletRemoveServer(QString Username, QString APIKey,
                                            QString NotaryID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->Wallet_RemoveServer(NotaryID.toStdString());
    QJsonObject object{{"WalletRemoveServerResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletCanRemoveAssetType(QString Username, QString APIKey,
                                                  QString InstrumentDefinitionID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->Wallet_CanRemoveAssetType(InstrumentDefinitionID.toStdString());
    QJsonObject object{{"WalletCanRemoveAssetTypeResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletRemoveAssetType(QString Username, QString APIKey,
                                               QString InstrumentDefinitionID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->Wallet_RemoveAssetType(InstrumentDefinitionID.toStdString());
    QJsonObject object{{"WalletRemoveAssetTypeResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletCanRemoveNym(QString Username, QString APIKey,
                                            QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->Wallet_CanRemoveNym(NymID.toStdString());
    QJsonObject object{{"WalletCanRemoveNymResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletRemoveNym(QString Username, QString APIKey,
                                         QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->Wallet_RemoveNym(NymID.toStdString());
    QJsonObject object{{"WalletRemoveNymResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletCanRemoveAccount(QString Username, QString APIKey,
                                                QString AccountID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->Wallet_CanRemoveAccount(AccountID.toStdString());
    QJsonObject object{{"WalletCanRemoveAccountResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::doesBoxReceiptExist(QString Username, QString APIKey,
                                             QString NotaryID, QString NymID,
                                             QString AccountID, int BoxType,
                                             qint64 TransactionNumber)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->DoesBoxReceiptExist(NotaryID.toStdString(),
                                                                 NymID.toStdString(),
                                                                 AccountID.toStdString(),
                                                                 BoxType,
                                                                 TransactionNumber);
    QJsonObject object{{"DoesBoxReceiptExistResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getBoxReceipt(QString Username, QString APIKey,
                                       QString NotaryID, QString NymID,
                                       QString AccountID, int BoxType,
                                       qint64 TransactionNumber)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->getBoxReceipt(NotaryID.toStdString(),
                                                          NymID.toStdString(),
                                                          AccountID.toStdString(),
                                                          BoxType,
                                                          TransactionNumber);
    QJsonObject object{{"BoxReceipt", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::deleteAssetAccount(QString Username, QString APIKey,
                                            QString NotaryID, QString NymID,
                                            QString AccountID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->deleteAssetAccount(NotaryID.toStdString(),
                                                               NymID.toStdString(),
                                                               AccountID.toStdString());
    QJsonObject object{{"DeleteAssetAccountResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletExportNym(QString Username, QString APIKey,
                                         QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Wallet_ExportNym(NymID.toStdString());
    QJsonObject object{{"ExportedNym", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletImportNym(QString Username, QString APIKey,
                                         QString FileContents)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Wallet_ImportNym(FileContents.toStdString());
    QJsonObject object{{"WalletImportNymResult", QString(result.c_str())}};
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

QJsonValue MCRPCService::walletGetNymIdFromPartial(QString Username, QString APIKey,
                                                   QString PartialID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    /* Not putting ID check in here for the moment */

    std::string result = opentxs::OTAPI_Wrap::It()->Wallet_GetNymIDFromPartial(PartialID.toStdString());
    QJsonObject object{{"NymIdFromPartial", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletGetNotaryIdFromPartial(QString Username, QString APIKey,
                                                      QString PartialID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Wallet_GetNotaryIDFromPartial(PartialID.toStdString());
    QJsonObject object{{"NotaryIdFromPartial", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletGetInstrumentDefinitionIdFromPartial(QString Username, QString APIKey,
                                                                    QString PartialID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Wallet_GetInstrumentDefinitionIDFromPartial(PartialID.toStdString());
    QJsonObject object{{"InstrumentDefinitionIdFromPartial", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletGetAccountIdFromPartial(QString Username, QString APIKey,
                                                       QString PartialID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Wallet_GetAccountIDFromPartial(PartialID.toStdString());
    QJsonObject object{{"AccountIdFromPartial", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymID(QString Username, QString APIKey,
                                  int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_ID(Index);
    QJsonObject object{{"NymID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymName(QString Username, QString APIKey,
                                    QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_Name(NymID.toStdString());
    QJsonObject object{{"NymName", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::isNymRegisteredAtServer(QString Username, QString APIKey,
                                                 QString NymID, QString NotaryID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->IsNym_RegisteredAtServer(NymID.toStdString(),
                                                                      NotaryID.toStdString());
    QJsonObject object{{"IsNymRegisteredAtServerResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymStats(QString Username, QString APIKey,
                                     QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_Stats(NymID.toStdString());
    QJsonObject object{{"NymStats", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymNymboxHash(QString Username, QString APIKey,
                                          QString NotaryID, QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_NymboxHash(NotaryID.toStdString(),
                                                                      NymID.toStdString());
    QJsonObject object{{"NymNymboxHash", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymRecentHash(QString Username, QString APIKey,
                                          QString NotaryID, QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_RecentHash(NotaryID.toStdString(),
                                                                      NymID.toStdString());
    QJsonObject object{{"NymRecentHash", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymInboxHash(QString Username, QString APIKey,
                                         QString NotaryID, QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_InboxHash(NotaryID.toStdString(),
                                                                     NymID.toStdString());
    QJsonObject object{{"NymInboxHash", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymOutboxHash(QString Username, QString APIKey,
                                          QString NotaryID, QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_OutboxHash(NotaryID.toStdString(),
                                                                      NymID.toStdString());
    QJsonObject object{{"NymOutboxHash", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymMailCount(QString Username, QString APIKey,
                                         QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->GetNym_MailCount(NymID.toStdString());
    QJsonObject object{{"NymMailCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymContentsByIndex(QString Username, QString APIKey,
                                               QString NymID, int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }


    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_MailContentsByIndex(NymID.toStdString(),
                                                                               Index);
    QJsonObject object{{"NymContentsByIndex", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymMailSenderIDByIndex(QString Username, QString APIKey,
                                                   QString NymID, int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }


    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_MailSenderIDByIndex(NymID.toStdString(),
                                                                               Index);
    QJsonObject object{{"NymMailSenderIDByIndex", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymMailNotaryIDByIndex(QString Username, QString APIKey,
                                                   QString NymID, int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }


    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_MailNotaryIDByIndex(NymID.toStdString(),
                                                                               Index);
    QJsonObject object{{"NymMailNotaryIDByIndex", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::nymRemoveMailByIndex(QString Username, QString APIKey,
                                              QString NymID, int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }


    bool result = opentxs::OTAPI_Wrap::It()->Nym_RemoveMailByIndex(NymID.toStdString(),
                                                                   Index);
    QJsonObject object{{"NymRemoveMailByIndexResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::nymVerifyMailByIndex(QString Username, QString APIKey,
                                              QString NymID, int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }


    bool result = opentxs::OTAPI_Wrap::It()->Nym_VerifyMailByIndex(NymID.toStdString(),
                                                                   Index);
    QJsonObject object{{"NymVerifyMailByIndexResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymOutmailCount(QString Username, QString APIKey,
                                            QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }


    int result = opentxs::OTAPI_Wrap::It()->GetNym_OutmailCount(NymID.toStdString());
    QJsonObject object{{"NymOutmailCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymOutmailContentsByIndex(QString Username, QString APIKey,
                                                      QString NymID, int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }


    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_OutmailContentsByIndex(NymID.toStdString(),
                                                                                  Index);
    QJsonObject object{{"NymOutmailContentsIDByIndex", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymOutmailRecipientIDByIndex(QString Username, QString APIKey,
                                                         QString NymID, int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }


    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_OutmailRecipientIDByIndex(NymID.toStdString(),
                                                                                     Index);
    QJsonObject object{{"NymOutmailRecipientIDByIndex", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymOutmailNotaryIDByIndex(QString Username, QString APIKey,
                                                      QString NymID, int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }


    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_OutmailNotaryIDByIndex(NymID.toStdString(),
                                                                                  Index);
    QJsonObject object{{"NymOutmailNotaryIDIDByIndex", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::nymRemoveOutmailByIndex(QString Username, QString APIKey,
                                                 QString NymID, int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }


    bool result = opentxs::OTAPI_Wrap::It()->Nym_RemoveOutmailByIndex(NymID.toStdString(),
                                                                      Index);
    QJsonObject object{{"NymRemoveOutmailByIndexResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::nymVerifyOutmailByIndex(QString Username, QString APIKey,
                                                 QString NymID, int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }


    bool result = opentxs::OTAPI_Wrap::It()->Nym_VerifyOutmailByIndex(NymID.toStdString(),
                                                                      Index);
    QJsonObject object{{"NymVerifyOutmailByIndexResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymOutpaymentsCount(QString Username, QString APIKey,
                                                QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }


    int result = opentxs::OTAPI_Wrap::It()->GetNym_OutpaymentsCount(NymID.toStdString());
    QJsonObject object{{"NymOutpaymentsCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymOutpaymentsContentsByIndex(QString Username, QString APIKey,
                                                          QString NymID, int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }


    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_OutpaymentsContentsByIndex(NymID.toStdString(),
                                                                                      Index);
    QJsonObject object{{"NymOutpaymentsContentsByIndex", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymOutpaymentsRecipientIDByIndex(QString Username, QString APIKey,
                                                             QString NymID, int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }


    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_OutpaymentsRecipientIDByIndex(NymID.toStdString(),
                                                                                         Index);
    QJsonObject object{{"NymOutpaymentsRecipientIDByIndex", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymOutpaymentsNotaryIDByIndex(QString Username, QString APIKey,
                                                          QString NymID, int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }


    std::string result = opentxs::OTAPI_Wrap::It()->GetNym_OutpaymentsNotaryIDByIndex(NymID.toStdString(),
                                                                                      Index);
    QJsonObject object{{"NymOutpaymentsNotaryIDByIndex", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::nymRemoveOutpaymentsByIndex(QString Username, QString APIKey,
                                                     QString NymID, int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }


    bool result = opentxs::OTAPI_Wrap::It()->Nym_RemoveOutpaymentsByIndex(NymID.toStdString(),
                                                                          Index);
    QJsonObject object{{"NymRemoveOutpaymentsByIndexResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::nymVerifyOutpaymentsByIndex(QString Username, QString APIKey,
                                                     QString NymID, int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }


    bool result = opentxs::OTAPI_Wrap::It()->Nym_VerifyOutpaymentsByIndex(NymID.toStdString(),
                                                                          Index);
    QJsonObject object{{"NymVerifyOutpaymentsByIndexResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetAmount(QString Username, QString APIKey,
                                             QString Instrument)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->Instrmnt_GetAmount(Instrument.toStdString());
    QJsonObject object{{"InstrumentAmount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetTransactionNumber(QString Username, QString APIKey,
                                                        QString Instrument)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->Instrmnt_GetTransNum(Instrument.toStdString());
    QJsonObject object{{"InstrumentTransactionNumber", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetValidFrom(QString Username, QString APIKey,
                                                QString Instrument)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    time64_t result = opentxs::OTAPI_Wrap::It()->Instrmnt_GetValidFrom(Instrument.toStdString());
    QJsonObject object{{"InstrumentValidFrom", QString(std::to_string(result).c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetValidTo(QString Username, QString APIKey,
                                              QString Instrument)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    time64_t result = opentxs::OTAPI_Wrap::It()->Instrmnt_GetValidTo(Instrument.toStdString());
    QJsonObject object{{"InstrumentValidTo", QString(std::to_string(result).c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetType(QString Username, QString APIKey,
                                           QString Instrument)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Instrmnt_GetType(Instrument.toStdString());
    QJsonObject object{{"InstrumentType", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetMemo(QString Username, QString APIKey,
                                           QString Instrument)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Instrmnt_GetMemo(Instrument.toStdString());
    QJsonObject object{{"InstrumentMemo", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetNotaryID(QString Username, QString APIKey,
                                               QString Instrument)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Instrmnt_GetNotaryID(Instrument.toStdString());
    QJsonObject object{{"InstrumentNotaryID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetInstrumentDefinitionID(QString Username, QString APIKey,
                                                             QString Instrument)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Instrmnt_GetInstrumentDefinitionID(Instrument.toStdString());
    QJsonObject object{{"InstrumentDefinitionID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetRemitterNymID(QString Username, QString APIKey,
                                                    QString Instrument)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Instrmnt_GetRemitterNymID(Instrument.toStdString());
    QJsonObject object{{"InstrumentRemitterNymID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetRemitterAccountID(QString Username, QString APIKey,
                                                        QString Instrument)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Instrmnt_GetRemitterAcctID(Instrument.toStdString());
    QJsonObject object{{"InstrumentRemitterAccountID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetSenderNymID(QString Username, QString APIKey,
                                                  QString Instrument)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Instrmnt_GetSenderNymID(Instrument.toStdString());
    QJsonObject object{{"InstrumentSenderNymID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetSenderAccountID(QString Username, QString APIKey,
                                                      QString Instrument)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Instrmnt_GetSenderAcctID(Instrument.toStdString());
    QJsonObject object{{"InstrumentSenderAccountID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetRecipientNymID(QString Username, QString APIKey,
                                                     QString Instrument)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Instrmnt_GetRecipientNymID(Instrument.toStdString());
    QJsonObject object{{"InstrumentRecipientNymID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::instrumentGetRecipientAccountID(QString Username, QString APIKey,
                                                         QString Instrument)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Instrmnt_GetRecipientAcctID(Instrument.toStdString());
    QJsonObject object{{"InstrumentRecipientAccountID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::setNymName(QString Username, QString APIKey,
                                    QString NymID, QString SignerNymID,
                                    QString NewName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SignerNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }


    bool result = opentxs::OTAPI_Wrap::It()->SetNym_Name(NymID.toStdString(),
                                                         SignerNymID.toStdString(),
                                                         NewName.toStdString());
    QJsonObject object{{"SetNymNameResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::setServerName(QString Username, QString APIKey,
                                       QString NotaryID, QString NewName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }


    bool result = opentxs::OTAPI_Wrap::It()->SetServer_Name(NotaryID.toStdString(),
                                                            NewName.toStdString());
    QJsonObject object{{"SetServerNameResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::setAssetTypeName(QString Username, QString APIKey,
                                          QString InstrumentDefinitionID, QString NewName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }


    bool result = opentxs::OTAPI_Wrap::It()->SetAssetType_Name(InstrumentDefinitionID.toStdString(),
                                                               NewName.toStdString());
    QJsonObject object{{"SetAssetTypeNameResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymTransactionNumberCount(QString Username, QString APIKey,
                                                      QString NotaryID, QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->GetNym_TransactionNumCount(NotaryID.toStdString(), NymID.toStdString());
    QJsonObject object{{"NymTransactionNumberCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getServerID(QString Username, QString APIKey,
                                     int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetServer_ID(Index);
    QJsonObject object{{"ServerID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getServerName(QString Username, QString APIKey,
                                       QString ServerID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(ServerID.toStdString())){
        QJsonObject object{{"Error", "Invalid ServerID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetServer_Name(ServerID.toStdString());
    QJsonObject object{{"ServerID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAssetTypeID(QString Username, QString APIKey,
                                        int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetAssetType_ID(Index);
    QJsonObject object{{"AssetTypeID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAssetTypeName(QString Username, QString APIKey,
                                          QString AssetTypeID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AssetTypeID.toStdString())){
        QJsonObject object{{"Error", "Invalid AssetTypeID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetAssetType_Name(AssetTypeID.toStdString());
    QJsonObject object{{"AssetTypeName", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAssetTypeTLA(QString Username, QString APIKey,
                                         QString AssetTypeID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AssetTypeID.toStdString())){
        QJsonObject object{{"Error", "Invalid AssetTypeID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetAssetType_TLA(AssetTypeID.toStdString());
    QJsonObject object{{"AssetTypeTLA", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAccountWalletID(QString Username, QString APIKey,
                                            int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetAccountWallet_ID(Index);
    QJsonObject object{{"AccountWalletID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAccountWalletName(QString Username, QString APIKey,
                                              QString AccountWalletID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountWalletID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountWalletID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetAccountWallet_Name(AccountWalletID.toStdString());
    QJsonObject object{{"AccountWalletName", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAccountWalletInboxHash(QString Username, QString APIKey,
                                                   QString AccountWalletID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountWalletID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountWalletID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetAccountWallet_InboxHash(AccountWalletID.toStdString());
    QJsonObject object{{"AccountWalletInboxHash", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAccountWalletOutboxHash(QString Username, QString APIKey,
                                                    QString AccountWalletID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountWalletID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountWalletID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetAccountWallet_OutboxHash(AccountWalletID.toStdString());
    QJsonObject object{{"AccountWalletOutboxHash", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getTime(QString Username, QString APIKey)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->GetTime();
    QJsonObject object{{"Time", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::encode(QString Username, QString APIKey,
                                QString Plaintext, bool LineBreaks)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Encode(Plaintext.toStdString(),
                                                           LineBreaks);
    QJsonObject object{{"EncodedText", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::decode(QString Username, QString APIKey,
                                QString Plaintext, bool LineBreaks)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Decode(Plaintext.toStdString(),
                                                           LineBreaks);
    QJsonObject object{{"DecodedText", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::encrypt(QString Username, QString APIKey,
                                 QString RecipientNymID, QString Plaintext)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(RecipientNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid RecipientNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Encrypt(RecipientNymID.toStdString(),
                                                            Plaintext.toStdString());
    QJsonObject object{{"EncryptedText", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::decrypt(QString Username, QString APIKey,
                                 QString RecipientNymID, QString CipherText)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(RecipientNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid RecipientNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Decrypt(RecipientNymID.toStdString(),
                                                            CipherText.toStdString());
    QJsonObject object{{"DecryptedText", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::createSymmetricKey(QString Username, QString APIKey)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->CreateSymmetricKey();
    QJsonObject object{{"SymmetricKey", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::symmetricEncrypt(QString Username, QString APIKey,
                                          QString SymmetricKey, QString Plaintext)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->SymmetricEncrypt(SymmetricKey.toStdString(),
                                                                     Plaintext.toStdString());
    QJsonObject object{{"EncryptedText", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::symmetricDecrypt(QString Username, QString APIKey,
                                          QString SymmetricKey, QString CipherTextEnvelope)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->SymmetricDecrypt(SymmetricKey.toStdString(),
                                                                     CipherTextEnvelope.toStdString());
    QJsonObject object{{"DecryptedText", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::signContract(QString Username, QString APIKey,
                                      QString SignerNymID, QString Contract)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SignerNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->SignContract(SignerNymID.toStdString(),
                                                                 Contract.toStdString());
    QJsonObject object{{"SignedContract", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::flatSign(QString Username, QString APIKey,
                                  QString SignerNymID, QString Input,
                                  QString ContractType)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SignerNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->FlatSign(SignerNymID.toStdString(),
                                                             Input.toStdString(),
                                                             ContractType.toStdString());
    QJsonObject object{{"FlatSignedText", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::addSignature(QString Username, QString APIKey,
                                      QString SignerNymID, QString Contract)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SignerNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->AddSignature(SignerNymID.toStdString(),
                                                                 Contract.toStdString());
    QJsonObject object{{"SignedContract", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::verifySignature(QString Username, QString APIKey,
                                         QString SignerNymID, QString Contract)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SignerNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->VerifySignature(SignerNymID.toStdString(),
                                                             Contract.toStdString());
    QJsonObject object{{"VerifySignatureResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::verifyAndRetrieveXMLContents(QString Username, QString APIKey,
                                                      QString Contract, QString SignerID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SignerID.toStdString())){
        QJsonObject object{{"Error", "Invalid SignerID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->VerifyAndRetrieveXMLContents(Contract.toStdString(),
                                                                                 SignerID.toStdString());
    QJsonObject object{{"ContractXMLContents", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::verifyAccountReceipt(QString Username, QString APIKey,
                                              QString NotaryID, QString NymID,
                                              QString AccountID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->VerifyAccountReceipt(NotaryID.toStdString(),
                                                                  NymID.toStdString(),
                                                                  AccountID.toStdString());
    QJsonObject object{{"VerifyAccountReceiptResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::setAccountWalletName(QString Username, QString APIKey,
                                              QString AccountID, QString SignerNymID,
                                              QString AccountName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SignerNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->SetAccountWallet_Name(AccountID.toStdString(),
                                                                   SignerNymID.toStdString(),
                                                                   AccountName.toStdString());
    QJsonObject object{{"SetAccountWalletNameResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAccountWalletBalance(QString Username, QString APIKey,
                                                 QString AccountWalletID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountWalletID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountWalletID"}};
        return QJsonValue(object);
    }

    qint64 result = opentxs::OTAPI_Wrap::It()->GetAccountWallet_Balance(AccountWalletID.toStdString());
    QJsonObject object{{"AccountWalletBalance", QString(std::to_string(result).c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAccountWalletType(QString Username, QString APIKey,
                                              QString AccountWalletID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountWalletID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountWalletID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetAccountWallet_Type(AccountWalletID.toStdString());
    QJsonObject object{{"AccountWalletType", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAccountWalletInstrumentDefinitionID(QString Username, QString APIKey,
                                                                QString AccountWalletID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountWalletID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountWalletID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetAccountWallet_InstrumentDefinitionID(AccountWalletID.toStdString());
    QJsonObject object{{"AccountWalletInstrumentDefinitionID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAccountWalletNotaryID(QString Username, QString APIKey,
                                                  QString AccountWalletID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountWalletID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountWalletID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetAccountWallet_NotaryID(AccountWalletID.toStdString());
    QJsonObject object{{"AccountWalletNotaryID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAccountWalletNymID(QString Username, QString APIKey,
                                               QString AccountWalletID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountWalletID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountWalletID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetAccountWallet_NymID(AccountWalletID.toStdString());
    QJsonObject object{{"AccountWalletNymID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::writeCheque(QString Username, QString APIKey,
                                     QString NotaryID, qint64 ChequeAmount,
                                     time64_t ValidFrom, time64_t ValidTo,
                                     QString SenderAccountID, QString SenderNymID,
                                     QString ChequeMemo, QString RecipientNymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SenderAccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid SenderAccountID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SenderNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid SenderNymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(RecipientNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid RecipientNymID"}};
        return QJsonValue(object);
    }

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

QJsonValue MCRPCService::discardCheque(QString Username, QString APIKey,
                                       QString NotaryID, QString NymID,
                                       QString AccountID, QString Cheque)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->DiscardCheque(NotaryID.toStdString(),
                                                           NymID.toStdString(),
                                                           AccountID.toStdString(),
                                                           Cheque.toStdString());
    QJsonObject object{{"DiscardChequeResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::proposePaymentPlan(QString Username, QString APIKey,
                                            QString NotaryID, time64_t ValidFrom,
                                            time64_t ValidTo, QString SenderAccountID,
                                            QString SenderNymID, QString PlanConsideration,
                                            QString RecipientAccountID, QString RecipientNymID,
                                            qint64 InitialPaymentAmount, time64_t InitialPaymentDelay,
                                            qint64 PaymentPlanAmount, time64_t PaymentPlanDelay,
                                            time64_t PaymentPlanPeriod, time64_t PaymentPlanLength,
                                            int MaxPayments)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SenderNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid SenderNymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(RecipientAccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid RecipientAccountID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(RecipientNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid RecipientNymID"}};
        return QJsonValue(object);
    }

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

QJsonValue MCRPCService::easyProposePlan(QString Username, QString APIKey,
                                         QString NotaryID, QString DateRange,
                                         QString SenderAccountID, QString SenderNymID,
                                         QString PlanConsideration, QString RecipientAccountID,
                                         QString RecipientNymID, QString InitialPayment,
                                         QString PaymentPlan, QString PlanExpiry)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SenderAccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid SenderAccountID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SenderNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid SenderNymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(RecipientAccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid RecipientAccountID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(RecipientNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid RecipientNymID"}};
        return QJsonValue(object);
    }

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

QJsonValue MCRPCService::confirmPaymentPlan(QString Username, QString APIKey,
                                            QString NotaryID, QString SenderNymID,
                                            QString SenderAccountID, QString RecipientNymID,
                                            QString PaymentPlan)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SenderNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid SenderNymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SenderAccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid SenderAccountID"}};
        return QJsonValue(object);
    }

    if(!opentxs::OTAPI_Wrap::It()->IsValidID(RecipientNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid RecipientNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->ConfirmPaymentPlan(NotaryID.toStdString(),
                                                                       SenderNymID.toStdString(),
                                                                       SenderAccountID.toStdString(),
                                                                       RecipientNymID.toStdString(),
                                                                       PaymentPlan.toStdString());
    QJsonObject object{{"ConfirmPaymentPlanResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::createSmartContract(QString Username, QString APIKey,
                                             QString SignerNymID, time64_t ValidFrom,
                                             time64_t ValidTo, bool SpecifyAssets,
                                             bool SpecifyParties)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SignerNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Create_SmartContract(SignerNymID.toStdString(),
                                                                         ValidFrom,
                                                                         ValidTo,
                                                                         SpecifyAssets,
                                                                         SpecifyParties);
    QJsonObject object{{"CreateSmartContractResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractSetDates(QString Username, QString APIKey,
                                               QString Contract, QString SignerNymID,
                                               time64_t ValidFrom, time64_t ValidTo)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SignerNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->SmartContract_SetDates(Contract.toStdString(),
                                                                           SignerNymID.toStdString(),
                                                                           ValidFrom,
                                                                           ValidTo);
    QJsonObject object{{"SmartContractSetDatesResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartArePartiesSpecified(QString Username, QString APIKey,
                                                  QString Contract)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->Smart_ArePartiesSpecified(Contract.toStdString());
    QJsonObject object{{"SmartArePartiesSpecifiedResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartAreAssetTypesSpecified(QString Username, QString APIKey,
                                                     QString Contract)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->Smart_AreAssetTypesSpecified(Contract.toStdString());
    QJsonObject object{{"SmartAreAssetTypesSpecifiedResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractAddBylaw(QString Username, QString APIKey,
                                               QString Contract, QString SignerNymID,
                                               QString BylawName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SignerNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->SmartContract_AddBylaw(Contract.toStdString(),
                                                                           SignerNymID.toStdString(),
                                                                           BylawName.toStdString());
    QJsonObject object{{"SmartContractAddBylawResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractAddClause(QString Username, QString APIKey,
                                                QString Contract, QString SignerNymID,
                                                QString BylawName, QString ClauseName,
                                                QString SourceCode)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SignerNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->SmartContract_AddClause(Contract.toStdString(),
                                                                            SignerNymID.toStdString(),
                                                                            BylawName.toStdString(),
                                                                            ClauseName.toStdString(),
                                                                            SourceCode.toStdString());
    QJsonObject object{{"SmartContractAddClauseResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractAddVariable(QString Username, QString APIKey,
                                                  QString Contract, QString SignerNymID,
                                                  QString BylawName, QString VarName,
                                                  QString VarAccess, QString VarType,
                                                  QString VarValue)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SignerNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

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

QJsonValue MCRPCService::smartContractAddCallback(QString Username, QString APIKey,
                                                  QString Contract, QString SignerNymID,
                                                  QString BylawName, QString CallbackName,
                                                  QString ClauseName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SignerNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->SmartContract_AddCallback(Contract.toStdString(),
                                                                              SignerNymID.toStdString(),
                                                                              BylawName.toStdString(),
                                                                              CallbackName.toStdString(),
                                                                              ClauseName.toStdString());
    QJsonObject object{{"SmartContractAddCallbackResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractAddHook(QString Username, QString APIKey,
                                              QString Contract, QString SignerNymID,
                                              QString BylawName, QString HookName,
                                              QString ClauseName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SignerNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->SmartContract_AddHook(Contract.toStdString(),
                                                                          SignerNymID.toStdString(),
                                                                          BylawName.toStdString(),
                                                                          HookName.toStdString(),
                                                                          ClauseName.toStdString());
    QJsonObject object{{"SmartContractAddHookResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractAddParty(QString Username, QString APIKey,
                                               QString Contract, QString SignerNymID,
                                               QString PartyNymID, QString PartyName,
                                               QString AgentName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SignerNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(PartyNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid PartyNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->SmartContract_AddParty(Contract.toStdString(),
                                                                           SignerNymID.toStdString(),
                                                                           PartyNymID.toStdString(),
                                                                           PartyName.toStdString(),
                                                                           AgentName.toStdString());
    QJsonObject object{{"SmartContractAddPartyResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractAddAccount(QString Username, QString APIKey,
                                                 QString Contract, QString SignerNymID,
                                                 QString PartyName, QString AccountName,
                                                 QString InstrumentDefinitionID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SignerNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }
    std::string result = opentxs::OTAPI_Wrap::It()->SmartContract_AddAccount(Contract.toStdString(),
                                                                             SignerNymID.toStdString(),
                                                                             PartyName.toStdString(),
                                                                             AccountName.toStdString(),
                                                                             InstrumentDefinitionID.toStdString());
    QJsonObject object{{"SmartContractAddAccountResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractRemoveBylaw(QString Username, QString APIKey,
                                                  QString Contract, QString SignerNymID,
                                                  QString BylawName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SignerNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->SmartContract_RemoveBylaw(Contract.toStdString(),
                                                                              SignerNymID.toStdString(),
                                                                              BylawName.toStdString());
    QJsonObject object{{"SmartContractRemoveBylawResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractUpdateClause(QString Username, QString APIKey,
                                                   QString Contract, QString SignerNymID,
                                                   QString BylawName, QString ClauseName,
                                                   QString SourceCode)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SignerNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->SmartContract_UpdateClause(Contract.toStdString(),
                                                                               SignerNymID.toStdString(),
                                                                               BylawName.toStdString(),
                                                                               ClauseName.toStdString(),
                                                                               SourceCode.toStdString());
    QJsonObject object{{"SmartContractUpdateClauseResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractRemoveClause(QString Username, QString APIKey,
                                                   QString Contract, QString SignerNymID,
                                                   QString BylawName, QString ClauseName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SignerNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->SmartContract_RemoveClause(Contract.toStdString(),
                                                                               SignerNymID.toStdString(),
                                                                               BylawName.toStdString(),
                                                                               ClauseName.toStdString());
    QJsonObject object{{"SmartContractRemoveClauseResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractRemoveVariable(QString Username, QString APIKey,
                                                     QString Contract, QString SignerNymID,
                                                     QString BylawName, QString VarName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SignerNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->SmartContract_RemoveVariable(Contract.toStdString(),
                                                                                 SignerNymID.toStdString(),
                                                                                 BylawName.toStdString(),
                                                                                 VarName.toStdString());
    QJsonObject object{{"SmartContractRemoveVariableResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractRemoveCallback(QString Username, QString APIKey,
                                                     QString Contract, QString SignerNymID,
                                                     QString BylawName, QString CallbackName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SignerNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->SmartContract_RemoveCallback(Contract.toStdString(),
                                                                                 SignerNymID.toStdString(),
                                                                                 BylawName.toStdString(),
                                                                                 CallbackName.toStdString());
    QJsonObject object{{"SmartContractRemoveCallbackResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractRemoveHook(QString Username, QString APIKey,
                                                 QString Contract, QString SignerNymID,
                                                 QString BylawName, QString HookName,
                                                 QString ClauseName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SignerNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->SmartContract_RemoveHook(Contract.toStdString(),
                                                                             SignerNymID.toStdString(),
                                                                             BylawName.toStdString(),
                                                                             HookName.toStdString(),
                                                                             ClauseName.toStdString());
    QJsonObject object{{"SmartContractRemoveHookResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractRemoveParty(QString Username, QString APIKey,
                                                  QString Contract, QString SignerNymID,
                                                  QString PartyName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SignerNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->SmartContract_RemoveParty(Contract.toStdString(),
                                                                              SignerNymID.toStdString(),
                                                                              PartyName.toStdString());
    QJsonObject object{{"SmartContractRemovePartyResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractCountNumbersNeeded(QString Username, QString APIKey,
                                                         QString Contract, QString AgentName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->SmartContract_CountNumsNeeded(Contract.toStdString(),
                                                                          AgentName.toStdString());
    QJsonObject object{{"SmartContractNumbersNeeded", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractConfirmAccount(QString Username, QString APIKey,
                                                     QString Contract, QString SignerNymID,
                                                     QString PartyName, QString AccountName,
                                                     QString AgentName, QString AccountID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SignerNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->SmartContract_ConfirmAccount(Contract.toStdString(),
                                                                                 SignerNymID.toStdString(),
                                                                                 PartyName.toStdString(),
                                                                                 AccountName.toStdString(),
                                                                                 AgentName.toStdString(),
                                                                                 AccountID.toStdString());
    QJsonObject object{{"SmartContractConfirmAccountResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartContractConfirmParty(QString Username, QString APIKey,
                                                   QString Contract, QString PartyName,
                                                   QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->SmartContract_ConfirmParty(Contract.toStdString(),
                                                                               PartyName.toStdString(),
                                                                               NymID.toStdString());
    QJsonObject object{{"SmartContractConfirmPartyResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartAreAllPartiesConfirmed(QString Username, QString APIKey,
                                                     QString Contract)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->Smart_AreAllPartiesConfirmed(Contract.toStdString());
    QJsonObject object{{"SmartAreAllPartiesConfirmedResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartIsPartyConfirmed(QString Username, QString APIKey,
                                               QString Contract, QString PartyName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->Smart_IsPartyConfirmed(Contract.toStdString(),
                                                                    PartyName.toStdString());
    QJsonObject object{{"SmartIsPartyConfirmedResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartGetPartyCount(QString Username, QString APIKey,
                                            QString Contract)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->Smart_GetPartyCount(Contract.toStdString());
    QJsonObject object{{"SmartPartyCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartGetBylawCount(QString Username, QString APIKey,
                                            QString Contract)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->Smart_GetBylawCount(Contract.toStdString());
    QJsonObject object{{"SmartBylawCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartGetPartyByIndex(QString Username, QString APIKey,
                                              QString Contract, int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Smart_GetPartyByIndex(Contract.toStdString(),
                                                                          Index);
    QJsonObject object{{"SmartGetPartyByIndexResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::smartGetBylawByIndex(QString Username, QString APIKey,
                                              QString Contract, int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Smart_GetBylawByIndex(Contract.toStdString(),
                                                                          Index);
    QJsonObject object{{"SmartGetBylawByIndexResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::bylawGetLanguage(QString Username, QString APIKey,
                                          QString Contract, QString BylawName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Bylaw_GetLanguage(Contract.toStdString(),
                                                                      BylawName.toStdString());
    QJsonObject object{{"BylawLanguage", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::bylawGetClauseCount(QString Username, QString APIKey,
                                             QString Contract, QString BylawName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->Bylaw_GetClauseCount(Contract.toStdString(),
                                                                 BylawName.toStdString());
    QJsonObject object{{"BylawClauseCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::bylawGetVariableCount(QString Username, QString APIKey,
                                               QString Contract, QString BylawName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->Bylaw_GetVariableCount(Contract.toStdString(),
                                                                   BylawName.toStdString());
    QJsonObject object{{"BylawVariableCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::bylawGetHookCount(QString Username, QString APIKey,
                                           QString Contract, QString BylawName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->Bylaw_GetHookCount(Contract.toStdString(),
                                                               BylawName.toStdString());
    QJsonObject object{{"BylawHookCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::bylawGetCallbackCount(QString Username, QString APIKey,
                                               QString Contract, QString BylawName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->Bylaw_GetCallbackCount(Contract.toStdString(),
                                                                   BylawName.toStdString());
    QJsonObject object{{"BylawCallbackCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::clauseGetNameByIndex(QString Username, QString APIKey,
                                              QString Contract, QString BylawName,
                                              int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Clause_GetNameByIndex(Contract.toStdString(),
                                                                          BylawName.toStdString(),
                                                                          Index);
    QJsonObject object{{"ClauseGetNameByIndexResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::clauseGetContents(QString Username, QString APIKey,
                                           QString Contract, QString BylawName,
                                           QString ClauseName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Clause_GetContents(Contract.toStdString(),
                                                                       BylawName.toStdString(),
                                                                       ClauseName.toStdString());
    QJsonObject object{{"ClauseContents", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::variableGetNameByIndex(QString Username, QString APIKey,
                                                QString Contract, QString BylawName,
                                                int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Variable_GetNameByIndex(Contract.toStdString(),
                                                                            BylawName.toStdString(),
                                                                            Index);
    QJsonObject object{{"VariableGetNameByIndexResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::variableGetType(QString Username, QString APIKey,
                                         QString Contract, QString BylawName,
                                         QString VariableName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Variable_GetType(Contract.toStdString(),
                                                                     BylawName.toStdString(),
                                                                     VariableName.toStdString());
    QJsonObject object{{"VariableType", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::variableGetAccess(QString Username, QString APIKey,
                                           QString Contract, QString BylawName,
                                           QString VariableName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Variable_GetAccess(Contract.toStdString(),
                                                                       BylawName.toStdString(),
                                                                       VariableName.toStdString());
    QJsonObject object{{"VariableAccess", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::variableGetContents(QString Username, QString APIKey,
                                             QString Contract, QString BylawName,
                                             QString VariableName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Variable_GetContents(Contract.toStdString(),
                                                                         BylawName.toStdString(),
                                                                         VariableName.toStdString());
    QJsonObject object{{"VariableContents", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::hookGetNameByIndex(QString Username, QString APIKey,
                                            QString Contract, QString BylawName,
                                            int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Hook_GetNameByIndex(Contract.toStdString(),
                                                                        BylawName.toStdString(),
                                                                        Index);
    QJsonObject object{{"HookGetNameByIndexResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::hookGetClauseCount(QString Username, QString APIKey,
                                            QString Contract, QString BylawName,
                                            QString HookName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->Hook_GetClauseCount(Contract.toStdString(),
                                                                BylawName.toStdString(),
                                                                HookName.toStdString());
    QJsonObject object{{"HookClauseCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::hookGetClauseAtIndex(QString Username, QString APIKey,
                                              QString Contract, QString BylawName,
                                              QString HookName, int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Hook_GetClauseAtIndex(Contract.toStdString(),
                                                                          BylawName.toStdString(),
                                                                          HookName.toStdString(),
                                                                          Index);
    QJsonObject object{{"HookGetClauseAtIndexResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::callbackGetNameByIndex(QString Username, QString APIKey,
                                                QString Contract, QString BylawName,
                                                int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Callback_GetNameByIndex(Contract.toStdString(),
                                                                            BylawName.toStdString(),
                                                                            Index);
    QJsonObject object{{"CallbackGetNameByIndexResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::callbackGetClause(QString Username, QString APIKey,
                                           QString Contract, QString BylawName,
                                           QString ClauseName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Callback_GetClause(Contract.toStdString(),
                                                                       BylawName.toStdString(),
                                                                       ClauseName.toStdString());
    QJsonObject object{{"CallbackGetClauseResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::partyGetAccountCount(QString Username, QString APIKey,
                                              QString Contract, QString PartyName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->Party_GetAcctCount(Contract.toStdString(),
                                                               PartyName.toStdString());
    QJsonObject object{{"PartyAccountCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::partyGetAgentCount(QString Username, QString APIKey,
                                            QString Contract, QString PartyName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->Party_GetAgentCount(Contract.toStdString(),
                                                                PartyName.toStdString());
    QJsonObject object{{"PartyAgentCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::partyGetID(QString Username, QString APIKey,
                                    QString Contract, QString PartyName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Party_GetID(Contract.toStdString(),
                                                                PartyName.toStdString());
    QJsonObject object{{"PartyID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::partyGetAccountNameByIndex(QString Username, QString APIKey,
                                                    QString Contract, QString PartyName,
                                                    int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Party_GetAcctNameByIndex(Contract.toStdString(),
                                                                             PartyName.toStdString(),
                                                                             Index);
    QJsonObject object{{"PartyGetAccountNameByIndexResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::partyGetAccountID(QString Username, QString APIKey,
                                           QString Contract, QString PartyName,
                                           QString AccountName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Party_GetAcctID(Contract.toStdString(),
                                                                    PartyName.toStdString(),
                                                                    AccountName.toStdString());
    QJsonObject object{{"PartyGetAccountIDResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::partyGetAccountInstrumentDefinitionID(QString Username, QString APIKey,
                                                               QString Contract, QString PartyName,
                                                               QString AccountName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Party_GetAcctInstrumentDefinitionID(Contract.toStdString(),
                                                                                        PartyName.toStdString(),
                                                                                        AccountName.toStdString());
    QJsonObject object{{"PartyGetAccountInstrumentDefinitionIDResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::partyGetAccountAgentName(QString Username, QString APIKey,
                                                  QString Contract, QString PartyName,
                                                  QString AccountName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Party_GetAcctAgentName(Contract.toStdString(),
                                                                           PartyName.toStdString(),
                                                                           AccountName.toStdString());
    QJsonObject object{{"PartyAccountAgentName", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::partyGetAgentNameByIndex(QString Username, QString APIKey,
                                                  QString Contract, QString PartyName,
                                                  int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Party_GetAgentNameByIndex(Contract.toStdString(),
                                                                              PartyName.toStdString(),
                                                                              Index);
    QJsonObject object{{"PartyGetAgentNameByIndexResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::partyGetAgentID(QString Username, QString APIKey,
                                         QString Contract, QString PartyName,
                                         QString AgentName)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Party_GetAgentID(Contract.toStdString(),
                                                                     PartyName.toStdString(),
                                                                     AgentName.toStdString());
    QJsonObject object{{"PartyGetAgentIDResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::activateSmartContract(QString Username, QString APIKey,
                                               QString NotaryID, QString NymID,
                                               QString SmartContract)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->activateSmartContract(NotaryID.toStdString(),
                                                                  NymID.toStdString(),
                                                                  SmartContract.toStdString());
    QJsonObject object{{"ActivateSmartContractResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::triggerClause(QString Username, QString APIKey,
                                       QString NotaryID, QString NymID,
                                       qint64 TransactionNumber, QString ClauseName,
                                       QString Parameter)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->triggerClause(NotaryID.toStdString(),
                                                          NymID.toStdString(),
                                                          TransactionNumber,
                                                          ClauseName.toStdString(),
                                                          Parameter.toStdString());
    QJsonObject object{{"TriggerClauseResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageHarvestTransactionNumbers(QString Username, QString APIKey,
                                                          QString Message, QString NymID,
                                                          bool HarvestingForRetry, bool ReplyWasSuccess,
                                                          bool ReplyWasFailure, bool TransactionWasSuccess,
                                                          bool TransactionWasFailure)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

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

QJsonValue MCRPCService::loadPubkeyEncryption(QString Username, QString APIKey,
                                              QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->LoadPubkey_Encryption(NymID.toStdString());
    QJsonObject object{{"LoadPubkeyEncryptionResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadPubkeySigning(QString Username, QString APIKey,
                                           QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->LoadPubkey_Signing(NymID.toStdString());
    QJsonObject object{{"LoadPubkeySigningResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadUserPubkeyEncryption(QString Username, QString APIKey,
                                                  QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->LoadUserPubkey_Encryption(NymID.toStdString());
    QJsonObject object{{"LoadUserPubkeyEncryptionResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadUserPubkeySigning(QString Username, QString APIKey,
                                               QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->LoadUserPubkey_Signing(NymID.toStdString());
    QJsonObject object{{"LoadUserPubkeySigningResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::verifyUserPrivateKey(QString Username, QString APIKey,
                                              QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->VerifyUserPrivateKey(NymID.toStdString());
    QJsonObject object{{"VerifyUserPrivateKeyResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::mintIsStillGood(QString Username, QString APIKey,
                                         QString NotaryID, QString InstrumentDefinitionID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->Mint_IsStillGood(NotaryID.toStdString(),
                                                              InstrumentDefinitionID.toStdString());
    QJsonObject object{{"MintIsStillGoodResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadMint(QString Username, QString APIKey,
                                  QString NotaryID, QString InstrumentDefinitionID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->LoadMint(NotaryID.toStdString(),
                                                             InstrumentDefinitionID.toStdString());
    QJsonObject object{{"LoadMintResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadUnitDefinition(QString Username, QString APIKey,
                                           QString InstrumentDefinitionID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->LoadUnitDefinition(InstrumentDefinitionID.toStdString());
    QJsonObject object{{"LoadUnitDefinitionResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadServerContract(QString Username, QString APIKey,
                                            QString NotaryID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->LoadServerContract(NotaryID.toStdString());
    QJsonObject object{{"LoadServerContractResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadAssetAccount(QString Username, QString APIKey,
                                          QString NotaryID, QString NymID,
                                          QString AccountID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->LoadAssetAccount(NotaryID.toStdString(),
                                                                     NymID.toStdString(),
                                                                     AccountID.toStdString());
    QJsonObject object{{"LoadAssetAccountResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::nymboxGetReplyNotice(QString Username, QString APIKey,
                                              QString NotaryID, QString NymID,
                                              qint64 RequestNumber)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Nymbox_GetReplyNotice(NotaryID.toStdString(),
                                                                          NymID.toStdString(),
                                                                          RequestNumber);
    QJsonObject object{{"NymboxGetReplyNoticeResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::haveAlreadySeenReply(QString Username, QString APIKey,
                                              QString NotaryID, QString NymID,
                                              qint64 RequestNumber)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->HaveAlreadySeenReply(NotaryID.toStdString(),
                                                                  NymID.toStdString(),
                                                                  RequestNumber);
    QJsonObject object{{"HaveAlreadySeenReplyResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadNymbox(QString Username, QString APIKey,
                                    QString NotaryID, QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->LoadNymbox(NotaryID.toStdString(),
                                                               NymID.toStdString());
    QJsonObject object{{"LoadNymboxResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadNymboxNoVerify(QString Username, QString APIKey,
                                            QString NotaryID, QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->LoadNymboxNoVerify(NotaryID.toStdString(),
                                                                       NymID.toStdString());
    QJsonObject object{{"LoadNymboxNoVerifyResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadInbox(QString Username, QString APIKey,
                                   QString NotaryID, QString NymID,
                                   QString AccountID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->LoadInbox(NotaryID.toStdString(),
                                                              NymID.toStdString(),
                                                              AccountID.toStdString());
    QJsonObject object{{"LoadInboxResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadInboxNoVerify(QString Username, QString APIKey,
                                           QString NotaryID, QString NymID,
                                           QString AccountID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->LoadInboxNoVerify(NotaryID.toStdString(),
                                                                      NymID.toStdString(),
                                                                      AccountID.toStdString());
    QJsonObject object{{"LoadInboxResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadOutbox(QString Username, QString APIKey,
                                    QString NotaryID, QString NymID,
                                    QString AccountID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->LoadOutbox(NotaryID.toStdString(),
                                                               NymID.toStdString(),
                                                               AccountID.toStdString());
    QJsonObject object{{"LoadOutboxResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadOutboxNoVerify(QString Username, QString APIKey,
                                            QString NotaryID, QString NymID,
                                            QString AccountID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->LoadOutboxNoVerify(NotaryID.toStdString(),
                                                                       NymID.toStdString(),
                                                                       AccountID.toStdString());
    QJsonObject object{{"LoadOutboxNoVerifyResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadPaymentInbox(QString Username, QString APIKey,
                                          QString NotaryID, QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->LoadPaymentInbox(NotaryID.toStdString(),
                                                                     NymID.toStdString());
    QJsonObject object{{"LoadPaymentInboxResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadPaymentInboxNoVerify(QString Username, QString APIKey,
                                                  QString NotaryID, QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->LoadPaymentInboxNoVerify(NotaryID.toStdString(),
                                                                             NymID.toStdString());
    QJsonObject object{{"LoadPaymentInboxNoVerifyResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadRecordBox(QString Username, QString APIKey,
                                       QString NotaryID, QString NymID,
                                       QString AccountID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->LoadRecordBox(NotaryID.toStdString(),
                                                                  NymID.toStdString(),
                                                                  AccountID.toStdString());
    QJsonObject object{{"LoadRecordBoxResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadRecordBoxNoVerify(QString Username, QString APIKey,
                                               QString NotaryID, QString NymID,
                                               QString AccountID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->LoadRecordBoxNoVerify(NotaryID.toStdString(),
                                                                          NymID.toStdString(),
                                                                          AccountID.toStdString());
    QJsonObject object{{"LoadRecordBoxNoVerifyResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadExpiredBox(QString Username, QString APIKey,
                                        QString NotaryID, QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->LoadExpiredBox(NotaryID.toStdString(),
                                                                   NymID.toStdString());
    QJsonObject object{{"LoadExpiredBoxResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadExpiredBoxNoVerify(QString Username, QString APIKey,
                                                QString NotaryID, QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->LoadExpiredBoxNoVerify(NotaryID.toStdString(),
                                                                           NymID.toStdString());
    QJsonObject object{{"LoadExpiredBoxNoVerifyResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::recordPayment(QString Username, QString APIKey,
                                       QString NotaryID, QString NymID,
                                       bool IsInbox, int Index,
                                       bool SaveCopy)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->RecordPayment(NotaryID.toStdString(),
                                                           NymID.toStdString(),
                                                           IsInbox,
                                                           Index,
                                                           SaveCopy);
    QJsonObject object{{"RecordPaymentResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::clearRecord(QString Username, QString APIKey,
                                     QString NotaryID, QString NymID,
                                     QString AccountID, int Index,
                                     bool ClearAll)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->ClearRecord(NotaryID.toStdString(),
                                                         NymID.toStdString(),
                                                         AccountID.toStdString(),
                                                         Index,
                                                         ClearAll);
    QJsonObject object{{"ClearRecordResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::clearExpired(QString Username, QString APIKey,
                                      QString NotaryID, QString NymID,
                                      int Index, bool ClearAll)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->ClearExpired(NotaryID.toStdString(),
                                                          NymID.toStdString(),
                                                          Index,
                                                          ClearAll);
    QJsonObject object{{"ClearExpiredResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::ledgerGetCount(QString Username, QString APIKey,
                                        QString NotaryID, QString NymID,
                                        QString AccountID, QString Ledger)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->Ledger_GetCount(NotaryID.toStdString(),
                                                            NymID.toStdString(),
                                                            AccountID.toStdString(),
                                                            Ledger.toStdString());
    QJsonObject object{{"LedgerCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::ledgerCreateResponse(QString Username, QString APIKey,
                                              QString NotaryID, QString NymID,
                                              QString AccountID, QString OriginalLedger)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Ledger_CreateResponse(NotaryID.toStdString(),
                                                                          NymID.toStdString(),
                                                                          AccountID.toStdString(),
                                                                          OriginalLedger.toStdString());
    QJsonObject object{{"LedgerResponse", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::ledgerGetTransactionByIndex(QString Username, QString APIKey,
                                                     QString NotaryID, QString NymID,
                                                     QString AccountID, QString Ledger,
                                                     int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Ledger_GetTransactionByIndex(NotaryID.toStdString(),
                                                                                 NymID.toStdString(),
                                                                                 AccountID.toStdString(),
                                                                                 Ledger.toStdString(),
                                                                                 Index);
    QJsonObject object{{"LedgerGetTransactionByIndexResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::ledgerGetTransactionByID(QString Username, QString APIKey,
                                                  QString NotaryID, QString NymID,
                                                  QString AccountID, QString Ledger,
                                                  qint64 TransactionNumber)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Ledger_GetTransactionByIndex(NotaryID.toStdString(),
                                                                                 NymID.toStdString(),
                                                                                 AccountID.toStdString(),
                                                                                 Ledger.toStdString(),
                                                                                 TransactionNumber);
    QJsonObject object{{"LedgerGetTransactionByIDResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::ledgerGetInstrument(QString Username, QString APIKey,
                                             QString NotaryID, QString NymID,
                                             QString AccountID, QString Ledger,
                                             int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Ledger_GetInstrument(NotaryID.toStdString(),
                                                                         NymID.toStdString(),
                                                                         AccountID.toStdString(),
                                                                         Ledger.toStdString(),
                                                                         Index);
    QJsonObject object{{"LedgerGetInstrumentResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::ledgerGetTransactionIDByIndex(QString Username, QString APIKey,
                                                       QString NotaryID, QString NymID,
                                                       QString AccountID, QString Ledger,
                                                       int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    qint64 result = opentxs::OTAPI_Wrap::It()->Ledger_GetTransactionIDByIndex(NotaryID.toStdString(),
                                                                              NymID.toStdString(),
                                                                              AccountID.toStdString(),
                                                                              Ledger.toStdString(),
                                                                              Index);
    QJsonObject object{{"LedgerGetTransactionIDByIndexResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::ledgerAddTransaction(QString Username, QString APIKey,
                                              QString NotaryID, QString NymID,
                                              QString AccountID, QString Ledger,
                                              QString Transaction)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Ledger_AddTransaction(NotaryID.toStdString(),
                                                                          NymID.toStdString(),
                                                                          AccountID.toStdString(),
                                                                          Ledger.toStdString(),
                                                                          Transaction.toStdString());
    QJsonObject object{{"LedgerAddTransactionResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionCreateResponse(QString Username, QString APIKey,
                                                   QString NotaryID, QString NymID,
                                                   QString AccountID, QString Ledger,
                                                   QString Transaction, bool DoIAccept)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Transaction_CreateResponse(NotaryID.toStdString(),
                                                                               NymID.toStdString(),
                                                                               AccountID.toStdString(),
                                                                               Ledger.toStdString(),
                                                                               Transaction.toStdString(),
                                                                               DoIAccept);
    QJsonObject object{{"TransactionCreateResponseResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::ledgerFinalizeResponse(QString Username, QString APIKey,
                                                QString NotaryID, QString NymID,
                                                QString AccountID, QString Ledger)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Ledger_FinalizeResponse(NotaryID.toStdString(),
                                                                            NymID.toStdString(),
                                                                            AccountID.toStdString(),
                                                                            Ledger.toStdString());
    QJsonObject object{{"LedgerFinalizeResponseResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionGetVoucher(QString Username, QString APIKey,
                                               QString NotaryID, QString NymID,
                                               QString AccountID, QString Transaction)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Transaction_GetVoucher(NotaryID.toStdString(),
                                                                           NymID.toStdString(),
                                                                           AccountID.toStdString(),
                                                                           Transaction.toStdString());
    QJsonObject object{{"TransactionVoucher", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionGetSenderNymID(QString Username, QString APIKey,
                                                   QString NotaryID, QString NymID,
                                                   QString AccountID, QString Transaction)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Transaction_GetSenderNymID(NotaryID.toStdString(),
                                                                               NymID.toStdString(),
                                                                               AccountID.toStdString(),
                                                                               Transaction.toStdString());
    QJsonObject object{{"TransactionSenderNymID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionGetRecipientNymID(QString Username, QString APIKey,
                                                      QString NotaryID, QString NymID,
                                                      QString AccountID, QString Transaction)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Transaction_GetRecipientNymID(NotaryID.toStdString(),
                                                                                  NymID.toStdString(),
                                                                                  AccountID.toStdString(),
                                                                                  Transaction.toStdString());
    QJsonObject object{{"TransactionRecipientNymID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionGetSenderAccountID(QString Username, QString APIKey,
                                                       QString NotaryID, QString NymID,
                                                       QString AccountID, QString Transaction)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Transaction_GetSenderAcctID(NotaryID.toStdString(),
                                                                                NymID.toStdString(),
                                                                                AccountID.toStdString(),
                                                                                Transaction.toStdString());
    QJsonObject object{{"TransactionSenderAccountID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionGetRecipientAccountID(QString Username, QString APIKey,
                                                          QString NotaryID, QString NymID,
                                                          QString AccountID, QString Transaction)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Transaction_GetRecipientAcctID(NotaryID.toStdString(),
                                                                                   NymID.toStdString(),
                                                                                   AccountID.toStdString(),
                                                                                   Transaction.toStdString());
    QJsonObject object{{"TransactionRecipientAccountID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::pendingGetNote(QString Username, QString APIKey,
                                        QString NotaryID, QString NymID,
                                        QString AccountID, QString Transaction)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Pending_GetNote(NotaryID.toStdString(),
                                                                    NymID.toStdString(),
                                                                    AccountID.toStdString(),
                                                                    Transaction.toStdString());
    QJsonObject object{{"PendingNote", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionGetAmount(QString Username, QString APIKey,
                                              QString NotaryID, QString NymID,
                                              QString AccountID, QString Transaction)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    qint64 result = opentxs::OTAPI_Wrap::It()->Transaction_GetAmount(NotaryID.toStdString(),
                                                                     NymID.toStdString(),
                                                                     AccountID.toStdString(),
                                                                     Transaction.toStdString());
    QJsonObject object{{"TransactionAmount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionGetDisplayReferenceToNumber(QString Username, QString APIKey,
                                                                QString NotaryID, QString NymID,
                                                                QString AccountID, QString Transaction)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    qint64 result = opentxs::OTAPI_Wrap::It()->Transaction_GetDisplayReferenceToNum(NotaryID.toStdString(),
                                                                                    NymID.toStdString(),
                                                                                    AccountID.toStdString(),
                                                                                    Transaction.toStdString());
    QJsonObject object{{"TransactionDisplayReferenceToNumber", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionGetType(QString Username, QString APIKey,
                                            QString NotaryID, QString NymID,
                                            QString AccountID, QString Transaction)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Transaction_GetType(NotaryID.toStdString(),
                                                                        NymID.toStdString(),
                                                                        AccountID.toStdString(),
                                                                        Transaction.toStdString());
    QJsonObject object{{"TransactionType", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::replyNoticeGetRequestNumber(QString Username, QString APIKey,
                                                     QString NotaryID, QString NymID,
                                                     QString Transaction)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    qint64 result = opentxs::OTAPI_Wrap::It()->ReplyNotice_GetRequestNum(NotaryID.toStdString(),
                                                                         NymID.toStdString(),
                                                                         Transaction.toStdString());
    QJsonObject object{{"ReplyNoticeRequestNumber", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionGetDateSigned(QString Username, QString APIKey,
                                                  QString NotaryID, QString NymID,
                                                  QString AccountID, QString Transaction)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    time64_t result = opentxs::OTAPI_Wrap::It()->Transaction_GetDateSigned(NotaryID.toStdString(),
                                                                           NymID.toStdString(),
                                                                           AccountID.toStdString(),
                                                                           Transaction.toStdString());
    QJsonObject object{{"TransactionDateSigned", QString(std::to_string(result).c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionGetSuccess(QString Username, QString APIKey,
                                               QString NotaryID, QString NymID,
                                               QString AccountID, QString Transaction)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->Transaction_GetSuccess(NotaryID.toStdString(),
                                                                    NymID.toStdString(),
                                                                    AccountID.toStdString(),
                                                                    Transaction.toStdString());
    QJsonObject object{{"TransactionSuccess", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionIsCanceled(QString Username, QString APIKey,
                                               QString NotaryID, QString NymID,
                                               QString AccountID, QString Transaction)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->Transaction_IsCanceled(NotaryID.toStdString(),
                                                                    NymID.toStdString(),
                                                                    AccountID.toStdString(),
                                                                    Transaction.toStdString());
    QJsonObject object{{"TransactionIsCanceled", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::transactionGetBalanceAgreementSuccess(QString Username, QString APIKey,
                                                               QString NotaryID, QString NymID,
                                                               QString AccountID, QString Transaction)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->Transaction_GetBalanceAgreementSuccess(NotaryID.toStdString(),
                                                                                    NymID.toStdString(),
                                                                                    AccountID.toStdString(),
                                                                                    Transaction.toStdString());
    QJsonObject object{{"TransactionBalanceAgreementSuccess", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetBalanceAgreementSuccess(QString Username, QString APIKey,
                                                           QString NotaryID, QString NymID,
                                                           QString AccountID, QString Message)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->Message_GetBalanceAgreementSuccess(NotaryID.toStdString(),
                                                                                NymID.toStdString(),
                                                                                AccountID.toStdString(),
                                                                                Message.toStdString());
    QJsonObject object{{"MessageBalanceAgreementSuccess", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::savePurse(QString Username, QString APIKey,
                                   QString NotaryID, QString InstrumentDefinitionID,
                                   QString NymID, QString Purse)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->SavePurse(NotaryID.toStdString(),
                                                       InstrumentDefinitionID.toStdString(),
                                                       NymID.toStdString(),
                                                       Purse.toStdString());
    QJsonObject object{{"SavePurseResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::loadPurse(QString Username, QString APIKey,
                                   QString NotaryID, QString InstrumentDefinitionID,
                                   QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->LoadPurse(NotaryID.toStdString(),
                                                              InstrumentDefinitionID.toStdString(),
                                                              NymID.toStdString());
    QJsonObject object{{"Purse", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::purseGetTotalValue(QString Username, QString APIKey,
                                            QString NotaryID, QString InstrumentDefinitionID,
                                            QString Purse)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    qint64 result = opentxs::OTAPI_Wrap::It()->Purse_GetTotalValue(NotaryID.toStdString(),
                                                                   InstrumentDefinitionID.toStdString(),
                                                                   Purse.toStdString());
    QJsonObject object{{"PurseTotalValue", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::purseCount(QString Username, QString APIKey,
                                    QString NotaryID, QString InstrumentDefinitionID,
                                    QString Purse)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->Purse_Count(NotaryID.toStdString(),
                                                        InstrumentDefinitionID.toStdString(),
                                                        Purse.toStdString());
    QJsonObject object{{"PurseCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::purseHasPassword(QString Username, QString APIKey,
                                          QString NotaryID, QString Purse)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->Purse_HasPassword(NotaryID.toStdString(),
                                                               Purse.toStdString());
    QJsonObject object{{"PurseHasPassword", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::createPurse(QString Username, QString APIKey,
                                     QString NotaryID, QString InstrumentDefinitionID,
                                     QString OwnerID, QString SignerID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(OwnerID.toStdString())){
        QJsonObject object{{"Error", "Invalid OwnerID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SignerID.toStdString())){
        QJsonObject object{{"Error", "Invalid SignerID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->CreatePurse(NotaryID.toStdString(),
                                                                InstrumentDefinitionID.toStdString(),
                                                                OwnerID.toStdString(),
                                                                SignerID.toStdString());
    QJsonObject object{{"Purse", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::createPursePassphrase(QString Username, QString APIKey,
                                               QString NotaryID, QString InstrumentDefinitionID,
                                               QString SignerID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SignerID.toStdString())){
        QJsonObject object{{"Error", "Invalid SignerID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->CreatePurse_Passphrase(NotaryID.toStdString(),
                                                                           InstrumentDefinitionID.toStdString(),
                                                                           SignerID.toStdString());
    QJsonObject object{{"PursePassphrase", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::pursePeek(QString Username, QString APIKey,
                                   QString NotaryID, QString InstrumentDefinitionID,
                                   QString OwnerID, QString Purse)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(OwnerID.toStdString())){
        QJsonObject object{{"Error", "Invalid OwnerID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Purse_Peek(NotaryID.toStdString(),
                                                               InstrumentDefinitionID.toStdString(),
                                                               OwnerID.toStdString(),
                                                               Purse.toStdString());
    QJsonObject object{{"PursePeek", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::pursePop(QString Username, QString APIKey,
                                  QString NotaryID, QString InstrumentDefinitionID,
                                  QString OwnerOrSignerID, QString Purse)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(OwnerOrSignerID.toStdString())){
        QJsonObject object{{"Error", "Invalid OwnerID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Purse_Pop(NotaryID.toStdString(),
                                                              InstrumentDefinitionID.toStdString(),
                                                              OwnerOrSignerID.toStdString(),
                                                              Purse.toStdString());
    QJsonObject object{{"PursePop", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::purseEmpty(QString Username, QString APIKey,
                                    QString NotaryID, QString InstrumentDefinitionID,
                                    QString SignerID, QString Purse)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SignerID.toStdString())){
        QJsonObject object{{"Error", "Invalid SignerID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Purse_Empty(NotaryID.toStdString(),
                                                                InstrumentDefinitionID.toStdString(),
                                                                SignerID.toStdString(),
                                                                Purse.toStdString());
    QJsonObject object{{"PurseEmpty", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::pursePush(QString Username, QString APIKey,
                                   QString NotaryID, QString InstrumentDefinitionID,
                                   QString SignerID, QString OwnerID,
                                   QString Purse, QString Token)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SignerID.toStdString())){
        QJsonObject object{{"Error", "Invalid SignerID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(OwnerID.toStdString())){
        QJsonObject object{{"Error", "Invalid OwnerID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Purse_Push(NotaryID.toStdString(),
                                                               InstrumentDefinitionID.toStdString(),
                                                               SignerID.toStdString(),
                                                               OwnerID.toStdString(),
                                                               Purse.toStdString(),
                                                               Token.toStdString());
    QJsonObject object{{"PursePurse", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::walletImportPurse(QString Username, QString APIKey,
                                           QString NotaryID, QString InstrumentDefinitionID,
                                           QString NymID, QString Purse)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->Wallet_ImportPurse(NotaryID.toStdString(),
                                                                InstrumentDefinitionID.toStdString(),
                                                                NymID.toStdString(),
                                                                Purse.toStdString());
    QJsonObject object{{"WalletImportPurseResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::exchangePurse(QString Username, QString APIKey,
                                       QString NotaryID, QString InstrumentDefinitionID,
                                       QString NymID, QString Purse)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->exchangePurse(NotaryID.toStdString(),
                                                          InstrumentDefinitionID.toStdString(),
                                                          NymID.toStdString(),
                                                          Purse.toStdString());
    QJsonObject object{{"ExchangePurseResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::tokenChangeOwner(QString Username, QString APIKey,
                                          QString NotaryID, QString InstrumentDefinitionID,
                                          QString Token, QString SignerNymID,
                                          QString OldOwner, QString NewOwner)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SignerNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid SignerNymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Token_ChangeOwner(NotaryID.toStdString(),
                                                                      InstrumentDefinitionID.toStdString(),
                                                                      Token.toStdString(),
                                                                      SignerNymID.toStdString(),
                                                                      OldOwner.toStdString(),
                                                                      NewOwner.toStdString());
    QJsonObject object{{"TokenChangerOwnerResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::tokenGetID(QString Username, QString APIKey,
                                    QString NotaryID, QString InstrumentDefinitionID,
                                    QString Token)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Token_GetID(NotaryID.toStdString(),
                                                                InstrumentDefinitionID.toStdString(),
                                                                Token.toStdString());
    QJsonObject object{{"TokenID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::tokenGetDenomination(QString Username, QString APIKey,
                                              QString NotaryID, QString InstrumentDefinitionID,
                                              QString Token)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    qint64 result = opentxs::OTAPI_Wrap::It()->Token_GetDenomination(NotaryID.toStdString(),
                                                                     InstrumentDefinitionID.toStdString(),
                                                                     Token.toStdString());
    QJsonObject object{{"TokenDenomination", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::tokenGetSeries(QString Username, QString APIKey,
                                        QString NotaryID, QString InstrumentDefinitionID,
                                        QString Token)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->Token_GetSeries(NotaryID.toStdString(),
                                                            InstrumentDefinitionID.toStdString(),
                                                            Token.toStdString());
    QJsonObject object{{"TokenSeries", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::tokenGetValidFrom(QString Username, QString APIKey,
                                           QString NotaryID, QString InstrumentDefinitionID,
                                           QString Token)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    time64_t result = opentxs::OTAPI_Wrap::It()->Token_GetValidFrom(NotaryID.toStdString(),
                                                                    InstrumentDefinitionID.toStdString(),
                                                                    Token.toStdString());
    QJsonObject object{{"TokenValidFrom", QString(std::to_string(result).c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::tokenGetValidTo(QString Username, QString APIKey,
                                         QString NotaryID, QString InstrumentDefinitionID,
                                         QString Token)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    time64_t result = opentxs::OTAPI_Wrap::It()->Token_GetValidTo(NotaryID.toStdString(),
                                                                  InstrumentDefinitionID.toStdString(),
                                                                  Token.toStdString());
    QJsonObject object{{"TokenValidTo", QString(std::to_string(result).c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::tokenGetInstrumentDefinitionID(QString Username, QString APIKey,
                                                        QString Token)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Token_GetInstrumentDefinitionID(Token.toStdString());
    QJsonObject object{{"TokenInstrumentDefinitionID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::tokenGetNotaryID(QString Username, QString APIKey,
                                          QString Token)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Token_GetNotaryID(Token.toStdString());
    QJsonObject object{{"TokenNotaryID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::isBasketCurrency(QString Username, QString APIKey,
                                          QString InstrumentDefinitionID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->IsBasketCurrency(InstrumentDefinitionID.toStdString());
    QJsonObject object{{"IsBasketCurrency", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::basketGetMemberCount(QString Username, QString APIKey,
                                              QString InstrumentDefinitionID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->Basket_GetMemberCount(InstrumentDefinitionID.toStdString());
    QJsonObject object{{"BasketMemberCount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::basketGetMemberType(QString Username, QString APIKey,
                                             QString BasketInstrumentDefinitionID, int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(BasketInstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid BasketInstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Basket_GetMemberType(BasketInstrumentDefinitionID.toStdString(),
                                                                         Index);
    QJsonObject object{{"BasketMemberType", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::basketGetMinimumTransferAmount(QString Username, QString APIKey,
                                                        QString BasketInstrumentDefinitionID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(BasketInstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid BasketInstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    qint64 result = opentxs::OTAPI_Wrap::It()->Basket_GetMinimumTransferAmount(BasketInstrumentDefinitionID.toStdString());
    QJsonObject object{{"BasketMinimumTransferAmount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::basketGetMemberMinimumTransferAmount(QString Username, QString APIKey,
                                                              QString BasketInstrumentDefinitionID, int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(BasketInstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid BasketInstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    qint64 result = opentxs::OTAPI_Wrap::It()->Basket_GetMemberMinimumTransferAmount(BasketInstrumentDefinitionID.toStdString(),
                                                                                     Index);
    QJsonObject object{{"BasketMemberMinimumTransferAmount", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::pingNotary(QString Username, QString APIKey,
                                    QString NotaryID, QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->pingNotary(NotaryID.toStdString(),
                                                       NymID.toStdString());
    QJsonObject object{{"PingNotaryResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::registerNym(QString Username, QString APIKey,
                                     QString NotaryID, QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->registerNym(NotaryID.toStdString(),
                                                        NymID.toStdString());
    QJsonObject object{{"RegisterNymResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::unregisterNym(QString Username, QString APIKey,
                                       QString NotaryID, QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->unregisterNym(NotaryID.toStdString(),
                                                          NymID.toStdString());
    QJsonObject object{{"UnregisterNymResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetUsageCredits(QString Username, QString APIKey,
                                                QString Message)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    qint64 result = opentxs::OTAPI_Wrap::It()->Message_GetUsageCredits(Message.toStdString());
    QJsonObject object{{"MessageUsageCredits", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::usageCredits(QString Username, QString APIKey,
                                      QString NotaryID, QString NymID,
                                      QString NymIDCheck, qint64 Adjustment)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymIDCheck.toStdString())){
        QJsonObject object{{"Error", "Invalid NymIDCheck"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->usageCredits(NotaryID.toStdString(),
                                                         NymID.toStdString(),
                                                         NymIDCheck.toStdString(),
                                                         Adjustment);
    QJsonObject object{{"MessageUsageCredits", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::checkNym(QString Username, QString APIKey,
                                  QString NotaryID, QString NymID,
                                  QString NymIDCheck)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymIDCheck.toStdString())){
        QJsonObject object{{"Error", "Invalid NymIDCheck"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->checkNym(NotaryID.toStdString(),
                                                     NymID.toStdString(),
                                                     NymIDCheck.toStdString());
    QJsonObject object{{"CheckNymResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::sendNymMessage(QString Username, QString APIKey,
                                        QString NotaryID, QString NymID,
                                        QString NymIDRecipient,
                                        QString Message)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymIDRecipient.toStdString())){
        QJsonObject object{{"Error", "Invalid NymIDRecipient"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->sendNymMessage(NotaryID.toStdString(),
                                                           NymID.toStdString(),
                                                           NymIDRecipient.toStdString(),
                                                           Message.toStdString());
    QJsonObject object{{"SendNymMessageResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::sendNymInstrument(QString Username, QString APIKey,
                                           QString NotaryID, QString NymID,
                                           QString NymIDRecipient,
                                           QString Instrument, QString InstrumentForSender)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymIDRecipient.toStdString())){
        QJsonObject object{{"Error", "Invalid NymIDRecipient"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->sendNymInstrument(NotaryID.toStdString(),
                                                              NymID.toStdString(),
                                                              NymIDRecipient.toStdString(),
                                                              Instrument.toStdString(),
                                                              InstrumentForSender.toStdString());
    QJsonObject object{{"SendNymInstrumentResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getRequestNumber(QString Username, QString APIKey,
                                          QString NotaryID, QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->getRequestNumber(NotaryID.toStdString(),
                                                             NymID.toStdString());
    QJsonObject object{{"RequestNumber", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::registerInstrumentDefinition(QString Username, QString APIKey,
                                                      QString NotaryID, QString NymID,
                                                      QString Contract)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->registerInstrumentDefinition(NotaryID.toStdString(),
                                                                         NymID.toStdString(),
                                                                         Contract.toStdString());
    QJsonObject object{{"RegisterInstrumentDefinitionResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getInstrumentDefinition(QString Username, QString APIKey,
                                                 QString NotaryID, QString NymID,
                                                 QString InstrumentDefinitionID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->getInstrumentDefinition(NotaryID.toStdString(),
                                                                    NymID.toStdString(),
                                                                    InstrumentDefinitionID.toStdString());
    QJsonObject object{{"InstrumentDefinition", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getMint(QString Username, QString APIKey,
                                 QString NotaryID, QString NymID,
                                 QString InstrumentDefinitionID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->getMint(NotaryID.toStdString(),
                                                    NymID.toStdString(),
                                                    InstrumentDefinitionID.toStdString());
    QJsonObject object{{"Mint", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getAccountData(QString Username, QString APIKey,
                                        QString NotaryID, QString NymID,
                                        QString AccountID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->getAccountData(NotaryID.toStdString(),
                                                           NymID.toStdString(),
                                                           AccountID.toStdString());
    QJsonObject object{{"AccountData", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::generateBasketCreation(QString Username, QString APIKey,
                                                QString NymID, qint64 MinimumTransfer)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GenerateBasketCreation(NymID.toStdString(),
                                                                           MinimumTransfer);
    QJsonObject object{{"BasketCreation", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::addBasketCreationItem(QString Username, QString APIKey,
                                               QString NymID, QString Basket,
                                               QString InstrumentDefinitionID, qint64 MinimumTransfer)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->AddBasketCreationItem(NymID.toStdString(),
                                                                          Basket.toStdString(),
                                                                          InstrumentDefinitionID.toStdString(),
                                                                          MinimumTransfer);
    QJsonObject object{{"AddBasketCreationItemResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::issueBasket(QString Username, QString APIKey,
                                     QString NotaryID, QString NymID,
                                     QString Basket)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->issueBasket(NotaryID.toStdString(),
                                                        NymID.toStdString(),
                                                        Basket.toStdString());
    QJsonObject object{{"IssueBasketResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::generateBasketExchange(QString Username, QString APIKey,
                                                QString NotaryID, QString NymID,
                                                QString BasketInstrumentDefinitionID, QString BasketAssetAccountID,
                                                int TransferMultiple)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(BasketInstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid BasketInstrumentDefinitionID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(BasketAssetAccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid BasketAssetAccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GenerateBasketExchange(NotaryID.toStdString(),
                                                                           NymID.toStdString(),
                                                                           BasketInstrumentDefinitionID.toStdString(),
                                                                           BasketAssetAccountID.toStdString(),
                                                                           TransferMultiple);
    QJsonObject object{{"BasketExchange", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::addBasketExchangeItem(QString Username, QString APIKey,
                                               QString NotaryID, QString NymID,
                                               QString Basket, QString InstrumentDefinitionID,
                                               QString AssetAccountID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(InstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid InstrumentDefinitionID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AssetAccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AssetAccountID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->AddBasketExchangeItem(NotaryID.toStdString(),
                                                                          NymID.toStdString(),
                                                                          Basket.toStdString(),
                                                                          InstrumentDefinitionID.toStdString(),
                                                                          AssetAccountID.toStdString());
    QJsonObject object{{"AddBasketExchangeItemResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::exchangeBasket(QString Username, QString APIKey,
                                        QString NotaryID, QString NymID,
                                        QString BasketInstrumentDefinitionID, QString Basket,
                                        bool ExchangeDirection)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(BasketInstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid BasketInstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->exchangeBasket(NotaryID.toStdString(),
                                                           NymID.toStdString(),
                                                           BasketInstrumentDefinitionID.toStdString(),
                                                           Basket.toStdString(),
                                                           ExchangeDirection);
    QJsonObject object{{"ExchangeBasketResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getTransactionNumbers(QString Username, QString APIKey,
                                               QString NotaryID, QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->getTransactionNumbers(NotaryID.toStdString(),
                                                                  NymID.toStdString());
    QJsonObject object{{"TransactionNumbers", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::notarizeWithdrawal(QString Username, QString APIKey,
                                            QString NotaryID, QString NymID,
                                            QString AccountID, qint64 Amount)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->notarizeWithdrawal(NotaryID.toStdString(),
                                                               NymID.toStdString(),
                                                               AccountID.toStdString(),
                                                               Amount);
    QJsonObject object{{"NotarizeWithdrawalResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::notarizeDeposit(QString Username, QString APIKey,
                                         QString NotaryID, QString NymID,
                                         QString AccountID, QString Purse)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->notarizeDeposit(NotaryID.toStdString(),
                                                            NymID.toStdString(),
                                                            AccountID.toStdString(),
                                                            Purse.toStdString());
    QJsonObject object{{"NotarizeDepositResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::notarizeTransfer(QString Username, QString APIKey,
                                          QString NotaryID, QString NymID,
                                          QString AccountFrom, QString AccountTo,
                                          qint64 Amount, QString Note)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->notarizeTransfer(NotaryID.toStdString(),
                                                             NymID.toStdString(),
                                                             AccountFrom.toStdString(),
                                                             AccountTo.toStdString(),
                                                             Amount,
                                                             Note.toStdString());
    QJsonObject object{{"NotarizeTransferResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymbox(QString Username, QString APIKey,
                                   QString NotaryID, QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->getNymbox(NotaryID.toStdString(),
                                                      NymID.toStdString());
    QJsonObject object{{"NymBox", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::processInbox(QString Username, QString APIKey,
                                      QString NotaryID, QString NymID,
                                      QString AccountID, QString AccountLedger)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->processInbox(NotaryID.toStdString(),
                                                         NymID.toStdString(),
                                                         AccountID.toStdString(),
                                                         AccountLedger.toStdString());
    QJsonObject object{{"ProcessInboxResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::processNymbox(QString Username, QString APIKey,
                                       QString NotaryID, QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->processNymbox(NotaryID.toStdString(),
                                                          NymID.toStdString());
    QJsonObject object{{"ProcessNymBoxResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::withdrawVoucher(QString Username, QString APIKey,
                                         QString NotaryID, QString NymID,
                                         QString AccountID, QString RecipientNymID,
                                         QString ChequeMemo, qint64 Amount)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(RecipientNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid RecipientNymID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->withdrawVoucher(NotaryID.toStdString(),
                                                            NymID.toStdString(),
                                                            AccountID.toStdString(),
                                                            RecipientNymID.toStdString(),
                                                            ChequeMemo.toStdString(),
                                                            Amount);
    QJsonObject object{{"WithdrawVoucherResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::payDividend(QString Username, QString APIKey,
                                     QString NotaryID, QString IssuerNymID,
                                     QString DividendFromAccountID, QString SharesInstrumentDefinitionID,
                                     QString DividendMemo, qint64 AmountPerShare)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(IssuerNymID.toStdString())){
        QJsonObject object{{"Error", "Invalid IssuerNymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(DividendFromAccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid DividendFromAccountID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(SharesInstrumentDefinitionID.toStdString())){
        QJsonObject object{{"Error", "Invalid SharesInstrumentDefinitionID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->payDividend(NotaryID.toStdString(),
                                                        IssuerNymID.toStdString(),
                                                        DividendFromAccountID.toStdString(),
                                                        SharesInstrumentDefinitionID.toStdString(),
                                                        DividendMemo.toStdString(),
                                                        AmountPerShare);
    QJsonObject object{{"PayDividendResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::depositCheque(QString Username, QString APIKey,
                                       QString NotaryID, QString NymID,
                                       QString AccountID, QString Cheque)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->depositCheque(NotaryID.toStdString(),
                                                          NymID.toStdString(),
                                                          AccountID.toStdString(),
                                                          Cheque.toStdString());
    QJsonObject object{{"DepositChequeResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::depositPaymentPlan(QString Username, QString APIKey,
                                            QString NotaryID, QString NymID,
                                            QString PaymentPlan)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->depositPaymentPlan(NotaryID.toStdString(),
                                                               NymID.toStdString(),
                                                               PaymentPlan.toStdString());
    QJsonObject object{{"DepositPaymentPlanResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::killMarketOffer(QString Username, QString APIKey,
                                         QString NotaryID, QString NymID,
                                         QString AssetAccountID, qint64 TransactionNumber)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AssetAccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AssetAccountID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->killMarketOffer(NotaryID.toStdString(),
                                                            NymID.toStdString(),
                                                            AssetAccountID.toStdString(),
                                                            TransactionNumber);
    QJsonObject object{{"KillMarketOfferResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::killPaymentPlan(QString Username, QString APIKey,
                                         QString NotaryID, QString NymID,
                                         QString FromAccountID, qint64 TransactionNumber)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(FromAccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid FromAccountID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->killPaymentPlan(NotaryID.toStdString(),
                                                            NymID.toStdString(),
                                                            FromAccountID.toStdString(),
                                                            TransactionNumber);
    QJsonObject object{{"KillPaymentPlanResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::issueMarketOffer(QString Username, QString APIKey,
                                          QString AssetAccountID, QString CurrencyAccountID,
                                          qint64 MarketScale, qint64 MinimumIncrement,
                                          qint64 TotalAssetsOnOffer, qint64 PriceLimit,
                                          bool BuyingOrSelling, time64_t LifeSpanInSeconds,
                                          QString StopSign, qint64 ActivationPrice)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AssetAccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AssetAccountID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(CurrencyAccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid CurrencyAccountID"}};
        return QJsonValue(object);
    }


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

QJsonValue MCRPCService::getMarketList(QString Username, QString APIKey,
                                       QString NotaryID, QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->getMarketList(NotaryID.toStdString(),
                                                          NymID.toStdString());
    QJsonObject object{{"GetMarketListResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getMarketOffers(QString Username, QString APIKey,
                                         QString NotaryID, QString NymID,
                                         QString MarketID, qint64 MaxDepth)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(MarketID.toStdString())){
        QJsonObject object{{"Error", "Invalid MarketID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->getMarketOffers(NotaryID.toStdString(),
                                                            NymID.toStdString(),
                                                            MarketID.toStdString(),
                                                            MaxDepth);
    QJsonObject object{{"GetMarketOffersResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getMarketRecentTrades(QString Username, QString APIKey,
                                               QString NotaryID, QString NymID,
                                               QString MarketID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(MarketID.toStdString())){
        QJsonObject object{{"Error", "Invalid MarketID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->getMarketRecentTrades(NotaryID.toStdString(),
                                                                  NymID.toStdString(),
                                                                  MarketID.toStdString());
    QJsonObject object{{"GetMarketRecentTradesResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getNymMarketOffers(QString Username, QString APIKey,
                                            QString NotaryID, QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->getNymMarketOffers(NotaryID.toStdString(),
                                                               NymID.toStdString());
    QJsonObject object{{"GetNymMarketOffersResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::popMessageBuffer(QString Username, QString APIKey,
                                          qint64 RequestNumber, QString NotaryID,
                                          QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->PopMessageBuffer(RequestNumber,
                                                                     NotaryID.toStdString(),
                                                                     NymID.toStdString());
    QJsonObject object{{"PopMessageBufferResult", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::flushMessageBuffer(QString Username, QString APIKey)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    opentxs::OTAPI_Wrap::It()->FlushMessageBuffer();
    QJsonObject object{{"MessageBufferFlushed", "True"}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::getSentMessage(QString Username, QString APIKey,
                                        qint64 RequestNumber, QString NotaryID,
                                        QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->GetSentMessage(RequestNumber,
                                                                   NotaryID.toStdString(),
                                                                   NymID.toStdString());
    QJsonObject object{{"SentMessage", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::removeSentMessage(QString Username, QString APIKey,
                                           qint64 RequestNumber, QString NotaryID,
                                           QString NymID)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->RemoveSentMessage(RequestNumber,
                                                               NotaryID.toStdString(),
                                                               NymID.toStdString());
    QJsonObject object{{"RemoveSentMessageResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::flushSentMessages(QString Username, QString APIKey,
                                           bool HarvestingForRetry, QString NotaryID,
                                           QString NymID, QString NymBox)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    opentxs::OTAPI_Wrap::It()->FlushSentMessages(HarvestingForRetry,
                                                 NotaryID.toStdString(),
                                                 NymID.toStdString(),
                                                 NymBox.toStdString());
    QJsonObject object{{"FlushedSentMessages", "True"}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::sleep(QString Username, QString APIKey,
                               qint64 Milliseconds)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    opentxs::OTAPI_Wrap::It()->Sleep(Milliseconds);
    QJsonObject object{{"Sleep", Milliseconds}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::resyncNymWithServer(QString Username, QString APIKey,
                                             QString NotaryID, QString NymID,
                                             QString Message)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->ResyncNymWithServer(NotaryID.toStdString(),
                                                                 NymID.toStdString(),
                                                                 Message.toStdString());
    QJsonObject object{{"ResyncNymWithServerResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::queryInstrumentDefinitions(QString Username, QString APIKey,
                                                    QString NotaryID, QString NymID,
                                                    QString EncodedMap)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->queryInstrumentDefinitions(NotaryID.toStdString(),
                                                                       NymID.toStdString(),
                                                                       EncodedMap.toStdString());
    QJsonObject object{{"QueryInstrumentDefinitionsResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetPayload(QString Username, QString APIKey,
                                           QString Message)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Message_GetPayload(Message.toStdString());
    QJsonObject object{{"MessagePayload", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetCommand(QString Username, QString APIKey,
                                           QString Message)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Message_GetCommand(Message.toStdString());
    QJsonObject object{{"MessageCommand", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetLedger(QString Username, QString APIKey,
                                          QString Message)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Message_GetLedger(Message.toStdString());
    QJsonObject object{{"MessageLedger", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetNewInstrumentDefinitionID(QString Username, QString APIKey,
                                                             QString Message)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Message_GetNewInstrumentDefinitionID(Message.toStdString());
    QJsonObject object{{"MessageNewInstrumentDefinitionID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetNewIssuerAccountID(QString Username, QString APIKey,
                                                      QString Message)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Message_GetNewIssuerAcctID(Message.toStdString());
    QJsonObject object{{"MessageNewIssuerAccountID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetNewAccountID(QString Username, QString APIKey,
                                                QString Message)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Message_GetNewAcctID(Message.toStdString());
    QJsonObject object{{"MessageNewAccountID", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetNymboxHash(QString Username, QString APIKey,
                                              QString Message)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    std::string result = opentxs::OTAPI_Wrap::It()->Message_GetNymboxHash(Message.toStdString());
    QJsonObject object{{"MessageNymboxHash", QString(result.c_str())}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetSuccess(QString Username, QString APIKey,
                                           QString Message)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->Message_GetSuccess(Message.toStdString());
    QJsonObject object{{"MessageNymboxHash", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetDepth(QString Username, QString APIKey,
                                         QString Message)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }

    int result = opentxs::OTAPI_Wrap::It()->Message_GetDepth(Message.toStdString());
    QJsonObject object{{"MessageDepth", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageIsTransactionCanceled(QString Username, QString APIKey,
                                                      QString NotaryID, QString NymID,
                                                      QString AccountID, QString Message)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->Message_IsTransactionCanceled(NotaryID.toStdString(),
                                                                           NymID.toStdString(),
                                                                           AccountID.toStdString(),
                                                                           Message.toStdString());
    QJsonObject object{{"MessageIsTransactionCanceledResult", result}};
    return QJsonValue(object);
}

QJsonValue MCRPCService::messageGetTransactionSuccess(QString Username, QString APIKey,
                                                      QString NotaryID, QString NymID,
                                                      QString AccountID, QString Message)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NotaryID.toStdString())){
        QJsonObject object{{"Error", "Invalid NotaryID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(NymID.toStdString())){
        QJsonObject object{{"Error", "Invalid NymID"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(AccountID.toStdString())){
        QJsonObject object{{"Error", "Invalid AccountID"}};
        return QJsonValue(object);
    }

    bool result = opentxs::OTAPI_Wrap::It()->Message_GetTransactionSuccess(NotaryID.toStdString(),
                                                                           NymID.toStdString(),
                                                                           AccountID.toStdString(),
                                                                           Message.toStdString());
    QJsonObject object{{"MessageTransactionSuccess", result}};
    return QJsonValue(object);
}



// Moneychanger::It() methods

QString MCRPCService::mcSendDialog(QString Username, QString APIKey,
                                   QString Account, QString Recipient,
                                   QString Asset, QString Amount)
{
    if(!validateAPIKey(Username, APIKey)){
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

    Moneychanger::It()->mc_rpc_sendfunds_show_dialog(Account, Recipient, Asset, Amount);
    return "Success";
}

QString MCRPCService::mcRequestFundsDialog(QString Username, QString APIKey,
                                           QString Account, QString Recipient,
                                           QString Asset, QString Amount)
{
    if(!validateAPIKey(Username, APIKey)){
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

    Moneychanger::It()->mc_rpc_requestfunds_show_dialog(Account, Recipient, Asset, Amount);
    return "Success";
}

QString MCRPCService::mcMessagesDialog(QString Username, QString APIKey){
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QString("Error: Invalid API Key");
    }

    Moneychanger::It()->mc_rpc_messages_show_dialog();

    return "Success";
}

QString MCRPCService::mcExchangeDialog(QString Username, QString APIKey){
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QString("Error: Invalid API Key");
    }

    Moneychanger::It()->mc_rpc_exchange_show_dialog();
    return "Success";
}

QString MCRPCService::mcPaymentsDialog(QString Username, QString APIKey){
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QString("Error: Invalid API Key");
    }

    return "Not yet implemented";
}

QString MCRPCService::mcManageAccountsDialog(QString Username, QString APIKey){
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QString("Error: Invalid API Key");
    }

    Moneychanger::It()->mc_rpc_manage_accounts_show_dialog();
    return "Success";
}

QString MCRPCService::mcManageNymsDialog(QString Username, QString APIKey){
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QString("Error: Invalid API Key");
    }

    Moneychanger::It()->mc_rpc_manage_nyms_show_dialog();
    return "Success";
}

QString MCRPCService::mcManageAssetsDialog(QString Username, QString APIKey){
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QString("Error: Invalid API Key");
    }

    Moneychanger::It()->mc_rpc_manage_assets_show_dialog();
    return "Success";
}

QString MCRPCService::mcManageSmartContractsDialog(QString Username, QString APIKey){
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QString("Error: Invalid API Key");
    }

    Moneychanger::It()->mc_rpc_manage_smartcontracts_show_dialog();
    return "Success";
}

bool MCRPCService::mcActivateSmartContract(QString Username, QString APIKey)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return false;
    }
    // This function should take the ID of a smart contract
    // And launch the Smart Contracts wizard accordingly
    return false;
}

QJsonValue MCRPCService::mcListSmartContracts(QString Username, QString APIKey)
{
    if(!validateAPIKey(Username, APIKey)){
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
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }


    if(m_RecordList == nullptr)
        createRecordList(Username, APIKey);

    int nServerCount  = opentxs::OTAPI_Wrap::It()->GetServerCount();
    int nAssetCount   = opentxs::OTAPI_Wrap::It()->GetAssetTypeCount();
    int nNymCount     = opentxs::OTAPI_Wrap::It()->GetNymCount();
    int nAccountCount = opentxs::OTAPI_Wrap::It()->GetAccountCount();
    // ----------------------------------------------------
    for (int ii = 0; ii < nServerCount; ++ii)
    {
        std::string NotaryID = opentxs::OTAPI_Wrap::It()->GetServer_ID(ii);
        m_RecordList->AddNotaryID(NotaryID);
    }
    // ----------------------------------------------------
    for (int ii = 0; ii < nAssetCount; ++ii)
    {
        std::string InstrumentDefinitionID = opentxs::OTAPI_Wrap::It()->GetAssetType_ID(ii);
        m_RecordList->AddInstrumentDefinitionID(InstrumentDefinitionID);
    }
    // ----------------------------------------------------
    for (int ii = 0; ii < nNymCount; ++ii)
    {
        std::string nymId = opentxs::OTAPI_Wrap::It()->GetNym_ID(ii);
        m_RecordList->AddNymID(nymId);
    }
    // ----------------------------------------------------
    for (int ii = 0; ii < nAccountCount; ++ii)
    {
        std::string accountID = opentxs::OTAPI_Wrap::It()->GetAccountWallet_ID(ii);
        m_RecordList->AddAccountID(accountID);
    }
    // ----------------------------------------------------
    m_RecordList->AcceptChequesAutomatically  (true);
    m_RecordList->AcceptReceiptsAutomatically (true);
    m_RecordList->AcceptTransfersAutomatically(false);

    m_RecordList->Populate();

    QJsonObject object{{"RecordListPopulated", "True"}};

    return object;
}

QJsonValue MCRPCService::recordListCount(QString Username, QString APIKey)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }


    QJsonObject reply_obj;
    if(m_RecordList == nullptr)
    {
        qDebug() << QString("Record List Null");
        QJsonValue reply_val = recordListPopulate(Username, APIKey);
        reply_obj.insert("RecordListPopulated", reply_val);
    }

    if (m_RecordList == nullptr)
    {
        QJsonObject object{{"Error", "Recordlist is a null pointer."}};
        return object;
    }
    /*
    if (QJsonValue("True") != reply_obj.value("RecordListPopulated"))
    {
        qDebug() << QString("Recordlist didn't populate properly: %1").arg(reply_obj.value("RecordListPopulated").toString());
        QJsonObject object{{"Error", QString("Recordlist didn't populate properly: %1").arg(reply_obj.value("RecordListPopulated").toString())}};
        return object;
    }
*/
    m_RecordList->Populate();
    int count = m_RecordList->size();

    QJsonObject object{{"RecordListCount", count}};

    return object;
}

QJsonValue MCRPCService::recordListRetrieve(QString Username, QString APIKey,
                                            int Index)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }


    //if(m_RecordList == nullptr)
    //    recordListPopulate(Username, APIKey);
    m_RecordList->Populate();
    int count = m_RecordList->size();

    if(Index >= count)
        Index = count;

    if(Index < 0)
        Index = 0;



    QJsonObject object;

    QJsonObject record{{"AccountID", QString(m_RecordList->GetRecord(Index).GetAccountID().c_str())},
                       {"Address", QString(m_RecordList->GetRecord(Index).GetAddress().c_str())},
                       {"Amount", QString(m_RecordList->GetRecord(Index).GetAmount().c_str())},
                       {"BoxIndex", m_RecordList->GetRecord(Index).GetBoxIndex()},
                       {"Contents", QString(m_RecordList->GetRecord(Index).GetContents().c_str())},
                       {"CurrencyTLA", QString(m_RecordList->GetRecord(Index).GetCurrencyTLA().c_str())},
                       {"Date", QString(m_RecordList->GetRecord(Index).GetDate().c_str())},
                       {"InitialPaymentAmount", qint64(m_RecordList->GetRecord(Index).GetInitialPaymentAmount())},
                       {"InitialPaymentDate", qint64(m_RecordList->GetRecord(Index).GetInitialPaymentDate())},
                       {"InstrumentDefinitionID", QString(m_RecordList->GetRecord(Index).GetInstrumentDefinitionID().c_str())},
                       {"InstrumentType", QString(m_RecordList->GetRecord(Index).GetInstrumentType().c_str())},
                       {"MaximumNoPayments", m_RecordList->GetRecord(Index).GetMaximumNoPayments()},
                       {"Memo", QString(m_RecordList->GetRecord(Index).GetMemo().c_str())},
                       {"MethodID", m_RecordList->GetRecord(Index).GetMethodID()},
                       {"MesssageID", QString(m_RecordList->GetRecord(Index).GetMsgID().c_str())},
                       {"MessageType", QString(m_RecordList->GetRecord(Index).GetMsgType().c_str())},
                       {"MessageTypeDisplay", QString(m_RecordList->GetRecord(Index).GetMsgTypeDisplay().c_str())},
                       {"Name", QString(m_RecordList->GetRecord(Index).GetName().c_str())},
                       {"NotaryID", QString(m_RecordList->GetRecord(Index).GetNotaryID().c_str())},
                       {"NymID", QString(m_RecordList->GetRecord(Index).GetNymID().c_str())},
                       {"OtherAccountID", QString(m_RecordList->GetRecord(Index).GetOtherAccountID().c_str())},
                       {"OtherAddress", QString(m_RecordList->GetRecord(Index).GetOtherAddress().c_str())},
                       {"OtherNymID", QString(m_RecordList->GetRecord(Index).GetOtherNymID().c_str())},
                       {"PaymentPlanAmount", qint64(m_RecordList->GetRecord(Index).GetPaymentPlanAmount())},
                       {"PaymentPlanStartDate", qint64(m_RecordList->GetRecord(Index).GetPaymentPlanStartDate())},
                       {"RecordType", m_RecordList->GetRecord(Index).GetRecordType()},
                       {"TimeBetweenPayments", qint64(m_RecordList->GetRecord(Index).GetTimeBetweenPayments())},
                       {"TransactionNum", qint64(m_RecordList->GetRecord(Index).GetTransactionNum())},
                       {"TransNumForDisplay", qint64(m_RecordList->GetRecord(Index).GetTransNumForDisplay())},
                       {"ValidFrom", qint64(m_RecordList->GetRecord(Index).GetValidFrom())},
                       {"ValidTo", qint64(m_RecordList->GetRecord(Index).GetValidTo())}
                      };

    object.insert(QString(Index), record);


    return object;
}

QJsonValue MCRPCService::recordListRetrieve(QString Username, QString APIKey,
                                            int BeginIndex, int EndIndex)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }


    //enum OTRecordType { Mail = 0, Transfer, Receipt, Instrument,ErrorState };

    QJsonObject reply_obj;
    if(m_RecordList == nullptr)
    {
        QJsonValue reply_val = recordListPopulate(Username, APIKey);
        reply_obj.insert("RecordListPopulated", reply_val);
    }

    if (m_RecordList == nullptr)
    {
        QJsonObject object{{"Error", "Recordlist is a null pointer."}};
        return object;
    }

    /*if (QJsonValue("True") != reply_obj.value("RecordListPopulated"))
    {
        qDebug() << QString("Recordlist didn't populate properly: %1").arg(reply_obj.value("RecordListPopulated").toString());
        QJsonObject object{{"Error", QString("Recordlist didn't populate properly: %1").arg(reply_obj.value("RecordListPopulated").toString())}};
        return object;
    }*/

    m_RecordList->Populate();
    int count = m_RecordList->size();

    // Swap if Begin > End
    if(BeginIndex > EndIndex){
        BeginIndex^=EndIndex;
        EndIndex^=BeginIndex;
        BeginIndex^=EndIndex;
    }

    if(BeginIndex < 0)
        BeginIndex = 0;

    if(BeginIndex > count){
        QJsonObject object{{"Error", "Out of Bound Request"}};
        return object;
    }

    if(EndIndex > count)
        EndIndex = count;

    QJsonObject object;
    for(auto x = BeginIndex; x < EndIndex; x++){

        QJsonObject record{{"AccountID", QString(m_RecordList->GetRecord(x).GetAccountID().c_str())},
                           {"Address", QString(m_RecordList->GetRecord(x).GetAddress().c_str())},
                           {"Amount", QString(m_RecordList->GetRecord(x).GetAmount().c_str())},
                           {"BoxIndex", m_RecordList->GetRecord(x).GetBoxIndex()},
                           {"Contents", QString(m_RecordList->GetRecord(x).GetContents().c_str())},
                           {"CurrencyTLA", QString(m_RecordList->GetRecord(x).GetCurrencyTLA().c_str())},
                           {"Date", QString(m_RecordList->GetRecord(x).GetDate().c_str())},
                           {"InitialPaymentAmount", qint64(m_RecordList->GetRecord(x).GetInitialPaymentAmount())},
                           {"InitialPaymentDate", qint64(m_RecordList->GetRecord(x).GetInitialPaymentDate())},
                           {"InstrumentDefinitionID", QString(m_RecordList->GetRecord(x).GetInstrumentDefinitionID().c_str())},
                           {"InstrumentType", QString(m_RecordList->GetRecord(x).GetInstrumentType().c_str())},
                           {"MaximumNoPayments", m_RecordList->GetRecord(x).GetMaximumNoPayments()},
                           {"Memo", QString(m_RecordList->GetRecord(x).GetMemo().c_str())},
                           {"MethodID", m_RecordList->GetRecord(x).GetMethodID()},
                           {"MesssageID", QString(m_RecordList->GetRecord(x).GetMsgID().c_str())},
                           {"MessageType", QString(m_RecordList->GetRecord(x).GetMsgType().c_str())},
                           {"MessageTypeDisplay", QString(m_RecordList->GetRecord(x).GetMsgTypeDisplay().c_str())},
                           {"Name", QString(m_RecordList->GetRecord(x).GetName().c_str())},
                           {"NotaryID", QString(m_RecordList->GetRecord(x).GetNotaryID().c_str())},
                           {"NymID", QString(m_RecordList->GetRecord(x).GetNymID().c_str())},
                           {"OtherAccountID", QString(m_RecordList->GetRecord(x).GetOtherAccountID().c_str())},
                           {"OtherAddress", QString(m_RecordList->GetRecord(x).GetOtherAddress().c_str())},
                           {"OtherNymID", QString(m_RecordList->GetRecord(x).GetOtherNymID().c_str())},
                           {"PaymentPlanAmount", qint64(m_RecordList->GetRecord(x).GetPaymentPlanAmount())},
                           {"PaymentPlanStartDate", qint64(m_RecordList->GetRecord(x).GetPaymentPlanStartDate())},
                           {"RecordType", m_RecordList->GetRecord(x).GetRecordType()},
                           {"TimeBetweenPayments", qint64(m_RecordList->GetRecord(x).GetTimeBetweenPayments())},
                           {"TransactionNum", qint64(m_RecordList->GetRecord(x).GetTransactionNum())},
                           {"TransNumForDisplay", qint64(m_RecordList->GetRecord(x).GetTransNumForDisplay())},
                           {"ValidFrom", qint64(m_RecordList->GetRecord(x).GetValidFrom())},
                           {"ValidTo", qint64(m_RecordList->GetRecord(x).GetValidTo())}
                          };

        object.insert(QString(x), record);

    }

    return object;
}


bool MCRPCService::createRecordList(QString Username, QString APIKey)
{
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return false;
    }

    if(m_RecordList == nullptr)
        m_RecordList = new opentxs::OTRecordList(*(new MTNameLookupQT));
    qDebug() << QString("Record List Created");
    return true;
}


QJsonValue MCRPCService::setDefaultNym(QString Username, QString APIKey,
                                       QString NymID, QString NymName){

    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QString("Error: Invalid API Key");
    }

    Moneychanger::It()->setDefaultNym(NymID, NymName);
    return "Success";
}

QJsonValue MCRPCService::getDefaultNym(QString Username, QString APIKey){

    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QString("Error: Invalid API Key");
    }

    QJsonObject record{{"NymID", Moneychanger::It()->getDefaultNymID()},
                       {"NymName", Moneychanger::It()->getDefaultNymName()}};

    return record;
}

QJsonValue MCRPCService::setDefaultAccount(QString Username, QString APIKey,
                                           QString AccountID, QString AccountName){

    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QString("Error: Invalid API Key");
    }

    Moneychanger::It()->setDefaultAccount(AccountID, AccountName);
    return "Success";
}

QJsonValue MCRPCService::getDefaultAccount(QString Username, QString APIKey){

    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QString("Error: Invalid API Key");
    }

    QJsonObject record{{"AccountID", Moneychanger::It()->getDefaultAccountID()},
                       {"AccountName", Moneychanger::It()->getDefaultAccountName()}};

    return record;

}

QJsonValue MCRPCService::setDefaultServer(QString Username, QString APIKey,
                                          QString NotaryID, QString ServerName){

    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QString("Error: Invalid API Key");
    }

    Moneychanger::It()->setDefaultServer(NotaryID, ServerName);
    return "Success";

}

QJsonValue MCRPCService::getDefaultServer(QString Username, QString APIKey){

    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QString("Error: Invalid API Key");
    }

    QJsonObject record{{"NotaryID", Moneychanger::It()->getDefaultNotaryID()},
                       {"ServerName", Moneychanger::It()->getDefaultServerName()}};

    return record;
}

QJsonValue MCRPCService::setDefaultAsset(QString Username, QString APIKey,
                                         QString AssetID, QString AssetName){

    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QString("Error: Invalid API Key");
    }

    Moneychanger::It()->setDefaultServer(AssetID, AssetName);
    return "Success";

}

QJsonValue MCRPCService::getDefaultAsset(QString Username, QString APIKey){

    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QString("Error: Invalid API Key");
    }

    QJsonObject record{{"AssetID", Moneychanger::It()->getDefaultAssetID()},
                       {"AssetName", Moneychanger::It()->getDefaultAssetName()}};
    return record;
}


QJsonValue MCRPCService::isValidID(QString ID, QString Username, QString APIKey){
    if(!validateAPIKey(Username, APIKey)){
        QJsonObject object{{"Error", "Invalid API Key"}};
        return QJsonValue(object);
    }
    if(!opentxs::OTAPI_Wrap::It()->IsValidID(ID.toStdString())){
        QJsonObject object{{"IsValidID", "False"}};
        return QJsonValue(object);
    }
    else{
        QJsonObject object{{"IsValidID", "True"}};
        return QJsonValue(object);
    }
}


QJsonValue MCRPCService::userLogin(QString Username, QString PlaintextPassword)
{
    if(m_userManager.activateUserAccount(Username, PlaintextPassword)){
        QString userKey;
        userKey = m_userManager.getAPIKey(Username);
        QJsonObject object{{"Success", "User Activated"},
                           {"APIKey", userKey}};
        return object;
    }
    else{
        QJsonObject object{{"Error", "User Login Failed"}};
        return object;
    }

}


QJsonValue MCRPCService::userLogout(QString Username, QString PlaintextPassword)
{
    if(m_userManager.validateUserInDatabase(Username, PlaintextPassword)){
        m_userManager.deactivateUserAccount(Username);
        QJsonObject object{{"Success", "User Logged Out"}};
        return object;
    }
    else{
        QJsonObject object{{"Error", "User Credentials Not Validated"}};
        return object;
    }

}

QJsonValue MCRPCService::refreshAPIKey(QString Username, QString PlaintextPassword)
{
    if(!m_userManager.checkUserActivated(Username)){
        QJsonObject object{{"Error", "User Not Logged In"}};
        return object;
    }
    else{
        if(m_userManager.validateUserInDatabase(Username, PlaintextPassword)){
            QString l_apiKey = m_userManager.getAPIKey(Username);
            QJsonObject object {{"Username", Username},
                                {"APIKey", l_apiKey}};
            return object;
        }
        else{
            QJsonObject object{{"Error", "User Credentials Invalid"}};
            return object;
        }
    }
}

bool MCRPCService::validateAPIKey(QString Username, QString APIKey)
{
    if(!m_userManager.checkUserActivated(Username)){
            return false;
    }
    else{
        if(m_userManager.validateAPIKey(Username, APIKey)){
            return true;
        }
        else{
            return false;
        }
    }
}

