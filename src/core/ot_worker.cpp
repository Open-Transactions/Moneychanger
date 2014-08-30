#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <core/ot_worker.hpp>

#include <core/handlers/contacthandler.hpp>

#include <opentxs/api/OTAPI.hpp>
#include <opentxs/api/OTAPI_Exec.hpp>
#include <opentxs/core/OTLog.hpp>

#include <QVariant>

ot_worker::ot_worker(QObject *parent) : QObject(parent), list(*(new MTNameLookupQT))
{
    /** ** ** **
     ** Init MTList
     **/
    overview_list = new QList< QMap<QString,QVariant> >();
    
    int nServerCount  = opentxs::OTAPI_Wrap::It()->GetServerCount();
    int nAssetCount   = opentxs::OTAPI_Wrap::It()->GetAssetTypeCount();
    int nNymCount     = opentxs::OTAPI_Wrap::It()->GetNymCount();
    int nAccountCount = opentxs::OTAPI_Wrap::It()->GetAccountCount();
    // ----------------------------------------------------
    for (int ii = 0; ii < nServerCount; ++ii)
    {
        std::string serverId = opentxs::OTAPI_Wrap::It()->GetServer_ID(ii);
        list.AddServerID(serverId);
    }
    // ----------------------------------------------------
    for (int ii = 0; ii < nAssetCount; ++ii)
    {
        std::string assetId = opentxs::OTAPI_Wrap::It()->GetAssetType_ID(ii);
        list.AddAssetID(assetId);
    }
    // ----------------------------------------------------
    for (int ii = 0; ii < nNymCount; ++ii)
    {
        std::string nymId = opentxs::OTAPI_Wrap::It()->GetNym_ID(ii);
        list.AddNymID(nymId);
    }
    // ----------------------------------------------------
    for (int ii = 0; ii < nAccountCount; ++ii)
    {
        std::string accountID = opentxs::OTAPI_Wrap::It()->GetAccountWallet_ID(ii);
        list.AddAccountID(accountID);
    }
    // ----------------------------------------------------
    list.AcceptChequesAutomatically  (true);
    list.AcceptReceiptsAutomatically (true);
    list.AcceptTransfersAutomatically(false);
    
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
        opentxs::weak_ptr_OTRecord weakRecord = list.GetRecord(a);
        opentxs::shared_ptr_OTRecord record = weakRecord.lock();
        if (weakRecord.expired()) {
            opentxs::OTLog::Output(2, "Reloading table due to expired pointer");
            list.Populate();
            listSize = list.size();
            a = 0;
            
        }else{
            opentxs::OTRecord recordmt = *record;
            
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
            
            /*
             bool  IsPending()     const;
             bool  IsOutgoing()    const;
             bool  IsRecord()      const;
             bool  IsReceipt()     const;
             bool  IsMail()        const;
             bool  IsTransfer()    const;
             bool  IsCheque()      const;
             bool  IsInvoice()     const;
             bool  IsVoucher()     const;
             bool  IsContract()    const;
             bool  IsPaymentPlan() const;
             bool  IsCash()        const;
             bool  HasContents()   const;
             bool  HasMemo()       const;
             // ---------------------------------------
             bool  IsExpired()     const;
             bool  IsCanceled()    const;
             
             time_t GetValidFrom();
             time_t GetValidTo();
             
             bool  CanDeleteRecord()        const;  // For completed records (not pending.)
             bool  CanAcceptIncoming()      const;  // For incoming, pending (not-yet-accepted) instruments.
             bool  CanDiscardIncoming()     const;  // For INcoming, pending (not-yet-accepted) instruments.
             bool  CanCancelOutgoing()      const;  // For OUTgoing, pending (not-yet-accepted) instruments.
             bool  CanDiscardOutgoingCash() const;  // For OUTgoing cash. (No way to see if it's been accepted, so this lets you erase the record of sending it.)
             
             int   GetBoxIndex() const;
             long  GetTransactionNum() const;
             long  GetTransNumForDisplay() const;
             
             OTRecordType  GetRecordType() const;
             // ---------------------------------------
             const std::string & GetServerID()       const;
             const std::string & GetAssetID()        const;
             const std::string & GetCurrencyTLA()    const; // BTC, USD, etc.
             const std::string & GetNymID()          const;
             const std::string & GetAccountID()      const;
             // ---------------------------------------
             const std::string & GetOtherNymID()     const; // Could be sender OR recipient depending on whether incoming/outgoing.
             const std::string & GetOtherAccountID() const; // Could be sender OR recipient depending on whether incoming/outgoing.
             // ---------------------------------------
             const std::string & GetName()           const;
             const std::string & GetDate()           const;
             const std::string & GetAmount()         const;
             const std::string & GetInstrumentType() const;
             const std::string & GetMemo()           const;
             const std::string & GetContents()       const;
             
             bool   HasInitialPayment();
             bool   HasPaymentPlan();
             
             time_t GetInitialPaymentDate();
             time_t GetPaymentPlanStartDate();
             time_t GetTimeBetweenPayments();
             
             long   GetInitialPaymentAmount();
             long   GetPaymentPlanAmount();
             
             int    GetMaximumNoPayments();
             // ---------------------------------------
             bool  FormatAmount              (std::string & str_output);
             bool  FormatDescription         (std::string & str_output);
             bool  FormatShortMailDescription(std::string & str_output);
             // ---------------------------------------
             
             */
            
            record_map.insert("isoutgoing", recordmt.IsOutgoing());
            record_map.insert("ispending", recordmt.IsPending());
            record_map.insert("isreceipt", recordmt.IsReceipt());
            record_map.insert("isrecord", recordmt.IsRecord());
            record_map.insert("ismail", recordmt.IsMail());
            record_map.insert("accountId", QString::fromStdString(recordmt.GetAccountID()));
            record_map.insert("amount", QString::fromStdString(recordmt.GetAmount()));
            
            std::string str_formatted;
            QString qstrAmount = QString("");
            if (recordmt.FormatAmount(str_formatted))
                qstrAmount = QString(QString::fromStdString(str_formatted));
            record_map.insert("formatAmount", qstrAmount);
            
            if (recordmt.IsMail())
            {
                std::string str_mail_desc;
                QString qstrDesc = QString("");
                if (recordmt.FormatShortMailDescription(str_mail_desc))
                    qstrDesc = QString(QString::fromStdString(str_mail_desc));
                record_map.insert("shortMail", qstrDesc);
            }
            
            record_map.insert("assetId", QString::fromStdString(recordmt.GetAssetID()));
            record_map.insert("currencyTLA", QString::fromStdString(recordmt.GetCurrencyTLA()));
            record_map.insert("date", QString::fromStdString(recordmt.GetDate()));
            record_map.insert("instrumentType", QString::fromStdString(recordmt.GetInstrumentType()));
            record_map.insert("name", QString::fromStdString(recordmt.GetName()));
            record_map.insert("nymId", QString::fromStdString(recordmt.GetNymID()));
            record_map.insert("recordType", recordmt.GetRecordType());
            record_map.insert("serverId", QString::fromStdString(recordmt.GetServerID()));
            
            //Special retrieval
            //Format Description
            std::string formatDescription_holder;
            recordmt.FormatDescription(formatDescription_holder);
            record_map.insert("formatDescription", QString::fromStdString(formatDescription_holder));
            
            
            
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

