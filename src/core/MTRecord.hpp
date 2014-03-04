//
//  MTRecord.h
//  OTClient module
//
//  Created by Chris Odom on 4/28/13.
//  Copyright (c) 2013 Monetas. All rights reserved.
//


#ifndef __OTClient__MTRecord__
#define __OTClient__MTRecord__

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <iostream>
#include <string>

const std::string & MTRecord_GetTypeString(int theType);


/*
 Could you add this to MTRecord:
 
 - Sender Nym, Account Ids
 - Accept & reject actions
 - Delete action
 
 */

class MTRecord
{
public:
    enum MTRecordType
    {
        Mail = 0,
        Transfer,
        Receipt,
        Instrument,
        Error
    };
private:
    int     m_nBoxIndex;
    time_t  m_ValidFrom;
    time_t  m_ValidTo;
    // ---------------------------------------
    const std::string & m_str_server_id;
    const std::string & m_str_asset_id;
    const std::string & m_str_currency_tla;
    const std::string & m_str_nym_id;
    const std::string & m_str_account_id;
    // ---------------------------------------
    std::string   m_str_other_nym_id;
    std::string   m_str_other_account_id;
    // ---------------------------------------
    std::string   m_str_name;
    std::string   m_str_date;
    std::string   m_str_amount;
    std::string   m_str_type;
    std::string   m_str_memo;
    // ---------------------------------------
    // Contains payment instrument or mail message (or nothing, if not applicable.)
    //
    std::string   m_str_contents;
    // ---------------------------------------
    // Contains transaction number of actual receipt in inbox,
    // or payment inbox, or record box. (If outpayment, contains
    // transaction number on outgoing instrument.)
    //
    long          m_lTransactionNum;
    long          m_lTransNumForDisplay;
    // ---------------------------------------
    bool          m_bIsPending;
    bool          m_bIsOutgoing;
    bool          m_bIsRecord;  // record box (closed, finished, historical only.)
    bool          m_bIsReceipt; // It's a receipt, not a payment.
    // ---------------------------------------
    bool          m_bIsPaymentPlan;
    bool          m_bIsSmartContract;
    bool          m_bIsVoucher;
    bool          m_bIsCheque;
    bool          m_bIsInvoice;
    bool          m_bIsCash;
    // ---------------------------------------
    bool          m_bIsExpired;
    bool          m_bIsCanceled;
    // ---------------------------------------
    MTRecordType        m_RecordType;
    // ---------------------------------------
    bool  AcceptIncomingTransferOrReceipt();
public:
    // ---------------------------------------
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
    // ---------------------------------------
    void  SetExpired();
    void  SetCanceled();
    // ---------------------------------------
    time_t GetValidFrom();
    time_t GetValidTo();
    // ---------------------------------------
    void  SetDateRange(time_t tValidFrom, time_t tValidTo);
    // ---------------------------------------
    bool  CanDeleteRecord()        const;  // For completed records (not pending.)
    bool  CanAcceptIncoming()      const;  // For incoming, pending (not-yet-accepted) instruments.
    bool  CanDiscardIncoming()     const;  // For INcoming, pending (not-yet-accepted) instruments.
    bool  CanCancelOutgoing()      const;  // For OUTgoing, pending (not-yet-accepted) instruments.
    bool  CanDiscardOutgoingCash() const;  // For OUTgoing cash. (No way to see if it's been accepted, so this lets you erase the record of sending it.)
    // ---------------------------------------
    bool  CancelOutgoing(const std::string str_via_acct); // For outgoing, pending (not-yet-accepted) instruments. NOTE: str_via_acct can be blank if a cheque. But if voucher, smart contract, payment plan, you must provide.
    bool  AcceptIncomingInstrument(const std::string & str_into_acct);   // For incoming, pending (not-yet-accepted) instruments.
    bool  AcceptIncomingTransfer();     // For incoming, pending (not-yet-accepted) transfers.
    bool  AcceptIncomingReceipt();      // For incoming, (not-yet-accepted) receipts.
    bool  DiscardIncoming();            // For incoming, pending (not-yet-accepted) instruments.
    bool  DeleteRecord();               // For completed records (not pending.)
    bool  DiscardOutgoingCash();        // For OUTgoing cash. (No way to see if it's been accepted, so this lets you erase the record of sending it.)
    // ---------------------------------------
    int   GetBoxIndex() const; // If this is set to 3, for example, for a payment in the payments inbox, then index 3 in that same box refers to the payment corresponding to this record.
    void  SetBoxIndex(int nBoxIndex);
    // ---------------------------------------
    long  GetTransactionNum() const; // Trans Num of receipt in the box. (Unless outpayment, contains number for instrument.)
    void  SetTransactionNum(long lTransNum);
    // ---------------------------------------
    long  GetTransNumForDisplay() const; // Trans Num of the cheque inside the receipt in the box.
    void  SetTransNumForDisplay(long lTransNum);
    // ---------------------------------------
    MTRecordType  GetRecordType() const;
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
    // ---------------------------------------
    void  SetOtherNymID    (const std::string & str_ID);
    void  SetOtherAccountID(const std::string & str_ID);
    void  SetMemo          (const std::string & str_memo);
    void  SetContents      (const std::string & str_contents);
    // ---------------------------------------
    // These don't work unless the record is for a pending
    // payment plan.
    //
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
    bool  FormatMailSubject         (std::string & str_output);
    // ---------------------------------------
    bool operator<(const MTRecord& rhs);
    // ---------------------------------------
    MTRecord(const std::string & str_server_id,
             const std::string & str_asset_id,
             const std::string & str_currency_tla,
             const std::string & str_nym_id,
             const std::string & str_account_id,
             // -------------------------------
             const std::string & str_name,
             const std::string & str_date,
             const std::string & str_amount,
             const std::string & str_type,
             bool  bIsPending,
             bool  bIsOutgoing,
             bool  bIsRecord,
             bool  bIsReceipt,
             MTRecordType eRecordType
             );
};



#endif /* defined(__OTClient__MTRecord__) */
