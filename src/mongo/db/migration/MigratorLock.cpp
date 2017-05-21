//
// Created by yannis on 30/4/2017.
//

#define MONGO_LOG_DEFAULT_COMPONENT ::mongo::logger::LogComponent::kCommand

#include "mongo/util/log.h"

#include "mongo/db/migration/MigratorLock.h"

namespace mongo {

    void MigratorLock::adminLock() {
        std::unique_lock<std::mutex> lock(mtx);

        admin = true; // priority in admin

        while (readers > 0) {
            cv.wait(lock);
        }

    }

    void MigratorLock::adminUnlock() {
        std::unique_lock<std::mutex> lock(mtx);

        admin = false;

        cv.notify_all();
    }

    void MigratorLock::userLock() {
        std::unique_lock<std::mutex> lock(mtx);
        while (admin) {
            cv.wait(lock);
        }

        ++readers;

    }

    void MigratorLock::userUnlock() {
        std::unique_lock<std::mutex> lock(mtx);

        --readers;

        cv.notify_all();
    }


    ///////////////////////////////////
    void MigratorThroughputLock::requestLock() {
        std::unique_lock<std::mutex> lock(mtx);

        while (!hasFinishedFlushing && (flushedOperations < 1.25 * (double) incomingOperations)) {
            cv.wait(lock);
        }

        ++incomingOperations;
    }

    void MigratorThroughputLock::flushLock() {
        std::unique_lock<std::mutex> lock(mtx);

    }

    void MigratorThroughputLock::flushUnlock() {
        std::unique_lock<std::mutex> lock(mtx);

        ++flushedOperations;
        cv.notify_one();
    }

    void MigratorThroughputLock::flushFinished() {
        std::unique_lock<std::mutex> lock(mtx);

        hasFinishedFlushing = true;

        cv.notify_one();
    }

    void MigratorThroughputLock::requestUnlock() {
        std::unique_lock<std::mutex> lock(mtx);

        cv.notify_one();
    }
}