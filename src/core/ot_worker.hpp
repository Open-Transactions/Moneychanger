#ifndef OT_WORKER_HPP
#define OT_WORKER_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <opentxs/OTRecordList.hpp>

#include <QObject>
#include <QMap>
#include <QMutex>
#include <QList>

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
    OTRecordList list;
    QList< QMap<QString,QVariant> > * overview_list;
    
signals:
    
    public slots:
    
};

#endif // OT_WORKER_HPP
