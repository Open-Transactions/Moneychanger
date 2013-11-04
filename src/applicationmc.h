#ifndef APPLICATIONMC_H
#define APPLICATIONMC_H

#include <QtCore>
#include <QtGui>
#include <QApplication>

class OTCaller;
class OTCallback;
class Moneychanger;

class MTApplicationMC : public QApplication
{
    Q_OBJECT

    Moneychanger * m_pMoneychanger;

public:
    MTApplicationMC(int &argc, char **argv);
    virtual ~MTApplicationMC();

public slots:
    void appStarting();
};

#endif // APPLICATIONMC_H


