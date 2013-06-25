#include "ot_worker.h"

ot_worker::ot_worker(QObject *parent) :
    QObject(parent)
{
    /** ** ** **
     ** Init MTList
     **/
    int nServerCount  = OTAPI_Wrap::GetServerCount();
    int nAssetCount   = OTAPI_Wrap::GetAssetTypeCount();
    int nNymCount     = OTAPI_Wrap::GetNymCount();
    int nAccountCount = OTAPI_Wrap::GetAccountCount();
    // ----------------------------------------------------
    for (int ii = 0; ii < nServerCount; ++ii)
    {
        std::string serverId = OTAPI_Wrap::GetServer_ID(ii);
        list.AddServerID(serverId);
    }
    // ----------------------------------------------------
    for (int ii = 0; ii < nAssetCount; ++ii)
    {
        std::string assetId = OTAPI_Wrap::GetAssetType_ID(ii);
        list.AddAssetID(assetId);
    }
    // ----------------------------------------------------
    for (int ii = 0; ii < nNymCount; ++ii)
    {
        std::string nymId = OTAPI_Wrap::GetNym_ID(ii);
        list.AddNymID(nymId);
    }
    // ----------------------------------------------------
    for (int ii = 0; ii < nAccountCount; ++ii)
    {
        std::string accountID = OTAPI_Wrap::GetAccountWallet_ID(ii);
        list.AddAccountID(accountID);
    }
    // ----------------------------------------------------
    list.AcceptChequesAutomatically  (true);
    list.AcceptReceiptsAutomatically (true);
    list.AcceptTransfersAutomatically(true);

}


void ot_worker::mc_overview_ping(){

    int listSize = list.size();
    for(int a = 0;a < listSize;a++){
        weak_ptr_MTRecord weakRecord = list.GetRecord(a);
        shared_ptr_MTRecord record = weakRecord.lock();
        if (weakRecord.expired()) {
          OTLog::Output(2, "Reloading table due to expired pointer");
          list.Populate();

          //Repopulating, wait till next ping(or timer ping)
          a = listSize;
        }else{
            qDebug() << "FILLING CELL\n========\n";
            MTRecord recordmt = *record;
            qDebug() << recordmt.IsOutgoing();
            qDebug() << recordmt.IsPending();
            qDebug() << recordmt.IsReceipt();
            qDebug() << recordmt.IsRecord();
            qDebug() << QString::fromStdString(recordmt.GetAccountID());
            qDebug() << QString::fromStdString(recordmt.GetAmount());
            qDebug() << QString::fromStdString(recordmt.GetAssetID());
            qDebug() << QString::fromStdString(recordmt.GetCurrencyTLA());
            qDebug() << QString::fromStdString(recordmt.GetDate());
            qDebug() << QString::fromStdString(recordmt.GetInstrumentType());
            qDebug() << QString::fromStdString(recordmt.GetName());
            qDebug() << QString::fromStdString(recordmt.GetNymID());
            qDebug() << recordmt.GetRecordType();
            qDebug() << QString::fromStdString(recordmt.GetServerID());







        }
    }
    list.Populate();
}
