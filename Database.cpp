//
//  Database.cpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//

#include "Database.hpp"

#include <iomanip>
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <cstring>
#include <algorithm>
#include "ShowsTablesView.hpp"

#include "Config.hpp"

namespace ECE141 {

Database::Database(const std::string aName, CreateDB)
    : name(aName), changed(true), storage(stream),entity_id(0) {
    std::string theDbPath = Config::getDBPath(aName);
    stream.clear();
    stream.open(theDbPath.c_str(), std::fstream::binary | std::fstream::in | std::fstream::out | std::fstream::trunc);
    stream.close();
    stream.open(theDbPath.c_str(), std::fstream::binary | std::fstream::binary | std::fstream::in | std::fstream::out);
    
    Block* theMetaBlock = new Block(BlockType::meta_block);
    theMetaBlock->header.theBlockId = 0;
    theMetaBlock->header.type = 'M';
    theMetaBlock->header.theEntityId = this->entity_id;
    theMetaBlock->header.theBlockNum = 0;
    strcpy(theMetaBlock->header.theTitle,this->name.c_str());
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
    stream.seekg(0,std::ios::end);
    size_t theFileSize = stream.tellg();
    size_t theNumBlocks = theFileSize/ECE141::kBlockSize;
    this->blockCount = theNumBlocks;
    stream.seekg(0,std::ios::beg);
    StatusResult theStatus = storage.decodeMetaBlock(this->idx_map,this->entity_id);
    storage.populateFreeBlockQ();
    
}

Database::~Database() {
    if (changed) {
        //encode the below variables into Block 0
       // idx_map, entity_id
       std::map<std::string,uint32_t> theMap = this->idx_map;
       uint32_t theEntityId = this->entity_id;
       Block theBlock = storage.encodeMetaBlock(theMap,theEntityId);
       StatusResult theStatus = storage.writeBlock(0,theBlock);
    }
    stream.close();
    
}

uint32_t Database::getEntityFromMap(std::string aKey){
    if(idx_map.find(aKey)!=idx_map.end()){
        return idx_map[aKey];
    }
    return -1;
}

// Check if our Queue that holds FreeBlocks is empty
bool Database::freeBlockQEmpty(){
    return storage.getQueue().empty();
}
// To get a Free BlockNumber
uint32_t Database::getFreeBlock(){
    if(storage.getQueue().empty()){
        return blockCount;
    }
    uint32_t thefreeBlock = storage.getQueue().front();
    storage.getQueue().pop();
    return thefreeBlock;
}

// USE: Call this to dump the db for debug purposes...
StatusResult Database::dump(std::ostream &anOutput) {
    for (auto i=0; i<blockCount; i++) {
        Block theTempBlock(BlockType::meta_block);
        storage.readBlock(i, theTempBlock);
        anOutput << theTempBlock.header.type << "|" << i << "|" << "ExtraInfo";
    }

    return StatusResult{noError};
}

// To populate the Map <Table_name,BlockNum>
void Database::setEntityMap(std::string& aName, uint32_t &aBlocNum){
    this->idx_map[aName] = aBlocNum;
}


StatusResult Database::openDB(){
    this->stream.open(Config::getDBPath(this->getDbName()).c_str(), std::fstream::out | std::fstream::in | std::fstream::app);
    return StatusResult(Errors::noError);

}

StatusResult Database::closeDB(){
    this->stream.close();
    return StatusResult(Errors::noError);
}

StatusResult Database::updateTable(SQLStatement *aSqlStmt,std::ostream &anOutput){
    DBQuery& theDBQuery = aSqlStmt->getDBQuery();
    Entity* theEntity = theDBQuery.getEntity();
    StringList theFieldsToUpdate;
    theDBQuery.getUpdateKeys(theFieldsToUpdate);
    uint32_t theEntityHash = theEntity->getEntityHashString();
    RawRowCollection theRows;
    each(theEntityHash,theRows);
    RawRowCollection theFilteredRows;
    filterRows(theDBQuery,theRows,theFilteredRows);
    for(auto &r: theFilteredRows){
        for(auto &s: theFieldsToUpdate){
            Value theNewVal = theDBQuery.getKeyValue(s);
            r.set(s,theNewVal);
        }
        Block theNewBlock;
        r.getBlock(theNewBlock);
        storage.writeBlock(r.getStorageBlockNumber(),theNewBlock);
    }
    anOutput<<"Query Ok. "<< theFilteredRows.size() <<" rows affected ("<<Config::getTimer().elapsed()<<" sec)"<<"\n";
    return StatusResult(Errors::noError);

}
StatusResult Database::deleteRow(SQLStatement *aSqlStmt,std::ostream &anOutput){
    DBQuery& theDBQuery = aSqlStmt->getDBQuery();
    Entity* theEntity = theDBQuery.getEntity();
    uint32_t theEntityHash = theEntity->getEntityHashString();
    RawRowCollection theRows;
    each(theEntityHash,theRows);
    RawRowCollection theFilteredRows;
    filterRows(theDBQuery,theRows,theFilteredRows);
    StatusResult theStatus(Errors::noError);
    for(auto &r: theFilteredRows){
        uint32_t theRowBlockNum = r.getStorageBlockNumber();
        theStatus = storage.freeBlocks(theRowBlockNum);
        if(!theStatus){
            return theStatus;
        }
    }
    anOutput<<"Query Ok. "<< theFilteredRows.size() <<" rows affected ("<<Config::getTimer().elapsed()<<" sec)"<<"\n";
    return StatusResult(Errors::noError);

}

// To get RowCollections based on DBQuery
bool Database::selectRows(DBQuery &aDB, Entity &anEntity,std::ostream &anOutput){
    std::string theEntityName = aDB.getEntityName();
    uint32_t theBlockNum = this->getEntityFromMap(theEntityName);
    Block* theEntityBlock = new Block(BlockType::entity_block);
    storage.readBlock(theBlockNum,*theEntityBlock);
    anEntity.decodeBlock(*theEntityBlock);
    uint32_t theEntityHash = anEntity.getEntityHashString();
    // Set the DBQuery stringList// 

    //Push the primary key first if it exists
    const Attribute* thePrimaryAttr = anEntity.getPrimaryKey();
    if(thePrimaryAttr != nullptr){
        aDB.setAttrPrimary(thePrimaryAttr->getName());
    }
    if(aDB.getAllField()){
        AttributeList theAttr = anEntity.getAttributes();
        for(int i = 0; i<theAttr.size(); i++){
            if(!theAttr.at(i).isPrimaryKey()){
                aDB.setAttr(theAttr.at(i).getName());
            }

        }
    }

    RawRowCollection  theRows;
    this->each(theEntityHash,theRows);
    aDB.setEntity(&anEntity);
    RawRowCollection theFilterRows;
    filterRows(aDB,theRows,theFilterRows);
    delete theEntityBlock;
    ShowsTablesView* theShow = new ShowsTablesView(false);
    theShow->showQuery(theFilterRows, aDB,anOutput);
    return true;

}
// To filter the RowCollection based on WHERE,ORDER BY and LIMIT
void Database::filterRows(DBQuery &aDB,RawRowCollection &theRow,RawRowCollection &theFilteredRow){
    theFilteredRow.assign(theRow.begin(),theRow.end());
    FilterRow* theFilter = new FilterRow();
    FilterKeyword theFilterKey = aDB.getFilterKey();
    theFilter->filterWhere(aDB,theFilteredRow).filterOrderBy(aDB,theFilteredRow).filterLimit(aDB,theFilteredRow);
    delete theFilter;
    
}

// To get all Rows of a given EntityHash
void Database::each(uint32_t &anEntityHash, RawRowCollection &theRows){
    
    int theCurrPos = stream.tellg();
    stream.seekg(0,std::ios::end);
    size_t theFileSize = stream.tellg();
    size_t theNumBlocks = theFileSize/ECE141::kBlockSize;
    stream.seekg(0,stream.beg);
    for(int i = 0; i<theNumBlocks;i++){
        Block* aBlock = new Block();
        storage.readBlock(i,*aBlock);
        if(aBlock->header.theTableNameHash==anEntityHash && aBlock->header.type=='D'){
            // It's a data block, so decode the rowBlock
            // Create a Data and Row object
            KeyValues theData;
            Row* theRow = new Row();
            theRow->decode(*aBlock);
            theRows.push_back(*theRow);
            delete theRow;
        }

        delete aBlock;

    }
    stream.seekg(theCurrPos,stream.beg);

    return;

}

}  // namespace ECE141
