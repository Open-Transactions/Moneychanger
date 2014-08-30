#ifndef SAMPLEESCROWMANAGER_HPP
#define SAMPLEESCROWMANAGER_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <bitcoin/sampleescrowclient.hpp>

class BtcGuiTest;

class SampleEscrowManager : public QObject
{
public:
    SampleEscrowManager();

    void OnSimulateEscrowServers();

    void OnInitializeEscrow(BtcGuiTest* btcGuiTest);

    void OnRequestWithdrawal(BtcGuiTest* btcGuiTest);

private:    
    SampleEscrowClientPtr client;
    EscrowPoolPtr escrowPool;

//    Q_OBJECT
//public slots:
};

#endif // SAMPLEESCROWMANAGER_HPP
