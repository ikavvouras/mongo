//
// Created by yannis on 3/12/2016.
//

#include <map>

#include "mongo/db/migration/Registry.h"

namespace mongo {

    class Migrator {
    private:
        Registry *registry = NULL;
    public:

        static Migrator *getInstance() {
            static Migrator *migrator = new Migrator();
            return migrator;
        }

        bool isRegistryEnabled() {
            return registry != NULL;
        }

        Registry *getRegistry() {
            return registry;
        }

        void start() {
            registry = new Registry();
        }
    };

}
