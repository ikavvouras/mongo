//
// Created by yannis on 30/4/2017.
//

#include "mongo/db/migration/Migrator.h"
#include "MigratorLock.h"

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