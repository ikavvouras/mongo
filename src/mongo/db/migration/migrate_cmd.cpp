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

        const string &action = cmdObj.getField(MIGRATE_COMMAND).toString(false);

        log() << "migration action: " << action;

        if (action == START_ACTION || action == "1.0") {
            Migrator *migrator = Migrator::getInstance();

            migrator->start(extractMongoServerCredentials(cmdObj));
        } else if (action == "status") {
            log() << "status " << Migrator::getInstance()->isRegistryEnabled();
            result.append("status", Migrator::getInstance()->isRegistryEnabled());
        } else {
            return false;
        }

        return true;
    }

    MongoServerCredentials MigrateCmd::extractMongoServerCredentials(BSONObj &obj) {

        const BSONObj &target = obj.getObjectField("target");

        MongoServerCredentials credentials;
        credentials.host = target.getField("host").toString(false);
        credentials.username = target.getField("username").toString(false);
        credentials.password = target.getField("password").toString(false);
        credentials.port = target.getField("port").numberInt();

        const std::vector<BSONElement> &dbsArray = target.getField("dbs").Array();
        std::transform(dbsArray.begin(), dbsArray.end(),
                       credentials.dbs.begin(),
                       [](BSONElement e) {
                           return e.toString(false);
                       });

        return credentials;
    }


}  // namespace mongo
