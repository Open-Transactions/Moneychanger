#ifndef MAPIDNAME_HPP
#define MAPIDNAME_HPP

#include <QMap>

#include <string>
#include <set>
#include <map>
#include <memory>
#include <tuple>

typedef QMap<QString, QString>   mapIDName; // ID, display name.
typedef QMap<QString, mapIDName> mapOfMapIDName; // TLA or share symbol, map of IDs/display names.


typedef std::set<std::string> SetOfStrings;
typedef std::shared_ptr<SetOfStrings> PtrSetOfStrings;
typedef std::pair<std::string, std::string> PairOfStrings;
typedef std::map<PairOfStrings, PtrSetOfStrings> MapOfPtrSetsOfStrings;
typedef std::map<std::string, PtrSetOfStrings> MapOfConversationsByNym;

// NymID, (conversational) Thread Id, Thread Item Id.
typedef std::tuple<std::string, std::string, std::string> TupleNymThreadAndItem;
typedef std::shared_ptr<TupleNymThreadAndItem> PtrNymThreadAndItem;
typedef std::set<PtrNymThreadAndItem> SetNymThreadAndItem;

#endif // MAPIDNAME_HPP

