//
// Created by yannis on 30/4/2017.
//

#ifndef MONGO_MIGRATORLOCK_H
#define MONGO_MIGRATORLOCK_H

#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable

/**
 * multiple users, single admin lock, with priority in the admin lock
 */
namespace mongo {
    class MigratorLock {
    private:
        std::mutex mtx;
        std::condition_variable cv;

        bool admin = false;
        long readers = 0;
    public:

        void adminLock();

        void adminUnlock();

        void userLock();

        void userUnlock();
    };

    class MigratorThroughputLock {
    private:
        std::mutex mtx;
        std::condition_variable cv;

        long flushedOperations = 0;
        long incomingOperations = 0;

        bool hasFinishedFlushing = false;
    public:

        void flushLock();

        void flushUnlock();

        void flushFinished();

        void requestLock();

        void requestUnlock();
    };
}

#endif //MONGO_MIGRATORLOCK_H
