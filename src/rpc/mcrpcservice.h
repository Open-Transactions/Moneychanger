#ifndef MCRPCSERVICE_H
#define MCRPCSERVICE_H

#include <qjsonrpcservice.h>

class MCRPCService : public QJsonRpcService
{
private:
    Q_OBJECT
    Q_CLASSINFO("serviceName", "moneychanger")

public:
    MCRPCService(QObject *parent = 0);

public Q_SLOTS:

    // opentxs::OTAPI methods
    QJsonValue getCurrencyTLA(QString InstrumentDefinitionID);
    QJsonValue getCurrencySymbol(QString InstrumentDefinitionID);
    QJsonValue stringToAmountLocale(QString InstrumentDefinitionID, QString Input,
                                    QString ThousandsSep, QString DecimalPoint);
    QJsonValue formatAmountLocale(QString InstrumentDefinitionID, int64_t Amount,
                                  QString ThousandsSep, QString DecimalPoint);
    QJsonValue formatAmountWithoutSymbolLocale(QString InstrumentDefinitionID, int64_t Amount,
                                               QString ThousandsSep, QString DecimalPoint);
    QJsonValue stringToAmount(QString InstrumentDefinitionID,
                              QString Input);
    QJsonValue formatAmount(QString InstrumentDefinitionID,
                            int64_t Amount);
    QJsonValue formatAmountWithoutSymbol(QString InstrumentDefinitionID,
                                         int64_t Amount);
    QJsonValue getAssetTypeContract(QString InstrumentDefinitionID);
    QJsonValue addServerContract(QString Contract);
    QJsonValue addAssetContract(QString Contract);
    QJsonValue getNymCount();
    QJsonValue getServerCount();
    QJsonValue getAssetTypeCount();
    QJsonValue getAccountCount();


    // Moneychanger::It() methods
    QString mcSendDialog(QString Account, QString Recipient,
                         QString Asset, QString Amount);
    QString mcRequestFundsDialog(QString Account, QString Recipient,
                                 QString Asset, QString Amount);

    bool mcActivateSmartContract(); // Init Wizard
    QJsonValue mcListSmartContracts();


};

#endif // MCRPCSERVICE_H
