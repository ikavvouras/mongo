//
// Created by yannis on 30/4/2017.
//

#ifndef MONGO_MIGRATORLOCK_H
#define MONGO_MIGRATORLOCK_H

/**
 * multiple users, single admin lock, with priority in the admin lock
 */
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

#include <set>
#include <condition_variable> // std::condition_variable
#include "mongo/db/migration/Registry.h"
#include "mongo/db/operation_context.h"

#endif //MONGO_MIGRATORLOCK_H
