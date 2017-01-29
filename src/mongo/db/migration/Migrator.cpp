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

    void Migrator::start(MongoServerCredentials mongoServerCredentials, OperationContext *txn) {
        enableRegistry();

        migrateData(mongoServerCredentials, txn);

        enableRequestForwarding();

        flushRegistry();

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
        this->status = FLUSHING_REGISTRY;

        // TODO
    }

    void Migrator::migrateData(MongoServerCredentials credentials, OperationContext *txn) {
        log() << "migrating data...";
        this->status = MIGRATING_DATA;

        this->credentials = credentials;

        StringData application = "migration";
        HostAndPort hostAndPort(credentials.host, credentials.port);

        DBClientConnection connection;
        mongo::Status connectionStatus = connection.connect(hostAndPort, application);
        log() << "connection status: " << connectionStatus.toString();

        int port = serverGlobalParams.port;

        if (credentials.dbs.empty()) {
            getLocalDatabases(txn, credentials.dbs);
        }

        for (string db : credentials.dbs) {
            string fromDB = db;
            string toDB = db;
            const string hostIp = "172.17.0.1"; // TODO retrieve
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
//        HostAndPort hostAndPort(credentials.host, credentials.port);
        HostAndPort hostAndPort("172.17.0.3", 27017); // TODO

        ScopedDbConnection connection("172.17.0.3:27017"); // TODO

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
        return status != MIGRATING_DATA;
    }

}