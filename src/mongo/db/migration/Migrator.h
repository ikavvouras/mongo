//
// Created by yannis on 3/12/2016.
//

#include <map>
#include <string>
#include <vector>

#include "mongo/db/migration/Registry.h"
#include "mongo/db/operation_context.h"
#include "mongo/rpc/request_interface.h"
#include "mongo/rpc/reply_builder_interface.h"

namespace mongo {

    struct MongoServerCredentials {
        std::string username;
        std::string password;
        std::string host;
        int port;
        std::vector<string> dbs;
    };

    enum MigrationStatus {
        MIGRATING_DATA,
        FLUSHING_REGISTRY,
        ENABLED_FORWARDING,
        DONE
    };

    class Migrator {
    private:
        Registry *registry = NULL;

        void enableRegistry();

        void flushRegistry();

        void migrateData(MongoServerCredentials credentials, OperationContext *txn);

        void getLocalDatabases(OperationContext *txn, std::vector<string> &dbs);

        void enableRequestForwarding();

        MigrationStatus status;

        MongoServerCredentials credentials;

    public:

        static Migrator *getInstance() {
            static Migrator *migrator = new Migrator();
            return migrator;
        }

        bool isRegistryEnabled();

        Registry *getRegistry();

        void start(MongoServerCredentials mongoServerCredentials, OperationContext *txn);

        void stop();

        MigrationStatus getStatus() const;

        bool hasEnabledForwading();

        void forwardCommand(const rpc::RequestInterface &request, rpc::ReplyBuilderInterface *replyBuilder);

        void flushDeletedData();
    };

}
