//
// Created by yannis on 3/12/2016.
//

#define MONGO_LOG_DEFAULT_COMPONENT ::mongo::logger::LogComponent::kCommand

#include "mongo/util/log.h"
#include "mongo/db/migration/Registry.h"

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

    BSONObj *InMemoryRegistry::update(string id, BSONObj *object) {

        const std::map<string, std::vector<BSONObj *>>::iterator &iterator = updated.find(id);
        if (iterator == updated.end()) {
            std::vector<BSONObj *> *updates = new std::vector<BSONObj *>();
            updates->push_back(object);
            updated.insert(std::make_pair(id, *updates));
        } else {
            std::vector<BSONObj *> &updates = iterator->second;
            updates.push_back(object);
        }

        return object;
    }

    BSONObj *InMemoryRegistry::insert(BSONObj *object) {
        inserted.insert(std::make_pair(getId(*object), object));

        return object;
    }

    void InMemoryRegistry::remove(string id) {
        updated.erase(id);
        inserted.erase(id);

        removed.insert(id);
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

        vector<BSONObj *> sequenceOfUpdates = this->updated.find(id)->second;
        for (BSONObj *update : sequenceOfUpdates) {
            if (update->hasElement("$set")) {
                finalObj = updateFields(finalObj, update);
            } else {
                finalObj = replaceFields(finalObj, update);
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
            log() << "updating " << field.toString();

            mutableUpdate->setField(field.fieldNameStringData(), val);
        }

        const Document &document = mutableUpdate->freeze();
        delete mutableUpdate;

        return document.toBson();
    }

    std::vector<BSONObj> InMemoryRegistry::getInserted() const {
        vector<BSONObj> insertedValues;

        for (const std::pair<const string, BSONObj *> & pair : inserted) {
            insertedValues.push_back(*pair.second);
        }

        return insertedValues;
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
}