//
// Created by yannis on 3/12/2016.
//

#include "mongo/db/migration/Registry.h"

class Migrator {
    static Registry* registry = NULL;
public:

    static bool isRegistryEnabled() {
        return registry != NULL;
    }


};
