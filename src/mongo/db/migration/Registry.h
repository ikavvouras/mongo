//
// Created by yannis on 3/12/2016.
//

#include <map>
#include <set>
#include <string>

namespace mongo {

    using std::string;

    class Registry {
        std::map<string, void *> inserted;
        std::map<string, void *> updated;
        std::set<string> removed;
    public:
        void *read(string id);

        void *write(string id, void *object);

        void *update(string id, void *object);

        void remove(string id);

        bool hasUpdated(const string &id) const;

        const std::map<string, void *> &getInserted() const;

        const std::map<string, void *> &getUpdated() const;

        const std::set<string> &getRemoved() const;
    };

}