#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <core/moneychanger.hpp>
#include <core/handlers/DBHandler.hpp>
#include <core/handlers/contacthandler.hpp>
#include <gui/widgets/homedetail.hpp>

#include <core/handlers/modelclaims.hpp>

#include <opentxs/opentxs.hpp>

#include <QDebug>
#include <QtGlobal>
#include <QDateTime>
#include <Qt>
#include <QLabel>
#include <QHBoxLayout>


// ------------------------------------------------------------

ClaimsProxyModel::ClaimsProxyModel(QObject *parent /*=0*/)
: QSortFilterProxyModel(parent)
{
}


QVariant ClaimsProxyModel::data ( const QModelIndex & index, int role/* = Qt::DisplayRole */) const
{
//  if (role == Qt::TextAlignmentRole)
//      return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
    // ----------------------------------------
    if ( role==Qt::DisplayRole && index.isValid() &&
        QSortFilterProxyModel::data(index,role).isValid())
    {
        const int nSourceRow    = headerData(index.row(),    Qt::Vertical,   Qt::UserRole).toInt();
        const int nSourceColumn = headerData(index.column(), Qt::Horizontal, Qt::UserRole).toInt();

        QModelIndex sourceIndex = sourceModel()->index(nSourceRow, nSourceColumn);
        QVariant    sourceData  = sourceModel()->data(sourceIndex, role);

//      if (nSourceColumn == CLAIM_SOURCE_COL_CLAIM_ID) // claim_id (a string)
//      {
//          if (nullptr != pTableView_)
//          {
//              if (nullptr == pTableView_->indexWidget(index))
//              {
//                  QWidget * pWidget = CreateDetailHeaderWidget(nSourceRow);
//                  pTableView_->setIndexWidget(index, pWidget);
//              }
//          }
//          return QVariant();
//      }
//      else
        if (nSourceColumn == CLAIM_SOURCE_COL_NYM_ID) // claim_nym_id
        {
            QString qstrID = sourceData.isValid() ? sourceData.toString() : "";

            std::string str_name;

            if (!qstrID.isEmpty())
            {
                const std::string str_id = qstrID.trimmed().toStdString();
                str_name = str_id.empty() ? "" : opentxs::OT::App().API().Exec().GetNym_Name(str_id);
            }
            // ------------------------
            if (str_name.empty() && !qstrID.isEmpty())
                return QVariant(qstrID);
            return QVariant(QString::fromStdString(str_name));
        }
        else if ((nSourceColumn == CLAIM_SOURCE_COL_START) || (nSourceColumn == CLAIM_SOURCE_COL_END)) // timestamp
        {
            time64_t the_time = sourceData.isValid() ? sourceData.toLongLong() : 0;
            QDateTime timestamp;
            timestamp.setTime_t(the_time);
            return QVariant(QString(timestamp.toString(Qt::SystemLocaleShortDate)));
        }
    }
    // -------------------------------
    else if (role==Qt::CheckStateRole && index.isValid())
    {
        const int nSourceRow           = headerData(index.row(),    Qt::Vertical,   Qt::UserRole).toInt();
        const int nSourceColumn        = headerData(index.column(), Qt::Horizontal, Qt::UserRole).toInt();

        const int nSourceColumnActive  = CLAIM_SOURCE_COL_ATT_ACTIVE;
        const int nSourceColumnPrimary = CLAIM_SOURCE_COL_ATT_PRIMARY;

        QModelIndex sourceIndexActive  = sourceModel()->index(nSourceRow, nSourceColumnActive);
        QModelIndex sourceIndexPrimary = sourceModel()->index(nSourceRow, nSourceColumnPrimary);

        QVariant    sourceDataActive   = sourceModel()->data(sourceIndexActive,  Qt::DisplayRole);
        QVariant    sourceDataPrimary  = sourceModel()->data(sourceIndexPrimary, Qt::DisplayRole);

        if (nSourceColumn == CLAIM_SOURCE_COL_ATT_ACTIVE)
        {
            const bool bIsTrue = sourceDataActive.isValid() ? sourceDataActive.toBool() : false;

            if (bIsTrue)
                return Qt::Checked;
            else
                return Qt::Unchecked;
        }
        else if (nSourceColumn == CLAIM_SOURCE_COL_ATT_PRIMARY)
        {
            const bool bIsTrue = sourceDataPrimary.isValid() ? sourceDataPrimary.toBool() : false;

            if (bIsTrue)
                return Qt::Checked;
            else
                return Qt::Unchecked;
        }
    }
    // --------------------------------------------
    return QSortFilterProxyModel::data(index,role);
}

// --------------------------------------------

bool ClaimsProxyModel::filterAcceptsColumn(int source_column, const QModelIndex & source_parent) const
{
    bool bReturn = true;

    switch (source_column)
    {
    case CLAIM_SOURCE_COL_CLAIM_ID:   bReturn = true;  break; // claim_id
    case CLAIM_SOURCE_COL_NYM_ID:     bReturn = true;  break; // claim_nym_id
    default:                          bReturn = true;  break;
    }
    return bReturn;
}

// --------------------------------------------

QVariant ClaimsProxyModel::rawData ( const QModelIndex & index, int role/* = Qt::DisplayRole */) const
{
    return QSortFilterProxyModel::data(index,role);
}

// ------------------------------------------------------------

ModelClaims::ModelClaims(QObject * parent /*= 0*/)
: QSqlQueryModel(parent) {}


// I'm overriding this so I can return the ACTUAL row or column back (depending on orientation) from the source
// model. This way, the proxy model can call this to find out the actual column or row, whenever it needs to.
//
QVariant ModelClaims::headerData(int section, Qt::Orientation orientation, int role /*=Qt::DisplayRole*/) const
{
    if (role==Qt::UserRole)
    {
        return section;
    }

    return QSqlQueryModel::headerData(section, orientation, role);
}


QVariant ModelClaims::rawData ( const QModelIndex & index, int role/* = Qt::DisplayRole */) const
{
    return QSqlQueryModel::data(index,role);
}

//#define CLAIM_SOURCE_COL_CLAIM_ID 0
//#define CLAIM_SOURCE_COL_NYM_ID 1
//#define CLAIM_SOURCE_COL_SECTION 2
//#define CLAIM_SOURCE_COL_TYPE 3
//#define CLAIM_SOURCE_COL_VALUE 4
//#define CLAIM_SOURCE_COL_START 5
//#define CLAIM_SOURCE_COL_END 6
//#define CLAIM_SOURCE_COL_ATTRIBUTES 7
//#define CLAIM_SOURCE_COL_ATT_ACTIVE 8
//#define CLAIM_SOURCE_COL_ATT_PRIMARY 9

QVariant ModelClaims::data ( const QModelIndex & index, int role/* = Qt::DisplayRole */) const
{
    if (role == Qt::TextAlignmentRole) // I don't want to center the memo column.
    {
//        if (index.column() == PMNT_SOURCE_COL_MEMO)
//            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
    }
    // ----------------------------------------
//    if (role==Qt::DisplayRole && QSqlQueryModel::data(index,role).isValid())
//    {
//        if (index.column() == CLAIM_SOURCE_COL_VALUE) // 'claim_value' is only encoded field. UPDATE: NOT ANY MORE!!
//        {
//            QVariant qvarData    = QSqlQueryModel::data(index,role);
//            QString  qstrData    = qvarData.isValid() ? qvarData.toString() : "";
//            QString  qstrDecoded = qstrData.isEmpty() ? "" : MTContactHandler::Decode(qstrData);
//            return QVariant(qstrDecoded);
//        }
//    }

    return QSqlQueryModel::data(index,role);
}
