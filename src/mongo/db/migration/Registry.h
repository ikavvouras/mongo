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
    MutableDocument *createMutableDocument(const BSONObj &bsonObj);

    class Registry {
        std::map<string, BSONObj *> inserted;
        std::map<string, std::vector<BSONObj *>> updated;
        std::set<string> removed;
    public:
        std::vector<BSONObj *> read(const BSONObj &query);

        BSONObj *write(string id, BSONObj *object);

        BSONObj *update(string id, BSONObj *object);

        void remove(string id);

        bool hasUpdated(const string &id) const;

        bool hasRemoved(const string &id) const;

        const std::map<string, BSONObj *> &getInserted() const;

        const std::map<string, std::vector<BSONObj *>> &getUpdated() const;

        const std::set<string> &getRemoved() const;

        BSONObj applyUpdates(const BSONElement &actualElement) const;
    };

}