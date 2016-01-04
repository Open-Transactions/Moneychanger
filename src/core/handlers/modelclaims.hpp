#ifndef MODELCLAIMS_H
#define MODELCLAIMS_H

#include <core/handlers/contacthandler.hpp>

#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QSortFilterProxyModel>
#include <QModelIndex>
#include <QVariant>

#include <memory>
#include <string>

#define CLAIM_SOURCE_COL_CLAIM_ID 0
#define CLAIM_SOURCE_COL_NYM_ID 1
#define CLAIM_SOURCE_COL_SECTION 2
#define CLAIM_SOURCE_COL_TYPE 3
#define CLAIM_SOURCE_COL_VALUE 4
#define CLAIM_SOURCE_COL_START 5
#define CLAIM_SOURCE_COL_END 6
#define CLAIM_SOURCE_COL_ATTRIBUTES 7
#define CLAIM_SOURCE_COL_ATT_ACTIVE 8
#define CLAIM_SOURCE_COL_ATT_PRIMARY 9


class ModelClaims : public QSqlQueryModel
{
    Q_OBJECT
public:
    explicit ModelClaims(QObject *parent = 0);

    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QVariant rawData ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

signals:

public slots:
};


class ClaimsProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    ClaimsProxyModel(QObject *parent = 0);

    QVariant data    ( const QModelIndex & index, int role = Qt::DisplayRole ) const override;
    QVariant rawData ( const QModelIndex & index, int role = Qt::DisplayRole ) const;

protected:
    bool filterAcceptsColumn(int source_column, const QModelIndex & source_parent) const Q_DECL_OVERRIDE;
};


#endif // MODELCLAIMS_H


