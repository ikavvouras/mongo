//
// Created by yannis on 3/12/2016.
//

#include <map>
#include <string>
#include <vector>

#include "mongo/db/migration/Registry.h"

namespace mongo {

    struct MongoServerCredentials {
        std::string username;
        std::string password;
        std::string host;
        int port;
        std::vector<string> dbs;
    };

    class Migrator {
    private:
        Registry *registry = NULL;

        void enableRegistry();

        void flushRegistry();

        void migrateData(MongoServerCredentials credentials);
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

        void start(MongoServerCredentials mongoServerCredentials);

        void stop();
    };

}
