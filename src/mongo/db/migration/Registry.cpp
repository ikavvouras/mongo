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
//        return hasUpdated(id) ?
//               updated.find(id)->second :
//               getOrNull(inserted, id);
        throw std::runtime_error("Unimplemented operation exception in registry");
    }

    BSONObj *Registry::update(string id, BSONObj *object) {

        const std::map<string, std::vector<BSONObj *>>::iterator &iterator = updated.find(id);
        if (iterator == updated.end()) {
            std::vector<BSONObj *> *updates = new std::vector<BSONObj *>();
            updates->push_back(object);
            updated.insert(std::make_pair(id, *updates));
        } else {
            std::vector<BSONObj *> &updates = iterator->second;
            updates.push_back(object);
        }

        return object;
    }

    BSONObj *Registry::write(string id, BSONObj *object) {
        throw std::runtime_error("Unimplemented operation exception in registry");
    }

    void Registry::remove(string id) {
        removed.insert(id);
    }

    bool Registry::hasUpdated(const string &id) const {
        return updated.find(id) != updated.end();
    }

    const std::map<string, BSONObj *> &Registry::getInserted() const {
        return inserted;
    }

    const std::map<string, std::vector<BSONObj *>> &Registry::getUpdated() const {
        return updated;
    }

    const std::set<string> &Registry::getRemoved() const {
        return removed;
    }

}