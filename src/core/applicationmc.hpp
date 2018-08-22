#ifndef APPLICATIONMC_HPP
#define APPLICATIONMC_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <QApplication>
#include <QScopedPointer>

#include <core/moneychanger.hpp>
#include <opentxs/opentxs.hpp>


class MTApplicationMC : public QApplication
{
    Q_OBJECT

public:
    MTApplicationMC(
            int &argc,
            char **argv,
            QScopedPointer<Moneychanger> & pMoneychanger,
            const opentxs::api::client::Manager& manager);
    virtual ~MTApplicationMC();

public slots:
    void appStarting();
};

#endif // APPLICATIONMC_HPP


