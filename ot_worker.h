#ifndef OT_WORKER_H
#define OT_WORKER_H

#include <QDebug>
#include <QObject>
#include <QThread>
#include <QList>
#include <QMap>
#include <QVariant>

#include <opentxs/OTAPI.h>
#include <opentxs/OT_ME.h>
#include <opentxs/OTLog.h>

#include "MTRecordList.h"
#include "MTRecord.h"


class ot_worker : public QObject
{
    Q_OBJECT
public:
    explicit ot_worker(QObject *parent = 0);
    void mc_overview_ping();

private:
    MTRecordList list;
    QList< QMap<QString,QVariant> > * overview_list;

signals:
    
public slots:
    
};

#endif // OT_WORKER_H
