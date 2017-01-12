//
// Created by yannis on 3/12/2016.
//

#include "mongo/db/migration/Registry.h"
namespace mongo {

    using std::string;

    BSONObj *getOrNull(std::map<string, BSONObj *> map, string id) {
        return map.find(id) != map.end() ?
               map.find(id)->second :
               NULL;
    }

    BSONObj *Registry::read(string id) {
        return hasUpdated(id) ?
               updated.find(id)->second :
               getOrNull(inserted, id);
    }

    BSONObj *Registry::update(string id, BSONObj *object) {
        // TODO generate a copy of object
        updated.insert(std::make_pair(id, object));

        return object;
    }

    BSONObj *Registry::write(string id, BSONObj *object) {
        throw std::runtime_error("Unimplemented operation exception in registry");
    }

    void Registry::remove(string id) {
        throw std::runtime_error("Unimplemented operation exception in registry");
    }

    bool Registry::hasUpdated(const string &id) const { return updated.find(id) != updated.end(); }

    const std::map<string, BSONObj *> &Registry::getInserted() const {
        return inserted;
    }

    const std::map<string, BSONObj *> &Registry::getUpdated() const {
        return updated;
    }

    const std::set<string> &Registry::getRemoved() const {
        return removed;
    }

}