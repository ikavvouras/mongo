//
// Created by yannis on 3/12/2016.
//

#include <map>
#include <set>
#include <string>

#include "mongo/bson/bsonobj.h"

namespace mongo {

    using std::string;

    class Registry {
        std::map<string, BSONObj *> inserted;
        std::map<string, BSONObj *> updated;
        std::set<string> removed;
    public:
        BSONObj *read(string id);

        BSONObj *write(string id, BSONObj *object);

        BSONObj *update(string id, BSONObj *object);

        void remove(string id);

        bool hasUpdated(const string &id) const;

        const std::map<string, BSONObj *> &getInserted() const;

        const std::map<string, BSONObj *> &getUpdated() const;

        const std::set<string> &getRemoved() const;
    };

}