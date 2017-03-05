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

        MigrationStatus status;

        MongoServerCredentials targetCredentials;


        void enableRegistry();

        void getLocalDatabases(OperationContext *txn, std::vector<string> &dbs);

        void flushDeletedData();

        void flushInsertedData();

        void flushUpdatedData();

    public:
        // TODO make private
        void migrateData(MongoServerCredentials targetCredentials, MongoServerCredentials hostCredentials,
                         OperationContext *txn);

        void enableRequestForwarding();

        void flushRegistry();


        static Migrator *getInstance() {
            static Migrator *migrator = new Migrator();
            return migrator;
        }

        bool isRegistryEnabled();

        bool isFlusingRegistry();

        Registry *getRegistry();

        void start(MongoServerCredentials targetCredentials, MongoServerCredentials hostCredentials,
                   OperationContext *txn);

        void stop();

        MigrationStatus getStatus() const;

        bool hasEnabledForwading();

        void forwardCommand(const rpc::RequestInterface &request, rpc::ReplyBuilderInterface *replyBuilder);

        void flushDeletedData(const std::list<string> &removedDocumentIds);
    };

}
