//
// Created by yannis on 3/12/2016.
//

#include <map>
#include <set>
#include <vector>
#include <string>

#include "mongo/bson/bsonobj.h"
#include "mongo/db/pipeline/document.h"

namespace mongo {

    using std::string;

    string getId(const BSONElement &actualElement);

    string getId(const BSONObj &actualElement);

    MutableDocument *createMutableDocument(const BSONObj &bsonObj);


    class Registry {
        std::map<string, BSONObj *> inserted;
        std::map<string, std::vector<BSONObj *>> updated;
        std::set<string> removed;


        BSONObj updateFields(const BSONObj &actualObj, const BSONObj *update) const;

        BSONObj replaceFields(const BSONObj &actualObj, const BSONObj *update) const;

    public:
        std::vector<BSONObj *> read(const BSONObj &query);

        BSONObj *insert(BSONObj *object);

        BSONObj *update(string id, BSONObj *object);

        void remove(string id);

        bool hasUpdated(const string &id) const;

        bool hasRemoved(const string &id) const;

        BSONObj applyUpdates(const BSONElement &actualElement) const;

        std::vector<BSONObj> getInserted() const;

    };

}