#define MONGO_LOG_DEFAULT_COMPONENT ::mongo::logger::LogComponent::kCommand

#include "mongo/platform/basic.h"
#include "mongo/util/log.h"

#include "mongo/db/migration/migrate_cmd.h"

namespace mongo {

    using std::string;

    static const string MIGRATE_COMMAND = "migrate";
    static const string START_ACTION = "start";


    void MigrateCmd::addRequiredPrivileges(const std::string &dbname,
                                           const BSONObj &cmdObj,
                                           std::vector<Privilege> *out) {
        ActionSet actions;
        actions.addAction(ActionType::migrate);
        out->push_back(Privilege(ResourcePattern::forClusterResource(), actions));
    }

    bool MigrateCmd::run(OperationContext *txn,
                         const std::string &dbname,
                         BSONObj &cmdObj,
                         int options,
                         std::string &errmsg,
                         BSONObjBuilder &result) {

        const string &action = cmdObj.getField(MIGRATE_COMMAND).str();

        log() << "migration action: " << action;

        if (action == START_ACTION || action == "1.0") {
            Migrator *migrator = Migrator::getInstance();

            migrator->start(extractTargetMongoServerCredentials(cmdObj), extractHostMongoServerCredentials(cmdObj), txn);

        } else if (action == "status") {
            log() << "status " << Migrator::getInstance()->isRegistryEnabled();
            result.append("status", Migrator::getInstance()->isRegistryEnabled());

        } else if (action == "migrateData") {
            // manual testing
            log() << "enabled registry: " << Migrator::getInstance()->isRegistryEnabled();
            Migrator::getInstance()->migrateData(extractTargetMongoServerCredentials(cmdObj),
                                                 extractHostMongoServerCredentials(cmdObj), txn);
        } else if (action == "enableRequestForwarding") {
            // manual testing
            Migrator::getInstance()->enableRequestForwarding();
        } else if (action == "flushRegistry") {
            // manual testing
            Migrator::getInstance()->flushRegistry();
        } else {
            return false;
        }

        return true;
    }

    MongoServerCredentials MigrateCmd::extractTargetMongoServerCredentials(BSONObj &obj) {

        const BSONObj &target = obj.getObjectField("target");

        return convertToMongoServerCredentials(target);

    }

    MongoServerCredentials MigrateCmd::extractHostMongoServerCredentials(BSONObj &obj) {
        const BSONObj &target = obj.getObjectField("host");

        return convertToMongoServerCredentials(target);
    }

    // TODO extract to converter class
    MongoServerCredentials MigrateCmd::convertToMongoServerCredentials(const BSONObj &target) const {
        MongoServerCredentials credentials;
        credentials.host = target.getField("host").str();

        if (target.hasField("port")) {
            credentials.port = target.getField("port").numberInt();
        }

        if (target.hasField("username")) {
            credentials.username = target.getField("username").str();
        }
        if (target.hasField("password")) {
            credentials.password = target.getField("password").str();
        }

        if (target.hasField("dbs")) {
            const std::vector<BSONElement> &dbsArray = target.getField("dbs").Array();
            credentials.dbs.resize(dbsArray.size());
            transform(dbsArray.begin(), dbsArray.end(),
                      credentials.dbs.begin(),
                      [](BSONElement e) {
                          return e.toString(false);
                      });
        }

        return credentials;
    }


}  // namespace mongo
