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
    */
    if(Recipient.isEmpty())
    {
        return "Error: Recipient not defined";
    }

    Moneychanger::It()->mc_rpc_sendfunds_show_dialog(Account, Recipient, Asset, Amount);
    return "Success";
}

