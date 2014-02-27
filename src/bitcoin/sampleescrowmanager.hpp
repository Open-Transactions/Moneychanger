#ifndef SAMPLEESCROWMANAGER_H
#define SAMPLEESCROWMANAGER_H

#include <QObject>
#include <QList>
#include <QSharedPointer>
//#include "sampletypedefs.h"
#include "sampleescrowserver.h"
#include "sampleescrowclient.h"
#include "widgets/btcguitest.h"

class SampleEscrowManager : public QObject
{
public:
    SampleEscrowManager();

    void OnSimulateEscrowServers();

    void OnInitializeEscrow(BtcGuiTest* btcGuiTest);

    void OnRequestWithdrawal(BtcGuiTest* BtcGuiTest);

private:    
    SampleEscrowClientPtr client;
    EscrowPoolPtr escrowPool;

//    Q_OBJECT
//public slots:
};

#endif // SAMPLEESCROWMANAGER_H
