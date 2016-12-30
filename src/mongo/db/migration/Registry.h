//
// Created by yannis on 3/12/2016.
//

#include <map>
#include <set>
#include <string>

class Registry {
    std::map<std::string, void*> inserted;
    std::map<std::string, void*> updated;
    std::set<std::string> removed;
public:
    void* read(std::string id);

    void* write(std::string id, void* object);

    void* update(std::string id, void* object);

    void remove(std::string id);

    bool hasUpdated(const std::string &id) const;
};
