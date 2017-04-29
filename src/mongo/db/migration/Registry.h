//
// Created by yannis on 3/12/2016.
//

#include <map>
#include <set>
#include <vector>
#include <string>
#include <mongo/client/connpool.h>

#include "mongo/bson/bsonobj.h"
#include "mongo/db/pipeline/document.h"

namespace mongo {

    using std::string;

    string getId(const BSONElement &actualElement);

    string getId(const BSONObj &actualElement);

    MutableDocument *createMutableDocument(const BSONObj &bsonObj);

    struct Record {
        BSONObj *bsonObj;
        string ns;
        string id;
        bool flushed = false;
    };

    class Registry {
    public:
        virtual std::vector<BSONObj *> read(const BSONObj &query) = 0;

        virtual BSONObj *insert(BSONObj *object, string ns) = 0;

        virtual BSONObj *update(const string &id, BSONObj *object, string ns) = 0;

        virtual void remove(string id, string ns) = 0;

        virtual bool hasUpdated(const string &id) const = 0;

        virtual bool hasRemoved(const string &id) const = 0;

        virtual BSONObj applyUpdates(const BSONElement &actualElement) const = 0;

        virtual std::vector<BSONObj> getInserted() const = 0;

        virtual void flushDeletedData(mongo::ScopedDbConnection &connection) = 0;

        virtual void flushDeletedRecord(ScopedDbConnection &connection, const string &id, string ns) const = 0;

        virtual void flushUpdatedData(mongo::ScopedDbConnection &connection) = 0;

        virtual std::list<string> filterFlushed(const std::list<string> &removedDocumentIds) = 0;

        virtual void flushUpdatedRecord(ScopedDbConnection &connection, string id, BSONObj *update, string ns)= 0;
    };

    class InMemoryRegistry : public Registry {
        std::map<string, Record *> inserted;
        std::map<string, std::vector<Record *>> updated;
        std::map<string, Record *> removed;

        BSONObj updateFields(const BSONObj &actualObj, const BSONObj *update) const;

        BSONObj replaceFields(const BSONObj &actualObj, const BSONObj *update) const;

        void filterFlushedUpdatedRecord(std::list<string> &flushed, const string &id) const;

        void filterFlushedInsertedRecord(std::list<string> &flushed, const string &id) const;

    public:
        std::vector<BSONObj *> read(const BSONObj &query);

        BSONObj *insert(BSONObj *object, string ns);

        BSONObj *update(const string &id, BSONObj *object, string ns);

        void remove(string id, string ns);

        bool hasUpdated(const string &id) const;

        bool hasRemoved(const string &id) const;

        BSONObj applyUpdates(const BSONElement &actualElement) const;

        std::vector<BSONObj> getInserted() const;

        void flushDeletedData(mongo::ScopedDbConnection &connection);

        void flushUpdatedData(mongo::ScopedDbConnection &connection);

        void flushDeletedRecord(ScopedDbConnection &connection, const string &id, string ns) const;

        std::list<string> filterFlushed(const std::list<string> &removedDocumentIds);

        void flushUpdatedRecord(ScopedDbConnection &connection, string id, BSONObj *update, string ns);

        void flushUpdatedBsonObj(ScopedDbConnection &connection, const string &id, const BSONObj &obj, string ns) const;
    };

}