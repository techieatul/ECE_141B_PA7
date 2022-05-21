//
//  Database.hpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//
#pragma once
#ifndef Database_hpp
#define Database_hpp

#include <stdio.h>

#include <fstream>
#include <map>
#include <queue>
#include <string>

#include "CreateTableStatement.hpp"
#include "DBQuery.hpp"
#include "DeleteRowStatement.hpp"
#include "DescribeTableStatement.hpp"
#include "DropTableStatement.hpp"
#include "Entity.hpp"
#include "FilterRow.hpp"
#include "InsertTableStatement.hpp"
#include "Row.hpp"
#include "SQLStatement.hpp"
#include "SelectStatement.hpp"
#include "ShowTableStatement.hpp"
#include "Storage.hpp"
#include "UpdateTableStatement.hpp"

namespace ECE141 {
class CreateTableStatement;

class Database {
   public:
    Database(const std::string aName, CreateDB);
    Database(const std::string aName, OpenDB);
    // Atul added to access protected member name
    std::string getDbName() { return name; }
    virtual ~Database();

    StatusResult                     dump(std::ostream &anOutput);
    Storage                         &getStorage() { return storage; }
    StatusResult                     openDB();
    StatusResult                     closeDB();
    uint32_t                         getBlockCount() { return blockCount; }
    void                             setBlockCount(uint32_t &aBlockCount) { blockCount = aBlockCount; }
    void                             setEntityMap(std::string &aName, uint32_t &aBlocNum);
    uint32_t                         getEntityFromMap(std::string aKey);
    void                             setEntityId(uint32_t &aId) { entity_id = aId; }
    uint32_t                         getEntityId() { return entity_id; }
    void                             removeEntityFromMap(const std::string &aName) { idx_map.erase(aName); }
    void                             setChange(bool aValue) { changed = aValue; }
    bool                             checkEntityInMap(const std::string aName) { return idx_map.find(aName) != idx_map.end(); }
    std::map<std::string, uint32_t> &getIdxMap() { return idx_map; }
    bool                             selectRows(DBQuery &aDB, Entity &anEntity, std::ostream &anOutput);
    void                             each(uint32_t &anEntityHash, RawRowCollection &theRow);
    void                             filterRows(DBQuery &aDB, RawRowCollection &theRow, RawRowCollection &theOrderedRow);
    uint32_t                         getFreeBlock();
    bool                             freeBlockQEmpty();
    StatusResult                     updateTable(UpdateTableStatement *aSqlStmt, std::ostream &anOutput);
    StatusResult                     deleteRow(DeleteRowStatement *aSqlStmt, std::ostream &anOutput);
    StatusResult                     createTable(CreateTableStatement *aStmt, std::ostream &anOutput);
    StatusResult                     showTable(ShowTableStatement *aStmt, std::ostream &anOutput);
    StatusResult                     describeTable(DescribeTableStatement *aStmt, std::ostream &anOutput);
    StatusResult                     insertTable(InsertTableStatement *aStmt, std::ostream &anOutput);
    StatusResult                     dropTable(DropTableStatement *aStmt, std::ostream &anOutput);
    StatusResult                     showQuery(SelectStatement *aStmt, std::ostream &anOutput);

   protected:
    std::string                     name;
    std::fstream                    stream;  // low level stream used by storage...
    Storage                         storage;
    uint32_t                        blockCount = 0;
    std::map<std::string, uint32_t> idx_map;
    uint32_t                        entity_id;
    bool                            changed;  // might be helpful, or ignore if you prefer.
};

}  // namespace ECE141
#endif /* Database_hpp */
