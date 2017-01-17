#define MONGO_LOG_DEFAULT_COMPONENT ::mongo::logger::LogComponent::kCommand

#include "mongo/platform/basic.h"
#include "mongo/util/log.h"

#include "mongo/db/migration/migrate_cmd.h"
#include "mongo/db/migration/Migrator.h"

namespace mongo {

    using std::string;

    void MigrateCmd::addRequiredPrivileges(const std::string& dbname,
                               const BSONObj& cmdObj,
                               std::vector<Privilege>* out) {
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

        const string &action = cmdObj.getField("migrate").toString(false);

        log() << "migration action: " << action;

        if (action == "start" || action == "1.0") {
            Migrator *migrator = Migrator::getInstance();

            migrator->start();
        } else if (action == "status") {
            log() << "status " << Migrator::getInstance()->isRegistryEnabled();
            result.append("status", Migrator::getInstance()->isRegistryEnabled());
        } else {
            return false;
        }

        return true;
    }


}  // namespace mongo
