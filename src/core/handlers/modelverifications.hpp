#ifndef MODELVERIFICATIONS_H
#define MODELVERIFICATIONS_H

#include <core/handlers/contacthandler.hpp>

#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QSortFilterProxyModel>
#include <QModelIndex>
#include <QVariant>

#include <memory>
#include <string>

#define VERIFY_SOURCE_COL_VERIFICATION_ID 0
#define VERIFY_SOURCE_COL_CLAIMANT_NYM_ID 1
#define VERIFY_SOURCE_COL_VERIFIER_NYM_ID 2
#define VERIFY_SOURCE_COL_CLAIM_ID 3
#define VERIFY_SOURCE_COL_POLARITY 4
#define VERIFY_SOURCE_COL_START 5
#define VERIFY_SOURCE_COL_END 6
#define VERIFY_SOURCE_COL_SIGNATURE 7
#define VERIFY_SOURCE_COL_SIG_VERIFIED 8


class ModelVerifications : public QSqlQueryModel
{
    Q_OBJECT
public:
    explicit ModelVerifications(QObject *parent = 0);

    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QVariant rawData ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

signals:

public slots:
};


class VerificationsProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    VerificationsProxyModel(QObject *parent = 0);

    QVariant data    ( const QModelIndex & index, int role = Qt::DisplayRole ) const override;
    QVariant rawData ( const QModelIndex & index, int role = Qt::DisplayRole ) const;

protected:
    bool filterAcceptsColumn(int source_column, const QModelIndex & source_parent) const Q_DECL_OVERRIDE;
};

#endif // MODELVERIFICATIONS_H

