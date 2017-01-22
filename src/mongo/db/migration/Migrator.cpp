//
// Created by yannis on 3/12/2016.
//
#define MONGO_LOG_DEFAULT_COMPONENT ::mongo::logger::LogComponent::kCommand

#include "mongo/util/log.h"
#include "mongo/db/migration/Migrator.h"

#include "mongo/db/dbdirectclient.h"

namespace mongo {

    void Migrator::start(MongoServerCredentials mongoServerCredentials, OperationContext *txn) {
        enableRegistry();

        migrateData(mongoServerCredentials, txn);
    }

    void Migrator::stop() {
        flushRegistry();
    }

    void Migrator::enableRegistry() {
        this->registry = new mongo::InMemoryRegistry();
        log() << "enabled registry...";
    }

    void Migrator::flushRegistry() {
        // TODO
    }

    void Migrator::migrateData(MongoServerCredentials credentials, OperationContext *txn) {
        log() << "migrating data...";

        StringData application = "migration";
        HostAndPort hostAndPort(credentials.host, credentials.port);

        DBClientConnection connection;
        Status connectionStatus = connection.connect(hostAndPort, application);
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

}