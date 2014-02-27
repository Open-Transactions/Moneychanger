#ifndef APPLICATIONMC_HPP
#define APPLICATIONMC_HPP

#include <WinsockWrapper.h>
#include <ExportWrapper.h>

#include <QtCore>
#include <QtGui>
#include <QApplication>

class OTCaller;
class OTCallback;
class Moneychanger;

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


