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

    std::vector<BSONObj *> Registry::read(const BSONObj &query) {
//        return hasUpdated(id) ?
//               updated.find(id)->second :
//               getOrNull(inserted, id);
        throw std::runtime_error("Unimplemented operation exception in registry");
    }

    BSONObj *Registry::update(string id, BSONObj *object) {

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

    BSONObj *Registry::write(string id, BSONObj *object) {
        throw std::runtime_error("Unimplemented operation exception in registry");
    }

    void Registry::remove(string id) {
        removed.insert(id);
    }

    bool Registry::hasUpdated(const string &id) const {
        return updated.find(id) != updated.end();
    }

    bool Registry::hasRemoved(const string &id) const {
        return removed.find(id) != removed.end();
    }

    const std::map<string, BSONObj *> &Registry::getInserted() const {
        return inserted;
    }

    const std::map<string, std::vector<BSONObj *>> &Registry::getUpdated() const {
        return updated;
    }

    const std::set<string> &Registry::getRemoved() const {
        return removed;
    }

    BSONObj Registry::applyUpdates(const BSONElement &actualElement) const {
        string id = getId(actualElement);

        BSONObj finalObj = actualElement.Obj();

        vector<BSONObj *> sequenceOfUpdates = this->getUpdated().find(id)->second;
        for (BSONObj *update : sequenceOfUpdates) {
            if (update->hasElement("$set")) {
                MutableDocument *mutableUpdate = createMutableDocument(finalObj);
                for (BSONElement field : update->getObjectField("$set")) {

                    const Value &val = Value(field);
                    log() << "updating " << field.toString();

                    mutableUpdate->setField(field.fieldNameStringData(), val);
                }

                const Document &document = mutableUpdate->freeze();
                delete mutableUpdate;

                finalObj = document.toBson();

            } else {
                MutableDocument *mutableUpdate = createMutableDocument(*update);
                mutableUpdate->addField("_id", Value(finalObj.getField("_id")));
                const Document &document = mutableUpdate->freeze();
                delete mutableUpdate;
                finalObj = document.toBson();

                log() << "replacing with " << document.toString();
            }
        }
        return finalObj;
    }

    string getId(const BSONElement &actualElement) {
        return actualElement.Obj().getField("_id").__oid().toString();
    }

    MutableDocument* createMutableDocument(const BSONObj &bsonObj) {
        Document document(bsonObj);
        return new MutableDocument(document);
    }
}