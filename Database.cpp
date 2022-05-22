//
//  Database.cpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//

#include "Database.hpp"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>

#include "Config.hpp"
#include "ShowsTablesView.hpp"

namespace ECE141 {

Database::Database(const std::string aName, CreateDB)
    : name(aName), changed(true), storage(stream), entity_id(0) {
    std::string theDbPath = Config::getDBPath(aName);
    stream.clear();
    stream.open(theDbPath.c_str(), std::fstream::binary | std::fstream::in | std::fstream::out | std::fstream::trunc);
    stream.close();
    stream.open(theDbPath.c_str(), std::fstream::binary | std::fstream::binary | std::fstream::in | std::fstream::out);

    Block *theMetaBlock = new Block(BlockType::meta_block);
    theMetaBlock->header.theBlockId = 0;
    theMetaBlock->header.type = 'M';
    theMetaBlock->header.theEntityId = this->entity_id;
    theMetaBlock->header.theBlockNum = 0;
    strcpy(theMetaBlock->header.theTitle, this->name.c_str());
    // Make the entity_id 1
    uint32_t theNewId = 1;
    this->setEntityId(theNewId);

    StatusResult theCreateResult = storage.writeBlock(0, *theMetaBlock);
    delete theMetaBlock;
    // Increase the block count
    Database::blockCount++;
}

Database::Database(const std::string aName, OpenDB)
    : name(aName), changed(false), storage(stream) {
    // decode the meta block and populate the below variables
    // idx_map, entity_id
    // open for read/write
    std::string theDbPath = Config::getDBPath(name);
    stream.clear();
    stream.open(theDbPath.c_str(), std::fstream::binary | std::fstream::in | std::fstream::out);
    stream.seekg(0, std::ios::end);
    size_t theFileSize = stream.tellg();
    size_t theNumBlocks = theFileSize / ECE141::kBlockSize;
    this->blockCount = theNumBlocks;
    stream.seekg(0, std::ios::beg);
    StatusResult theStatus = storage.decodeMetaBlock(this->idx_map, this->entity_id);
    storage.populateFreeBlockQ();
}

Database::~Database() {
    if (changed) {
        // encode the below variables into Block 0
        // idx_map, entity_id
        std::map<std::string, uint32_t> theMap = this->idx_map;
        uint32_t                        theEntityId = this->entity_id;
        Block                           theBlock = storage.encodeMetaBlock(theMap, theEntityId);
        StatusResult                    theStatus = storage.writeBlock(0, theBlock);
    }
    stream.close();
}

uint32_t Database::getEntityFromMap(std::string aKey) {
    if (idx_map.find(aKey) != idx_map.end()) {
        return idx_map[aKey];
    }
    return -1;
}

// Check if our Queue that holds FreeBlocks is empty
bool Database::freeBlockQEmpty() {
    return storage.getQueue().empty();
}
// To get a Free BlockNumber
uint32_t Database::getFreeBlock() {
    if (storage.getQueue().empty()) {
        return blockCount;
    }
    uint32_t thefreeBlock = storage.getQueue().front();
    storage.getQueue().pop();
    return thefreeBlock;
}

// USE: Call this to dump the db for debug purposes...
StatusResult Database::dump(std::ostream &anOutput) {
    for (auto i = 0; i < blockCount; i++) {
        Block theTempBlock(BlockType::meta_block);
        storage.readBlock(i, theTempBlock);
        anOutput << theTempBlock.header.type << "|" << i << "|"
                 << "ExtraInfo";
    }

    return StatusResult{noError};
}

// To populate the Map <Table_name,BlockNum>
void Database::setEntityMap(std::string &aName, uint32_t &aBlocNum) {
    this->idx_map[aName] = aBlocNum;
}

StatusResult Database::openDB() {
    this->stream.open(Config::getDBPath(this->getDbName()).c_str(), std::fstream::out | std::fstream::in | std::fstream::app);
    return StatusResult(Errors::noError);
}

StatusResult Database::closeDB() {
    this->stream.close();
    return StatusResult(Errors::noError);
}

StatusResult Database::updateTable(UpdateTableStatement *aSqlStmt, std::ostream &anOutput) {
    DBQuery   &theDBQuery = aSqlStmt->getDBQuery();
    Entity    *theEntity = theDBQuery.getEntity();
    StringList theFieldsToUpdate;
    theDBQuery.getUpdateKeys(theFieldsToUpdate);
    uint32_t         theEntityHash = theEntity->getEntityHashString();
    RawRowCollection theRows;
    each(theEntityHash, theRows);
    RawRowCollection theFilteredRows;
    filterRows(theDBQuery, theRows, theFilteredRows);
    for (auto &r : theFilteredRows) {
        for (auto &s : theFieldsToUpdate) {
            Value theNewVal = theDBQuery.getKeyValue(s);
            r.set(s, theNewVal);
        }
        Block theNewBlock;
        r.getBlock(theNewBlock);
        storage.writeBlock(r.getStorageBlockNumber(), theNewBlock);
    }
    anOutput << "Query Ok. " << theFilteredRows.size() << " rows affected (" << Config::getTimer().elapsed() << " sec)"
             << "\n";
    return StatusResult(Errors::noError);
}
StatusResult Database::deleteRow(DeleteRowStatement *aSqlStmt, std::ostream &anOutput) {
    DBQuery &theDBQuery = aSqlStmt->getDBQuery();
    Entity  *theEntity = theDBQuery.getEntity();
    uint32_t theEntityHash = theEntity->getEntityHashString();
    RawRowCollection theRows;
    each(theEntityHash, theRows);
    RawRowCollection theFilteredRows;
    filterRows(theDBQuery, theRows, theFilteredRows);
    StatusResult theStatus(Errors::noError);
    for (auto &r : theFilteredRows) {
        uint32_t theRowBlockNum = r.getStorageBlockNumber();
        theStatus = storage.freeBlocks(theRowBlockNum);
        if (!theStatus) {
            return theStatus;
        }
    }
    anOutput << "Query Ok. " << theFilteredRows.size() << " rows affected (" << Config::getTimer().elapsed() << " sec)"
             << "\n";
    return StatusResult(Errors::noError);
}

// To get RowCollections based on DBQuery
bool Database::selectRows(DBQuery &aDB, Entity &anEntity, std::ostream &anOutput) {
    std::string theEntityName = aDB.getEntityName();
    uint32_t    theBlockNum = this->getEntityFromMap(theEntityName);
    Block      *theEntityBlock = new Block(BlockType::entity_block);
    storage.readBlock(theBlockNum, *theEntityBlock);
    anEntity.decodeBlock(*theEntityBlock);
    uint32_t theEntityHash = anEntity.getEntityHashString();
    // Set the DBQuery stringList//

    // Push the primary key first if it exists
    const Attribute *thePrimaryAttr = anEntity.getPrimaryKey();
    if (thePrimaryAttr != nullptr) {
        aDB.setAttrPrimary(thePrimaryAttr->getName());
    }
    if (aDB.getAllField()) {
        AttributeList theAttr = anEntity.getAttributes();
        for (int i = 0; i < theAttr.size(); i++) {
            if (!theAttr.at(i).isPrimaryKey()) {
                aDB.setAttr(theAttr.at(i).getName());
            }
        }
    }

    RawRowCollection theRows;
    this->each(theEntityHash, theRows);
    aDB.setEntity(&anEntity);
    RawRowCollection theFilterRows;
    filterRows(aDB, theRows, theFilterRows);
    delete theEntityBlock;
    ShowsTablesView *theShow = new ShowsTablesView(false);
    theShow->showQuery(theFilterRows, aDB, anOutput);
    return true;
}
// To filter the RowCollection based on WHERE,ORDER BY and LIMIT
void Database::filterRows(DBQuery &aDB, RawRowCollection &theRow, RawRowCollection &theFilteredRow) {
    theFilteredRow.assign(theRow.begin(), theRow.end());
    FilterRow    *theFilter = new FilterRow();
    FilterKeyword theFilterKey = aDB.getFilterKey();
    theFilter->filterWhere(aDB, theFilteredRow).filterOrderBy(aDB, theFilteredRow).filterLimit(aDB, theFilteredRow);
    delete theFilter;
}

// To get all Rows of a given EntityHash
void Database::each(uint32_t &anEntityHash, RawRowCollection &theRows) {
    int theCurrPos = stream.tellg();
    stream.seekg(0, std::ios::end);
    size_t theFileSize = stream.tellg();
    size_t theNumBlocks = theFileSize / ECE141::kBlockSize;
    stream.seekg(0, stream.beg);
    for (int i = 0; i < theNumBlocks; i++) {
        Block *aBlock = new Block();
        storage.readBlock(i, *aBlock);
        if (aBlock->header.theTableNameHash == anEntityHash && aBlock->header.type == 'D') {
            // It's a data block, so decode the rowBlock
            // Create a Data and Row object
            KeyValues theData;
            Row      *theRow = new Row();
            theRow->decode(*aBlock);
            theRows.push_back(*theRow);
            delete theRow;
        }

        delete aBlock;
    }
    stream.seekg(theCurrPos, stream.beg);

    return;
}

StatusResult Database::createTable(CreateTableStatement *aStmt, std::ostream &anOutput) {
    CreateTableStatement *theStatement = aStmt;
    std::string           theTableName = theStatement->getTableName();
    Entity               *theEntity = new Entity(theStatement->getTableName());
    uint32_t              theCurrentEntityId = this->getEntityId();
    theEntity->setBlockId(theCurrentEntityId);
    std::vector<Attribute> theAttr = theStatement->getAttributevector();

    for (size_t i = 0; i < theAttr.size(); i++) {
        theEntity->addAttribute(theAttr.at(i));
    }

    // creating the table
    // Store the entity as a block
    StatusResult theStatus;
    // Checking if the table already exists
    bool theEntityExists = this->checkEntityInMap(theEntity->getName());
    if (theEntityExists) {
        return StatusResult(Errors::tableExists);
    }

    // Check if duplicate attributes
    bool theDuplicateAttrCheck = theEntity->checkDuplicateAttr();
    if (theDuplicateAttrCheck) {
        return StatusResult(Errors::attributeExists);
    }

    // This part of code handles the blockifying the entity
    Block theConvertedBlock = theEntity->getBlock();

    // For entity theBlockId is the entity number in the database
    theConvertedBlock.header.theBlockId = this->getEntityId();
    theConvertedBlock.header.theTableNameHash = Helpers::hashString(theEntity->getName().c_str());
    theEntity->setEntityHashString(theConvertedBlock.header.theTableNameHash);
    // For entity theEntityId is the current value of the auto_incr field

    theConvertedBlock.header.theEntityId = theEntity->getAutoIncr();
    bool     theCheckIfQIsEmpty = this->freeBlockQEmpty();
    uint32_t theBlockNum = this->getFreeBlock();
    theConvertedBlock.header.theBlockNum = theBlockNum;
    theEntity->setBlockNum(theBlockNum);
    this->getStorage().writeBlock(theBlockNum, theConvertedBlock);

    // Block count only incremented if no free block in between start to end of file
    uint32_t theNewBlockCount = theCheckIfQIsEmpty ? theBlockNum + 1 : this->getBlockCount();

    this->setBlockCount(theNewBlockCount);
    this->setChange(true);
    anOutput << "Query OK, 1 row affected";
    anOutput << " (" << Config::getTimer().elapsed() << " sec)" << std::endl;
    theStatus = StatusResult(Errors::noError);

    if (theStatus) {
        this->setEntityMap(theTableName, theBlockNum);
        uint32_t theEntityId = this->getEntityId() + 1;
        this->setEntityId(theEntityId);
    }
    delete theEntity;
    return theStatus;
}
StatusResult Database::insertTable(InsertTableStatement *aStmt, std::ostream &anOutput) {
    InsertTableStatement *theStatement = aStmt;
    Entity               *theEntity = new Entity(theStatement->getTableName());
    Block                *theDescribeBlock = new Block(BlockType::entity_block);
    uint32_t              theBlockNum = this->getEntityFromMap(theStatement->getTableName());
    bool                  theCheckIfQIsEmpty = this->freeBlockQEmpty();
    uint32_t              theBlockCount = this->getFreeBlock();
    this->getStorage().readBlock(theBlockNum, *theDescribeBlock);
    theEntity->decodeBlock(*theDescribeBlock);
    const Attribute *theAttr = theEntity->getPrimaryKey();
    std::string      thePrimaryKey = "";
    if (theAttr != nullptr) {
        thePrimaryKey = theAttr->getName();
    }

    delete theDescribeBlock;
    RowVectors theRowData = *(theStatement->getSQLProcessor()->getTheRowData());
    for (size_t i = 0; i < theRowData.size(); i++) {
        // blockify each row. The getBlock function in Row.cpp

        Block   *theRowBlock = new Block(BlockType::data_block);
        uint32_t theRowId = theEntity->getAutoIncr();

        //////////////////////////////////////////
        // Seeting the Autoincr id for primary Key

        if (thePrimaryKey != "") {
            Value thePrimaryValue = (int)theRowId;
            theRowData.at(i).set(thePrimaryKey, thePrimaryValue);
        }

        ///////////////////////////////////////////

        theRowData.at(i).setBlockNumber(theRowId);
        theRowData.at(i).tableName = theStatement->getTableName();
        theRowData.at(i).entityId = Helpers::hashString(theStatement->getTableName().c_str());
        theRowData.at(i).setStorageBlockNumber(theBlockCount);
        theRowData.at(i).getBlock(*theRowBlock);
        this->getStorage().writeBlock(theBlockCount, *theRowBlock);

        theEntity->insertDataRow(theBlockCount);
        theRowId++;
        theEntity->setAutoIncr(theRowId);
        // update blockCount
        theBlockCount = theCheckIfQIsEmpty ? ++theBlockCount : this->getBlockCount();
        this->setBlockCount(theBlockCount);
    }
    // Encode the entity block back
    Block theEntityBlock = theEntity->getBlock();
    this->getStorage().writeBlock(theBlockNum, theEntityBlock);

    delete theEntity;
    anOutput << "Query Ok, " << theRowData.size() << " rows affected (" << Config::getTimer().elapsed() << " secs)" << std::endl;
    return StatusResult(Errors::noError);
}
StatusResult Database::showTable(ShowTableStatement *aStmt, std::ostream &anOutput) {
    std::vector<std::string> theTableVector;
    std::stringstream        ss;
    ss << "Tables_in_" << this->getDbName();
    theTableVector.push_back(ss.str());
    // uint32_t theBlockNum = (*currentActiveDbPtr)->getBlockCount();
    for (auto const &imap : this->getIdxMap()) {
        theTableVector.push_back(imap.first);
    }
    size_t theLongestString = 0;
    for (size_t i = 0; i < theTableVector.size(); i++) {
        theLongestString =
            std::max(theLongestString, theTableVector.at(i).length());
    }
    MessageViewer *theMessageHandler = new MessageViewer();
    theMessageHandler->showTableView(anOutput, theTableVector,
                                     theLongestString);

    anOutput << theTableVector.size() - 1 << " rows in set ("
             << Config::getTimer().elapsed() << " sec.)"
             << "\n";

    delete theMessageHandler;
    return StatusResult(Errors::noError);
}

StatusResult Database::describeTable(DescribeTableStatement *aStmt, std::ostream &anOutput) {
    Block       theDescribeBlock;
    std::string theTableName = aStmt->getTableName();
    uint32_t    theBlockNum = this->getEntityFromMap(theTableName);
    this->getStorage().readBlock(theBlockNum, theDescribeBlock);

    if (theDescribeBlock.header.theTitle == theTableName) {
        // decode the block
        Entity *theEntity = new Entity(theTableName);
        theEntity->decodeBlock(theDescribeBlock);
        MessageViewer *theMessageHandler = new MessageViewer();
        theMessageHandler->printAttrTable(anOutput, theEntity->getAttributes());
        anOutput << theEntity->getAttributes().size() << " rows in set ("
                 << Config::getTimer().elapsed() << " sec.)" << std::endl;
        delete theEntity;
        delete theMessageHandler;
    }
    return StatusResult(Errors::noError);
}

StatusResult Database::dropTable(DropTableStatement *aStmt, std::ostream &anOutput) {
    std::string theTableName = aStmt->getTableName();
    if (!this->checkEntityInMap(theTableName)) {
        return StatusResult(Errors::unknownTable);
    }
    uint32_t         theBlockNum = this->getEntityFromMap(theTableName);
    StatusResult     theStatus = this->getStorage().freeBlocks(theBlockNum);
    uint32_t         theEntityHash = Helpers::hashString(theTableName.c_str());
    RawRowCollection theRows;
    this->each(theEntityHash, theRows);
    for (auto &r : theRows) {
        uint32_t theRowBlockNum = r.getStorageBlockNumber();
        theStatus = this->getStorage().freeBlocks(theRowBlockNum);
    }
    if (theStatus) {
        this->setChange(true);
        this->removeEntityFromMap(theTableName);
        anOutput << "Query OK, " << theRows.size() + 1 << " rows affected (" << Config::getTimer().elapsed()
                 << " sec)"
                 << "\n";
    }

    return theStatus;
}

StatusResult Database::showQuery(SelectStatement* aStmt,std::ostream &anOutput){
    DBQuery theDBQuery = aStmt->getDBQuery();
    Entity *theEntity = new Entity(theDBQuery.getEntityName());
    this->selectRows(theDBQuery, *theEntity, anOutput);
    delete theEntity;
    return StatusResult(Errors::noError);
}

}  // namespace ECE141
