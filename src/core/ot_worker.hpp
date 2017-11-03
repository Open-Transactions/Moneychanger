#ifndef OT_WORKER_HPP
#define OT_WORKER_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <opentxs/core/Version.hpp>
#include <opentxs/client/OTRecordList.hpp>

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
    opentxs::OTRecordList list;
    QList< QMap<QString,QVariant> > * overview_list;

signals:

    public slots:

};

#endif // OT_WORKER_HPP
