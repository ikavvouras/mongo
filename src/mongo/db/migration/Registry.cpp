//
// Created by yannis on 3/12/2016.
//

#include "mongo/db/migration/Registry.h"

void* getOrNull(std::map<std::string, void*> map, std::string id) {
    return map.find(id) != map.end() ? 
           map.find(id)->second :
           NULL;
}

void* Registry::read(std::string id) {
    return hasUpdated(id) ?
           updated.find(id)->second :
           getOrNull(inserted, id);
}

void* Registry::update(std::string id, void *object) {
    // TODO
    return NULL;
}

void *Registry::write(std::string id, void *object) {
    // TODO
    return NULL;
}

void Registry::remove(std::string id) {
    // TODO
}

bool Registry::hasUpdated(const std::string &id) const { return updated.find(id) != updated.end(); }