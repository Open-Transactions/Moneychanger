#ifndef APPLICATIONMC_HPP
#define APPLICATIONMC_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <QApplication>


class MTApplicationMC : public QApplication
{
    Q_OBJECT

public:
    MTApplicationMC(int &argc, char **argv);
    virtual ~MTApplicationMC();

public slots:
    void appStarting();
};

#endif // APPLICATIONMC_HPP


