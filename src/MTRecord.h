//
//  MTRecord.h
//  OTClient module
//
//  Created by Chris Odom on 4/28/13.
//  Copyright (c) 2013 Monetas. All rights reserved.
//


#ifndef __OTClient__MTRecord__
#define __OTClient__MTRecord__

#include <iostream>
#include <string>

const std::string & MTRecord_GetTypeString(int theType);

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
    // ---------------------------------------
    const std::string & m_str_server_id;
    const std::string & m_str_asset_id;
    const std::string & m_str_currency_tla;
    const std::string & m_str_nym_id;
    const std::string & m_str_account_id;
    // ---------------------------------------
          std::string   m_str_name;
          std::string   m_str_date;
          std::string   m_str_amount;
          std::string   m_str_type;
    // ---------------------------------------
          bool          m_bIsPending;
          bool          m_bIsOutgoing;
          bool          m_bIsRecord;  // record box (closed, finished, historical only.)
          bool          m_bIsReceipt; // It's a receipt, not a payment.
    // ---------------------------------------
    MTRecordType        m_RecordType;
public:
    // ---------------------------------------
    bool  IsPending();
    bool  IsOutgoing();
    bool  IsRecord();
    bool  IsReceipt();
    // ---------------------------------------
    MTRecordType  GetRecordType();
    // ---------------------------------------
    const std::string & GetServerID();
    const std::string & GetAssetID();
    const std::string & GetCurrencyTLA(); // BTC, USD, etc.
    const std::string & GetNymID();
    const std::string & GetAccountID();
    // ---------------------------------------
    const std::string & GetName();
    const std::string & GetDate();
    const std::string & GetAmount();
    const std::string & GetInstrumentType();
    // ---------------------------------------
    bool  FormatAmount     (std::string & str_output);
    bool  FormatDescription(std::string & str_output);
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
