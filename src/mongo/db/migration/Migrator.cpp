//
// Created by yannis on 3/12/2016.
//

#include "Migrator.h"

namespace mongo {

    void Migrator::start(MongoServerCredentials mongoServerCredentials) {
        enableRegistry();

        migrateData(mongoServerCredentials);
    }

    void Migrator::stop() {
        flushRegistry();
    }

    void Migrator::enableRegistry() {
        this->registry = new mongo::InMemoryRegistry();
    }

    void Migrator::flushRegistry() {
        // TODO
    }

    void Migrator::migrateData(MongoServerCredentials credentials) {

    }

}