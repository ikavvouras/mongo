//
// Created by yannis on 3/12/2016.
//
#define MONGO_LOG_DEFAULT_COMPONENT ::mongo::logger::LogComponent::kCommand

#include "mongo/util/log.h"
#include "mongo/db/migration/Migrator.h"

#include "mongo/db/dbdirectclient.h"

#include "mongo/db/server_options.h"
#include "mongo/client/connpool.h"


namespace mongo {

    void Migrator::start(MongoServerCredentials targetCredentials, MongoServerCredentials hostCredentials,
                         OperationContext *txn) {
//        enableRegistry(); TODO uncomment
        isRegistryEnabled(); // TODO remove after uncommenting previous line

        migrateData(targetCredentials, hostCredentials, txn);

        flushRegistry();

        enableRequestForwarding();

        this->status = DONE;
    }

    void Migrator::stop() {
        flushRegistry();// TODO in this case which cancels the migration flush locally
    }

    void Migrator::enableRegistry() {
        this->registry = new mongo::InMemoryRegistry();
        log() << "enabled registry...";
    }

    void Migrator::flushRegistry() {
        log() << "flushing registry...";

        this->status = FLUSHING_REGISTRY;

        flushDeletedData();

        flushInsertedData();

        flushUpdatedData();
    }

    void Migrator::migrateData(MongoServerCredentials targetCredentials,
                               MongoServerCredentials hostCredentials,
                               OperationContext *txn) {
        log() << "migrating data...";
        this->status = MIGRATING_DATA;

        this->targetCredentials = targetCredentials;

        StringData application = "migration";
        HostAndPort hostAndPort(targetCredentials.host, targetCredentials.port);

        DBClientConnection connection;
        mongo::Status connectionStatus = connection.connect(hostAndPort, application);
        log() << "connection status: " << connectionStatus.toString();

        int port = serverGlobalParams.port;

        if (targetCredentials.dbs.empty()) {
            getLocalDatabases(txn, targetCredentials.dbs);
        }

        for (string db : targetCredentials.dbs) {
            string fromDB = db;
            string toDB = db;
            const string hostIp = hostCredentials.host;
            const string host = hostIp + ":" + std::to_string(port);

            log() << "\t" << "copying database '" << db << "'...";

            BSONObj copyCommandResult;
            connection.copyDatabase(fromDB, toDB, host, &copyCommandResult);

            log() << "\t" << "copied database '" << db << "' :" << copyCommandResult.toString();

        }

    }

    void Migrator::getLocalDatabases(OperationContext *txn, std::vector<string> &dbs) {
        DBDirectClient localClient(txn);

        const std::__cxx11::list<string> &databaseNames = localClient.getDatabaseNames();

        dbs.resize(databaseNames.size());
        auto endIt = copy_if(databaseNames.begin(), databaseNames.end(),
                             dbs.begin(),
                             [](string dbName) {
                                 log() << "it->db " << dbName << " :: " << (dbName == "local" || dbName == "admin");
                                 return dbName != "local" && dbName != "admin";
                             });
        dbs.resize((unsigned long) distance(dbs.begin(), endIt));
    }

    void Migrator::enableRequestForwarding() {
        this->status = ENABLED_FORWARDING;
    }

    MigrationStatus Migrator::getStatus() const {
        return status;
    }

    void Migrator::forwardCommand(const rpc::RequestInterface &request, rpc::ReplyBuilderInterface *replyBuilder) {
        log() << "forwarding command : " << request.getCommandArgs();

        StringData application = "migration";
        HostAndPort hostAndPort(targetCredentials.host, targetCredentials.port);
        ScopedDbConnection connection(hostAndPort.toString());

        string dbname = request.getDatabase().toString();
        const BSONObj &cmd = request.getCommandArgs();
        BSONObj responseObj;
        connection.get()->runCommand(dbname, cmd, responseObj);

        replyBuilder->getInPlaceReplyBuilder(0u)
                .appendBuf(responseObj.objdata(), responseObj.objsize());

        replyBuilder->setMetadata(BSONObj());

        connection.done();
    }

    bool Migrator::hasEnabledForwading() {
        return status == ENABLED_FORWARDING;
    }

    Registry *Migrator::getRegistry() {
        return registry;
    }

    bool Migrator::isRegistryEnabled() {
        // TODO remove the following lines
        if (registry == NULL) {
            log() << " ---------------- should be called only once -----------------";
            enableRegistry();
        }

        return status == MIGRATING_DATA || status == FLUSHING_REGISTRY;
    }

    void Migrator::flushDeletedData() {

        log() << "\t" << "flushing deleted data";

        HostAndPort hostAndPort(targetCredentials.host, targetCredentials.port);
        ScopedDbConnection connection(hostAndPort.toString());

        log() << "registry enabled: " << (registry != NULL);
        registry->flushDeletedData(connection);
        connection.done();

        flushStatus.insert(FLUSHED_DELETIONS);
    }

    void Migrator::flushDeletedData(const std::list<string> &removedDocumentIds) {

        HostAndPort hostAndPort(targetCredentials.host, targetCredentials.port);
        ScopedDbConnection connection(hostAndPort.toString());

        const std::list<string> &flushedDeletedData = flushStatusesContains(FLUSHED_DELETIONS)
                                                      ? removedDocumentIds
                                                      : registry->filterFlushed(removedDocumentIds);

        for (string id : flushedDeletedData) {
            registry->flushDeletedRecord(connection, id); // TODO check/inform about flushed
        }

        connection.done();
    }

    void Migrator::flushInsertedData() {
        log() << "\t" << "flushing inserted data";

        HostAndPort hostAndPort(targetCredentials.host, targetCredentials.port);
        ScopedDbConnection connection(hostAndPort.toString());

        for (std::size_t i = 0; i < registry->getInserted().size(); ++i) {
            BSONObj bsonObj = registry->getInserted()[i];

            log() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
            log() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
            log() << "~~~~~~~~~~~~~~~ sleeping for 5\" ~~~~~~~~~~~~~~~~";
            log() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
            log() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
            sleep(5);
            flushInsertedRecord(connection, bsonObj);
        }
        flushStatus.insert(FLUSHED_INSERTIONS);

        connection.done();

    }

    void Migrator::flushInsertedData(const std::vector<BSONElement> &insertedData) {

        if (flushStatusesContains(FLUSHED_INSERTIONS)) {

            HostAndPort hostAndPort(targetCredentials.host, targetCredentials.port);
            ScopedDbConnection connection(hostAndPort.toString());
            for (BSONElement record : insertedData) {
                const BSONObj &obj = record.Obj();

                log() << "flushInsertedBsonObj :: query : " << obj;

                flushInsertedRecord(connection, obj);
            }
            connection.done();

        }

    }

    void Migrator::flushInsertedRecord(ScopedDbConnection &connection, const BSONObj &bsonObj) const {
        log() << "\t\t" << "flushing " << bsonObj.toString();
        connection.get()->insert("test_db.test_collection", bsonObj); // TODO
    }

    void Migrator::flushUpdatedData() {
        log() << "\t" << "flushing updated data";

        HostAndPort hostAndPort(targetCredentials.host, targetCredentials.port);
        ScopedDbConnection connection(hostAndPort.toString());

        registry->flushUpdatedData(connection);
        connection.done();

        flushStatus.insert(FLUSHED_UPDATES);
    }

    void Migrator::flushUpdatedData(const std::map<string, BSONObj *> updated) {
        HostAndPort hostAndPort(targetCredentials.host, targetCredentials.port);
        ScopedDbConnection connection(hostAndPort.toString());

        std::list<string> updatedDocumentIds;
        std::transform(
                updated.begin(), updated.end(),
                std::back_inserter(updatedDocumentIds),
                [](const std::map<string, BSONObj *>::value_type &pair) { return pair.first; });

        const std::list<string> &flushedUpdatedRecordIds = flushStatusesContains(FLUSHED_UPDATES)
                                                           ? updatedDocumentIds
                                                           : registry->filterFlushed(updatedDocumentIds);

        for (string id : flushedUpdatedRecordIds) {
            registry->flushUpdatedRecord(connection, id, updated.find(id)->second); // TODO check/inform about flushed
        }

        connection.done();
    }

    bool Migrator::isFlusingRegistry() {
        return status == FLUSHING_REGISTRY;
    }

    const std::set<MigrationFlushStatus> &Migrator::getFlushStatus() const {
        return flushStatus;
    }

    bool Migrator::flushStatusesContains(const MigrationFlushStatus &x) const {
        return flushStatus.find(x) != flushStatus.end();
    }

}