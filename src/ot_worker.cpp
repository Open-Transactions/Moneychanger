#include "ot_worker.h"

ot_worker::ot_worker(QObject *parent) :
    QObject(parent)
{
    /** ** ** **
     ** Init MTList
     **/
    overview_list = new QList< QMap<QString,QVariant> >();

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

    //Populate
    list.Populate();
}


void ot_worker::mc_overview_ping(){
    //Lock overview_list (Unlocks when function is returned)
    QMutexLocker overview_list_mutex_locker(&overview_list_mutex);

    //Repopulate the list
    list.Populate();
    int listSize = list.size();
        //Clear backend memory to the visual table.
        for(int a = 0; a < overview_list->size();a++){
            overview_list->removeAt(0);
        }

    //REadd to the backend memory to the visual table.
    for(int a = 0;a < listSize;a++){
        weak_ptr_MTRecord weakRecord = list.GetRecord(a);
        shared_ptr_MTRecord record = weakRecord.lock();
        if (weakRecord.expired()) {
          OTLog::Output(2, "Reloading table due to expired pointer");
          list.Populate();
          listSize = list.size();
          a = 0;

        }else{
            MTRecord recordmt = *record;

            /** Refernce Comment/Code **
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
            ** End of Reference Comment/Code **/

            //Add to overview list
                //Map of record
                QMap<QString, QVariant> record_map = QMap<QString,QVariant>();
                record_map.insert("isoutgoing", recordmt.IsOutgoing());
                record_map.insert("ispending", recordmt.IsPending());
                record_map.insert("isreceipt", recordmt.IsReceipt());
                record_map.insert("isrecord", recordmt.IsRecord());
                record_map.insert("accountId", QString::fromStdString(recordmt.GetAccountID()));
                record_map.insert("amount", QString::fromStdString(recordmt.GetAmount()));
                record_map.insert("assetId", QString::fromStdString(recordmt.GetAssetID()));
                record_map.insert("currencyTLA", QString::fromStdString(recordmt.GetCurrencyTLA()));
                record_map.insert("nymId", QString::fromStdString(recordmt.GetNymID()));
                record_map.insert("name", QString::fromStdString(recordmt.GetName()));
                record_map.insert("serverId", QString::fromStdString(recordmt.GetServerID()));
                record_map.insert("recordType", recordmt.GetRecordType());

                //Append
                overview_list->append(record_map);
        }
    }
}


/**
 ** Return the current overview_list (if not locked)
 **/
QList< QMap<QString, QVariant> > ot_worker::mc_overview_get_currentlist(){

    //Lock overview_list (Unlocks when function is returned)
    QMutexLocker overview_list_mutex_locker(&overview_list_mutex);

    //Make "DEEP COPY" of overviewlist
    QList< QMap<QString, QVariant> > deep_copy_overview_list = QList< QMap<QString, QVariant> >();
        //Copy
        for(int a = 0; a < overview_list->size(); a++){
            deep_copy_overview_list.append(overview_list->at(a));
        }

    return deep_copy_overview_list;
}

