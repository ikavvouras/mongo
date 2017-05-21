//
// Created by yannis on 3/12/2016.
//

#define MONGO_LOG_DEFAULT_COMPONENT ::mongo::logger::LogComponent::kCommand

#include "mongo/util/log.h"
#include "mongo/db/migration/Registry.h"

#include <unistd.h> // TODO remove after sleep(1) removal ~~~~~~~~~~~~~~~~~~~~~~

namespace mongo {

    using std::string;
    using std::vector;

    BSONObj *getOrNull(std::map<string, BSONObj *> map, string id) {
        return map.find(id) != map.end() ?
               map.find(id)->second :
               NULL;
    }

    std::vector<BSONObj *> InMemoryRegistry::read(const BSONObj &query) {
//        return hasUpdated(id) ?
//               updated.find(id)->second :
//               getOrNull(inserted, id);
        throw std::runtime_error("Unimplemented operation exception in registry");
    }

    BSONObj *InMemoryRegistry::update(const string &id, BSONObj *object, string ns) {

        const std::map<string, std::vector<Record *>>::iterator &iterator = updated.find(id);
        if (iterator == updated.end()) {
            std::vector<Record *> *updates = new std::vector<Record *>();
            Record *record = new Record();
            record->ns = ns;
            record->bsonObj = object;
            updates->push_back(record);
            updated.insert(std::make_pair(id, *updates));
        } else {
            std::vector<Record *> &updates = iterator->second;
            Record *record = new Record();
            record->ns = ns;
            record->bsonObj = object;
            updates.push_back(record);
        }

        return object;
    }

    BSONObj *InMemoryRegistry::insert(BSONObj *object, string ns) {
        Record *record = new Record();
        record->bsonObj = object;
        record->ns = ns;
        inserted.insert(std::make_pair(getId(*object), record));

        return object;
    }

    void InMemoryRegistry::remove(string id, string ns) {
        updated.erase(id);
        inserted.erase(id);

        log() << "InMemoryRegistry::remove( " << id << " )";

        Record *record = new Record();
        record->id = id;
        record->ns = ns;

        removed.insert(std::make_pair(id, record));

        log() << "InMemoryRegistry::removed.size() " << removed.size();
    }

    bool InMemoryRegistry::hasUpdated(const string &id) const {
        return updated.find(id) != updated.end();
    }

    bool InMemoryRegistry::hasRemoved(const string &id) const {
        return removed.find(id) != removed.end();
    }

    BSONObj InMemoryRegistry::applyUpdates(const BSONElement &actualElement) const {
        string id = getId(actualElement);

        BSONObj finalObj = actualElement.Obj();

        vector<Record *> sequenceOfUpdates = this->updated.find(id)->second;
        for (Record *update : sequenceOfUpdates) {
            if (update->bsonObj->hasElement("$set")) {
                finalObj = updateFields(finalObj, update->bsonObj);
            } else {
                finalObj = replaceFields(finalObj, update->bsonObj);
            }
        }
        return finalObj;
    }

    BSONObj InMemoryRegistry::replaceFields(const BSONObj &actualObj, const BSONObj *update) const {
        MutableDocument *mutableUpdate = createMutableDocument(*update);
        mutableUpdate->addField("_id", Value(actualObj.getField("_id")));
        const Document &document = mutableUpdate->freeze();
        delete mutableUpdate;
        log() << "replacing with " << document.toString();

        return document.toBson();
    }

    BSONObj InMemoryRegistry::updateFields(const BSONObj &actualObj, const BSONObj *update) const {
        MutableDocument *mutableUpdate = createMutableDocument(actualObj);
        for (BSONElement field : update->getObjectField("$set")) {

            const Value &val = Value(field);
            log() << "InMemoryRegistry::updateFields : updating " << field.toString();

            mutableUpdate->setField(field.fieldNameStringData(), val);
        }

        const Document &document = mutableUpdate->freeze();
        delete mutableUpdate;

        return document.toBson();
    }

    std::vector<BSONObj> InMemoryRegistry::getInserted() const {
        vector<BSONObj> insertedValues;

        for (const std::pair<const string, Record *> &pair : inserted) {
            insertedValues.push_back(*pair.second->bsonObj);
        }

        return insertedValues;
    }

    void InMemoryRegistry::flushDeletedData(mongo::ScopedDbConnection &connection) {
        log() << "flushing " << removed.size() << " deleted records";
        for (const std::pair<const string, Record *> &pair : removed) {

//            log() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
//            log() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
//            log() << "~~~~~~~~~~~~~~~ sleeping for 5\" ~~~~~~~~~~~~~~~~";
//            log() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
//            log() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
//            sleep(5);

            string id = pair.first;

            Record *record = pair.second;

            flushDeletedRecord(connection, id, record->ns);

            record->flushed = true;
        }
    }

    void InMemoryRegistry::flushDeletedRecord(ScopedDbConnection &connection, const string &id, string ns) const {
        Query query = QUERY("_id" << OID(id));
        log() << "flushDeletedData :: query : " << query.toString();
        connection.get()->remove(ns, query); // TODO
    }

    void InMemoryRegistry::flushUpdatedData(UpdateCommand &updateCommand) {
        for (const std::pair<const string, std::vector<Record *>> &pair : updated) {

            string id = pair.first;
            const vector<Record *> &updateRecords = pair.second;

            for (std::size_t j = 0; j != updateRecords.size(); ++j) {

//                log() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
//                log() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
//                log() << "~~~~~~~~~~~~~~~ sleeping for 5\" ~~~~~~~~~~~~~~~~";
//                log() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
//                log() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
//                sleep(5);

                Record *bsonObjRecord = updateRecords[j];
                bsonObjRecord->id = id;

                if (!bsonObjRecord->flushed) {
                    updateCommand.execute(*bsonObjRecord);
                }
            }

            log() << "flushUpdatedData::size=" << updateRecords.size();
        }
    }

    std::list<string> InMemoryRegistry::filterFlushed(const std::list<string> &removedDocumentIds) {

        log() << "--> InMemoryRegistry::filterFlushed";

        std::list<string> flushed;

        for (string id : removedDocumentIds) {
            filterFlushedUpdatedRecord(flushed, id);

            filterFlushedInsertedRecord(flushed, id);
        }

        return flushed;
    }

    void InMemoryRegistry::filterFlushedUpdatedRecord(std::list<string> &flushed, const string &id) const {
        log() << "InMemoryRegistry::filterFlushedUpdatedRecord( " << id << " )";

        std::map<string, std::vector<mongo::Record *>>::const_iterator iterator = updated.find(id);
        if (iterator != updated.end()) {
            std::vector<Record *> updatedList = iterator->second;

            log() << "updatedList.size: " << updatedList.size();

            if (std::all_of(updatedList.begin(), std::prev(updatedList.end()),
                            [](Record *record) { return record->flushed; })) {
                log() << "\t" << "document " << id << " updates has already bean flushed";
                flushed.push_back(id);
            }
        }
    }

    void InMemoryRegistry::filterFlushedInsertedRecord(std::list<string> &flushed, const string &id) const {
        std::map<string, mongo::Record *>::const_iterator iterator = inserted.find(id);
        if (iterator != inserted.end()) {
            Record *record = iterator->second;

            if (record->flushed) {
                log() << "\t" << "document " << id << " insertions has already bean flushed";
                flushed.push_back(id);
            }
        }
    }

    string getId(const BSONElement &actualElement) {
        return getId(actualElement.Obj());
    }

    string getId(const BSONObj &actualElement) {
        return actualElement.getField("_id").__oid().toString();
    }

    MutableDocument *createMutableDocument(const BSONObj &bsonObj) {
        Document document(bsonObj);
        return new MutableDocument(document);
    }

    void UpdateCommand::execute(Record &record) {

        throughputLock.flushLock();

        log() << "InMemoryRegistry::flushUpdatedBsonObj";
        Query query = QUERY("_id" << OID(record.id));

        log() << "flushUpdatedBsonObj :: query : " << query.toString()
              << " with " << record.bsonObj->toString();

        const BSONObj &o = connection.get()->findOne(record.ns, query);
        if (o.isEmpty()) {
            log() << "!!!!!!!!! object not fount in target !!!!!!!!!";
        }
        connection.get()->update(record.ns, query, *(record.bsonObj));

        record.flushed = true;

        throughputLock.flushUnlock();
    }

    UpdateCommand::UpdateCommand(ScopedDbConnection &connection, MigratorThroughputLock &throughputLock)
            : connection(connection), throughputLock(throughputLock) {}
}