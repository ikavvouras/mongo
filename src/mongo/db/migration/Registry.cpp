//
// Created by yannis on 3/12/2016.
//

#include "mongo/db/migration/Registry.h"
namespace mongo {

    using std::string;

    void *getOrNull(std::map<string, void *> map, string id) {
        return map.find(id) != map.end() ?
               map.find(id)->second :
               NULL;
    }

    void *Registry::read(string id) {
        return hasUpdated(id) ?
               updated.find(id)->second :
               getOrNull(inserted, id);
    }

    void *Registry::update(string id, void *object) {
        // TODO
        return NULL;
    }

    void *Registry::write(string id, void *object) {
        // TODO
        return NULL;
    }

    void Registry::remove(string id) {
        // TODO
    }

    bool Registry::hasUpdated(const string &id) const { return updated.find(id) != updated.end(); }

    const std::map<string, void *> &Registry::getInserted() const {
        return inserted;
    }

    const std::map<string, void *> &Registry::getUpdated() const {
        return updated;
    }

    const std::set<string> &Registry::getRemoved() const {
        return removed;
    }

}