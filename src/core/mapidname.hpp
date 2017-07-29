#ifndef MAPIDNAME_HPP
#define MAPIDNAME_HPP

#include <QMap>

typedef QMap<QString, QString> mapIDName; // ID, display name.

typedef QMap<QString, mapIDName> mapOfMapIDName; // TLA or share symbol, map of IDs/display names.

#endif // MAPIDNAME_HPP

