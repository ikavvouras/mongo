//
// Created by yannis on 3/12/2016.
//

#include <map>
#include <string>
#include <vector>

#include "mongo/db/migration/Registry.h"
#include "mongo/db/operation_context.h"

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

        void migrateData(MongoServerCredentials credentials, OperationContext *txn);

        void getLocalDatabases(OperationContext *txn, std::vector<string> &dbs);

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

        void start(MongoServerCredentials mongoServerCredentials, OperationContext *txn);

        void stop();
    };

}
