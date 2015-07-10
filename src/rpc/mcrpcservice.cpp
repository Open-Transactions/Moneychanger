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
