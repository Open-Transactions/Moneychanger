#ifndef OT_WORKER_H
#define OT_WORKER_H

#include <QDebug>
#include <QObject>
#include <QThread>
#include <QList>
#include <QMap>
#include <QVariant>
#include <QMutex>
#include <QMutexLocker>

#include <opentxs/OTAPI.h>
#include <opentxs/OT_ME.h>
#include <opentxs/OTLog.h>

#include "MTRecord.hpp"
#include "MTRecordList.hpp"

class MTNameLookupQT : public MTNameLookup{
public:
    
    // virtual std::string GetNymName(const std::string & str_id) const {}
    
    // virtual std::string GetAcctName(const std::string & str_id) const {}
};


class ot_worker : public QObject
{
    Q_OBJECT
public:
    explicit ot_worker(QObject *parent = 0);
    
    //Overview worker stuff
    void mc_overview_ping();
    QList< QMap<QString, QVariant> > mc_overview_get_currentlist();
    //Overview thread things
    QMutex overview_list_mutex;
    
private:
    MTNameLookupQT lookup;
    MTRecordList list;
    QList< QMap<QString,QVariant> > * overview_list;
    
signals:
    
    public slots:
    
};

#endif // OT_WORKER_H
