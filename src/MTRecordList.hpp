//
//  MTRecordList.h
//  OTClient module
//
//  Created by Chris Odom on 4/28/13.
//  Copyright (c) 2013 Monetas. All rights reserved.
//

#ifndef __OTClient__MTRecordList__
#define __OTClient__MTRecordList__

#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <string>

#ifdef _WIN32
#include <memory>
#elif __GXX_EXPERIMENTAL_CXX0X__ || __cplusplus >= 201103L
#include <memory>
#else
#include <tr1/memory>
#endif


#include "MTRecord.hpp"


class MTNameLookup
{
public:
    virtual ~MTNameLookup() {}

    virtual std::string GetNymName(const std::string & str_id,
                                   const std::string * p_server_id=NULL) const;

    virtual std::string GetAcctName(const std::string & str_id,
                                    const std::string * p_nym_id=NULL,
                                    const std::string * p_server_id=NULL,
                                    const std::string * p_asset_id=NULL) const;
};

/*
 class MTNameLookupIPhone : public MTNameLookup
 {
 public:
 virtual std::string GetNymName(const std::string & str_id) const;
 virtual std::string GetAcctName(const std::string & str_id) const;
 };
 */

#if __GXX_EXPERIMENTAL_CXX0X__ || __cplusplus >= 201103L
// -------------------------------------------------------------
typedef std::weak_ptr  <MTRecord>       weak_ptr_MTRecord;
typedef std::shared_ptr<MTRecord>     shared_ptr_MTRecord;
// -------------------------------------------------------------
#else
// -------------------------------------------------------------
typedef std::tr1::weak_ptr  <MTRecord>       weak_ptr_MTRecord;
typedef std::tr1::shared_ptr<MTRecord>     shared_ptr_MTRecord;
// -------------------------------------------------------------
#endif

typedef std::vector<shared_ptr_MTRecord>      vec_MTRecordList;
// -------------------------------------------------------------
typedef std::list<std::string>                 list_of_strings;
// -------------------------------------------------------------
typedef std::map<std::string, std::string>     map_of_strings;


class MTRecordList
{
    MTNameLookup           *  m_pLookup;
    // ------------------------------------------------
    // Defaults to false. If you set it true, it will run a lot faster. (And give you less data.)
    bool                      m_bRunFast;
    // ------------------------------------------------
    bool                      m_bAutoAcceptCheques;   // Cheques and vouchers, NOT invoices.
    bool                      m_bAutoAcceptReceipts;
    bool                      m_bAutoAcceptTransfers;
    bool                      m_bAutoAcceptCash;
    // ------------------------------------------------
    list_of_strings           m_servers;
     map_of_strings           m_assets;  // <asset_type_id, asset_name>
    list_of_strings           m_accounts;
    list_of_strings           m_nyms;
    // ------------------------------------------------
    vec_MTRecordList          m_contents;
    // ------------------------------------------------
    static const std::string  s_blank;
    static const std::string  s_message_type;
public:
    MTRecordList(MTNameLookup & theLookup);
    ~MTRecordList();
    // ------------------------------------------------
    void SetFastMode() { m_bRunFast = true; }
    // ------------------------------------------------
    // SETUP:
    
    void SetServerID(const std::string str_id); // Set the default server here.
    void AddServerID(const std::string str_id); // Unless you have many servers, then use this.
    void ClearServers(); // Also clears m_contents
    
    void SetAssetID(const std::string str_id);  // Etc.
    void AddAssetID(const std::string str_id);
    void ClearAssets(); // Also clears m_contents
    
    void SetNymID(const std::string str_id);
    void AddNymID(const std::string str_id);
    void ClearNyms(); // Also clears m_contents
    
    void SetAccountID(const std::string str_id);
    void AddAccountID(const std::string str_id);
    void ClearAccounts(); // Also clears m_contents
    
    // ------------------------------------------------
    void AcceptChequesAutomatically  (bool bVal=true);
    void AcceptReceiptsAutomatically (bool bVal=true);
    void AcceptTransfersAutomatically(bool bVal=true);
    void AcceptCashAutomatically     (bool bVal=true);
    
    bool DoesAcceptChequesAutomatically  ();
    bool DoesAcceptReceiptsAutomatically ();
    bool DoesAcceptTransfersAutomatically();
    bool DoesAcceptCashAutomatically     ();
    
    bool PerformAutoAccept(); // Before populating, process out any items we're supposed to accept automatically.
    // ------------------------------------------------
    // POPULATE:
    
    bool Populate();      // Populates m_contents from OT API. Calls ClearContents().
    void ClearContents(); // Clears m_contents (NOT nyms, accounts, servers, or asset types.)
    // ------------------------------------------------
    // RETRIEVE:
    //
    int               size();
    weak_ptr_MTRecord GetRecord(int nIndex);
    bool              RemoveRecord(int nIndex);
    // ------------------------------------------------
};

/*
 
 // TO USE:
 
 MTRecordList blah;
 
 blah.SetServerID("id goes here");
 blah.SetAssetID("id goes here");
 blah.SetNymID("id goes here");
 blah.SetAccountID("id goes here");
 
 blah.Populate();
 
 // THEN:
 
 int               nSize  = blah.size();
 int               nIndex = [0 .. nSize-1]
 weak_ptr_MTRecord record = blah.GetRecord(nIndex);
 
 
 // ACCESSING THE RECORD:
 
 shared_ptr_MTRecord pRecord(record);
 
 if (!sp)
 {
 // It's NULL -- this means MTRecordList got re-populated.
 // (Which means the list control on the UI needs to get
 // re-populated with fresh pointers.)
 }
 else // Pointer is good
 {
 pRecord->GetName();  // Etc.
 }
 
 
 // THEN LATER:
 
 blah.Populate();
 
 Etc.
 
 */

#endif /* defined(__OTClient__MTRecordList__) */












