//
//  SQLProcessor.cpp
//  PA3
//
//  Created by rick gessner on 4/2/22.
//

#include "SQLProcessor.hpp"

#include <sstream>
#include <unordered_set>

#include "Attribute.hpp"
#include "Config.hpp"
#include "Database.hpp"
#include "Entity.hpp"
#include "Helpers.hpp"
#include "Row.hpp"
#include "SQLStatement.hpp"
#include "Tokenizer.hpp"
namespace ECE141 {

using SQLStmtFactory = Statement *(*)(SQLProcessor *aProc,
                                   Tokenizer   &aTokenizer);

SQLProcessor::SQLProcessor(std::ostream &anOutput,
                           Database    **aCurrentActiveDbPtr)
    : CmdProcessor(anOutput), currentActiveDbPtr(aCurrentActiveDbPtr) {}

SQLProcessor::~SQLProcessor() {}


void SQLProcessor::setDatabaseInUse(Database *database){
    (*currentActiveDbPtr) = database;
}

// To remove a database connection
void SQLProcessor::releaseDatabase(){
    delete (*currentActiveDbPtr);
}
// Check if DB exists
bool SQLProcessor::dbExists(const std::string &aDBName) {
    // Atul added
    std::string theDBPath = Config::getDBPath(aDBName);
    return std::filesystem::exists(theDBPath);
}

// Check if the current active DB matches aDBName
bool SQLProcessor::checkActiveDBName(const std::string &aDBName) {
    if ((*currentActiveDbPtr) != nullptr && (*currentActiveDbPtr)->getDbName() == aDBName) {
            return true;
    }
    return false;
}

// To return current active Database pointer
Database *SQLProcessor::getDatabaseInUse() {
    return *currentActiveDbPtr;
}

RowVectors* SQLProcessor::getTheRowData() {
    return &theRowData;
}

// To dispact respectivve recognize function
std::map<Keywords,recognizeVisitor> theRecognizemap{
    {Keywords::create_kw,CreateTableStatement::checkCreateTable},
    {Keywords::show_kw,ShowTableStatement::checkShowTable},
    {Keywords::describe_kw,DescribeTableStatement::checkDescribeTable},
    {Keywords::drop_kw,DropTableStatement::checkDropTable},
    {Keywords::insert_kw,InsertTableStatement::checkInsertTable},
    {Keywords::select_kw, SelectStatement::checkSelectTable},
    {Keywords::update_kw,UpdateTableStatement::checkUpdateTable},
    {Keywords::delete_kw,DeleteRowStatement::checkDeleteRow},

  };
  
// Dispatcher to create statement based on keyword
std::map<Keywords,SQLStmtFactory> theSQLStatementMap{
    {Keywords::create_kw,CreateTableStatement::createTableStatement},
    {Keywords::show_kw,ShowTableStatement::showTableStatement},
    {Keywords::describe_kw,DescribeTableStatement::describeTableStatement},
    {Keywords::insert_kw,InsertTableStatement::insertTableStatement},
    {Keywords::select_kw,SelectStatement::selectStatement},
    {Keywords::update_kw,UpdateTableStatement::updateTableStatement},
    {Keywords::delete_kw,DeleteRowStatement::deleteRowStatement},

};

// Function to check if given command is a valid command
CmdProcessor *SQLProcessor::recognizes(Tokenizer &aTokenizer) {
    Keywords theKw = aTokenizer.current().keyword;
    if(theRecognizemap.find(theKw)!=theRecognizemap.end()){
        if(theRecognizemap[theKw](aTokenizer)){
            SQLProcessor::keywordStatement = theKw;
            return this;
        }

    }
    return nullptr;
}

// To create Statement for CREATE TABLE
Statement* SQLProcessor::createTableStatement(Tokenizer &aTokenizer){
    CreateTableStatement *theCreateTable = new CreateTableStatement(this,Keywords::create_kw);
    theCreateTable->createTableStatement(this,aTokenizer);
    return theCreateTable;

}
// To create Statement for SHOW TABLES
Statement* SQLProcessor::showTableStatement(Tokenizer &aTokenizer){
    ShowTableStatement *theShowTable = new ShowTableStatement(Keywords::show_kw);
    return theShowTable;

}
// To create Statement for DESCRIBE <TABLE_NAME>
Statement* SQLProcessor::describeTableStatement(Tokenizer &aTokenizer){
    DescribeTableStatement *theDescribeTable = new DescribeTableStatement(Keywords::describe_kw);
    aTokenizer.skipTo(TokenType::identifier);
    theDescribeTable->setTableName(aTokenizer.current().data);
    return theDescribeTable;

}
// To create Statement for DROP TABLE
Statement* SQLProcessor::dropTableStatement(Tokenizer &aTokenizer){
    DropTableStatement *theDropTable = new DropTableStatement(Keywords::drop_kw);
    aTokenizer.skipTo(TokenType::identifier);
    theDropTable->setTableName(aTokenizer.current().data);
    return theDropTable;

}
// To create Statement for INSERT INTO
Statement* SQLProcessor::insertTableStatement(Tokenizer &aTokenizer){
     aTokenizer.skipTo(TokenType::identifier);
     Block    theDescribeBlock;
     uint32_t theBlockNum = (*currentActiveDbPtr)->getEntityFromMap(aTokenizer.current().data);
     (*currentActiveDbPtr)->getStorage().readBlock(theBlockNum, theDescribeBlock);
     Entity *theEntity;

     if (theDescribeBlock.header.theTitle == aTokenizer.current().data) {
         theEntity = new Entity(aTokenizer.current().data);
         theEntity->decodeBlock(theDescribeBlock);
     }
     InsertTableStatement *theInsertTable = new InsertTableStatement(this, Keywords::insert_kw, &theRowData, theEntity);

     theInsertTable->setTableName(aTokenizer.current().data);
     theInsertTable->parse(aTokenizer);
     delete theEntity;
     return theInsertTable;

}
// To create Statement for SELECT 
Statement* SQLProcessor::selectQueryStatement(Tokenizer &aTokenizer){
    aTokenizer.skipTo(Keywords::from_kw);
    aTokenizer.next();
    Block    theDescribeBlock;
    uint32_t theBlockNum = (*currentActiveDbPtr)->getEntityFromMap(aTokenizer.current().data);
    (*currentActiveDbPtr)->getStorage().readBlock(theBlockNum, theDescribeBlock);
    Entity *theEntity;

    if (theDescribeBlock.header.theTitle == aTokenizer.current().data) {
        theEntity = new Entity(aTokenizer.current().data);
        theEntity->decodeBlock(theDescribeBlock);
    }
    SelectStatement *theSelectStatememt = new SelectStatement(Keywords::select_kw, theEntity);
    // Restart the token Index
    aTokenizer.restart();
    StatusResult theStatus = theSelectStatememt->parseStatement(aTokenizer);
    delete theEntity;
    return theSelectStatememt;

}

Statement* SQLProcessor::updateTableStatement(Tokenizer &aTokenizer){
    Block theDescribeBlock;
    std::string theTableName = aTokenizer.peek(1).data;
    uint32_t theBlockNum = (*currentActiveDbPtr)->getEntityFromMap(theTableName);

    if(theBlockNum==-1){
        return nullptr;
    }
    (*currentActiveDbPtr)->getStorage().readBlock(theBlockNum, theDescribeBlock);
    Entity *theEntity;
    if (theDescribeBlock.header.theTitle == theTableName) {
        theEntity = new Entity(theTableName);
        theEntity->decodeBlock(theDescribeBlock);
    }
    aTokenizer.restart();
    UpdateTableStatement *theUpdateStmt = new UpdateTableStatement(Keywords::update_kw,theEntity);
    StatusResult theStatus = theUpdateStmt->parse(aTokenizer);
    if(!theStatus){
        return nullptr;
    }
    delete theEntity;
    return theUpdateStmt;
    
}

Statement* SQLProcessor::deleteStatement(Tokenizer &aTokenizer){
    aTokenizer.skipTo(Keywords::from_kw);
    aTokenizer.next(); // Table Name
    std::string theTableName = aTokenizer.current().data;
    Block    theDescribeBlock;
    uint32_t theBlockNum = (*currentActiveDbPtr)->getEntityFromMap(theTableName);
    if(theBlockNum==-1){
        return nullptr;
    }
    (*currentActiveDbPtr)->getStorage().readBlock(theBlockNum, theDescribeBlock);
    Entity *theEntity;
    if (theDescribeBlock.header.theTitle == theTableName) {
        theEntity = new Entity(theTableName);
        theEntity->decodeBlock(theDescribeBlock);
    }

    DeleteRowStatement *theDeleteStmt = new DeleteRowStatement(Keywords::delete_kw,theEntity);
    StatusResult theStatus = theDeleteStmt->parse(aTokenizer);
    if(!theStatus){
        return nullptr;
    }
    delete theEntity;
    return theDeleteStmt;
    
}

// To create a Statement based on the command
Statement *SQLProcessor::makeStatement(Tokenizer    &aTokenizer,
                                       StatusResult &aResult) {
    // Atul: Added separate class to create statements for
    // create_table,show_table,drop_table, and describe_table
    Statement *theSQLStatement = nullptr;
    if(theSQLStatementMap.find(aTokenizer.current().keyword) != theSQLStatementMap.end()){
        theSQLStatement = theSQLStatementMap[aTokenizer.current().keyword](this,aTokenizer);
    }
    
    SQLProcessor::keywordStatement = Keywords::unknown_kw;  // resetting for the next command
    // Go to end of the command
    aTokenizer.skipTo(';');
    aTokenizer.next();
    return theSQLStatement;
}
// Dispatcher to create statement based on keyword
Statement *SQLProcessor::handleSqlStatements(Tokenizer &aTokenizer) {
    
    switch (aTokenizer.current().keyword) {
        case Keywords::create_kw:
            return createTableStatement(aTokenizer);
    
        case Keywords::show_kw:
            return showTableStatement(aTokenizer);
        
        case Keywords::describe_kw: 
            return describeTableStatement(aTokenizer);

        case Keywords::drop_kw:
            return dropTableStatement(aTokenizer);
            
        case Keywords::insert_kw:
            return insertTableStatement(aTokenizer);

        case Keywords::select_kw:
            return selectQueryStatement(aTokenizer);

        case Keywords::update_kw:
            return updateTableStatement(aTokenizer);

        case Keywords::delete_kw:
            return deleteStatement(aTokenizer);

        default:
            return nullptr;
    }
}
// Dispatcher to run methods for commands
StatusResult SQLProcessor::run(Statement *aStmt) {
    // running which command to run
    SQLStatement *theStatement = dynamic_cast<SQLStatement *>(aStmt);
    StatusResult  theStatus(Errors::noError);
    switch (theStatement->getType()) {
        case Keywords::create_kw:
            theStatus = createTable(theStatement);
            break;

        case Keywords::show_kw:
            theStatus = showTables();
            break;

        case Keywords::drop_kw:
            theStatus = dropTable(theStatement->getTableName());
            break;
        

        case Keywords::describe_kw: 
            theStatus = describeTable(theStatement->getTableName());
            break;
        
        
        case Keywords::insert_kw:
            theStatus = insertTable(theStatement->getTableName());
            break;
        
        case Keywords::select_kw: 
            theStatus = showQuery(theStatement->getDBQuery());
            break;

        case Keywords::update_kw:
            theStatus = updateTable(theStatement);
            break;
        
        case Keywords::delete_kw: 
            theStatus = deleteRow(theStatement);
            break;

        default:
            theStatus = StatusResult(Errors::unknownCommand);
    }

    return theStatus;
}


StatusResult SQLProcessor::deleteRow(SQLStatement *aSQLStmt){
    return (*currentActiveDbPtr)->deleteRow(aSQLStmt,output);

}

StatusResult SQLProcessor::updateTable(SQLStatement *aSQLStmt){
    return (*currentActiveDbPtr)->updateTable(aSQLStmt,output);
}

// To handle SELECT 
StatusResult SQLProcessor::showQuery(DBQuery &aDBQuery) {
    // Get vector of Rows
    Entity *theEntity = new Entity(aDBQuery.getEntityName());
    (*currentActiveDbPtr)->selectRows(aDBQuery, *theEntity, output);
    delete theEntity;
    return StatusResult(Errors::noError);
}

// To handle INSERT INTO statement
StatusResult SQLProcessor::insertTable(const std::string &aName) {
    
    Entity  *theEntity = new Entity(aName);
    Block   *theDescribeBlock = new Block(BlockType::entity_block);
    uint32_t theBlockNum = (*currentActiveDbPtr)->getEntityFromMap(aName);
    bool theCheckIfQIsEmpty = (*currentActiveDbPtr)->freeBlockQEmpty();
    uint32_t theBlockCount = (*currentActiveDbPtr)->getFreeBlock();
    (*currentActiveDbPtr)->getStorage().readBlock(theBlockNum, *theDescribeBlock);
    theEntity->decodeBlock(*theDescribeBlock);
    const Attribute *theAttr = theEntity->getPrimaryKey();
    std::string      thePrimaryKey = "";
    if (theAttr != nullptr) {
        thePrimaryKey = theAttr->getName();
    }

    delete theDescribeBlock;
    for (size_t i = 0; i < this->theRowData.size(); i++) {
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
        theRowData.at(i).tableName = aName;
        theRowData.at(i).entityId = Helpers::hashString(aName.c_str());
        theRowData.at(i).setStorageBlockNumber(theBlockCount);
        this->theRowData.at(i).getBlock(*theRowBlock);
        (*currentActiveDbPtr)->getStorage().writeBlock(theBlockCount, *theRowBlock);

        theEntity->insertDataRow(theBlockCount);
        theRowId++;
        theEntity->setAutoIncr(theRowId);
        // update blockCount
        theBlockCount = theCheckIfQIsEmpty?++theBlockCount:(*currentActiveDbPtr)->getBlockCount();
        (*currentActiveDbPtr)->setBlockCount(theBlockCount);
    }
    // Encode the entity block back
    Block theEntityBlock = theEntity->getBlock();
    (*currentActiveDbPtr)->getStorage().writeBlock(theBlockNum, theEntityBlock);

    delete theEntity;
    output << "Query Ok, " << this->theRowData.size() << " rows affected (" << Config::getTimer().elapsed() << " secs)" << std::endl;
    return StatusResult(Errors::noError);
}
// To handle CREATE TABLE
StatusResult SQLProcessor::createTable(SQLStatement *aStmt) {
    if (*currentActiveDbPtr == nullptr) {
        return StatusResult(Errors::noDatabaseSpecified);
    }
    SQLStatement *theStatement = aStmt;
    std::string theTableName = theStatement->getTableName();
    Entity  *theEntity = new Entity(theStatement->getTableName());
    uint32_t theCurrentEntityId = (*currentActiveDbPtr)->getEntityId();
    theEntity->setBlockId(theCurrentEntityId);
    std::vector<Attribute> theAttr = theStatement->getAttributevector();

    for (size_t i = 0; i < theAttr.size(); i++) {
        theEntity->addAttribute(theAttr.at(i));
    }

    // creating the table
    // Store the entity as a block
    StatusResult theStatus;
    // Checking if the table already exists
    bool theEntityExists = (*currentActiveDbPtr)->checkEntityInMap(theEntity->getName());
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
    theConvertedBlock.header.theBlockId = (*currentActiveDbPtr)->getEntityId();
    theConvertedBlock.header.theTableNameHash = Helpers::hashString(theEntity->getName().c_str());
    theEntity->setEntityHashString(theConvertedBlock.header.theTableNameHash);
    // For entity theEntityId is the current value of the auto_incr field

    theConvertedBlock.header.theEntityId = theEntity->getAutoIncr();
    bool theCheckIfQIsEmpty =  (*currentActiveDbPtr)->freeBlockQEmpty();
    uint32_t theBlockNum = (*currentActiveDbPtr)->getFreeBlock();
    theConvertedBlock.header.theBlockNum = theBlockNum;
    theEntity->setBlockNum(theBlockNum);
    (*currentActiveDbPtr)->getStorage().writeBlock(theBlockNum, theConvertedBlock);

    // Block count only incremented if no free block in between start to end of file
    uint32_t theNewBlockCount = theCheckIfQIsEmpty?theBlockNum+1:(*currentActiveDbPtr)->getBlockCount();  
    
    (*currentActiveDbPtr)->setBlockCount(theNewBlockCount);
    (*currentActiveDbPtr)->setChange(true);
    output << "Query OK, 1 row affected";
    output << " (" << Config::getTimer().elapsed() << " sec)" << std::endl;
    theStatus = StatusResult(Errors::noError);

    if (theStatus) {
        (*currentActiveDbPtr)->setEntityMap(theTableName, theBlockNum);
        uint32_t theEntityId = (*currentActiveDbPtr)->getEntityId() + 1;
        (*currentActiveDbPtr)->setEntityId(theEntityId);
    }
    delete theEntity;
    return theStatus;
   
}

// To handle DESCRIBE TABLE_NAME
StatusResult SQLProcessor::describeTable(const std::string &aName) {
    // need to write code for decoding
    Block    theDescribeBlock;
    uint32_t theBlockNum = (*currentActiveDbPtr)->getEntityFromMap(aName);
    (*currentActiveDbPtr)
        ->getStorage()
        .readBlock(theBlockNum, theDescribeBlock);

    if (theDescribeBlock.header.theTitle == aName) {
        // decode the block
        Entity *theEntity = new Entity(aName);
        theEntity->decodeBlock(theDescribeBlock);
        SQLMessageHandler.printAttrTable(output, theEntity->getAttributes());
        output << theEntity->getAttributes().size() << " rows in set ("
               << Config::getTimer().elapsed() << " sec.)" << std::endl;
        delete theEntity;
    }
    return StatusResult(Errors::noError);
}

// To handle DROP TABLE
StatusResult SQLProcessor::dropTable(const std::string &aName) {
    if (!(*currentActiveDbPtr)->checkEntityInMap(aName)) {
        return StatusResult(Errors::unknownTable);
    }
    uint32_t theBlockNum = (*currentActiveDbPtr)->getEntityFromMap(aName);
    StatusResult theStatus = (*currentActiveDbPtr)->getStorage().freeBlocks(theBlockNum);
    uint32_t theEntityHash = Helpers::hashString(aName.c_str());
    RawRowCollection theRows;
    (*currentActiveDbPtr)->each(theEntityHash,theRows);
    for(auto &r:theRows){
        uint32_t theRowBlockNum = r.getStorageBlockNumber();
        theStatus = (*currentActiveDbPtr)->getStorage().freeBlocks(theRowBlockNum);
    }
    if (theStatus) {
        (*currentActiveDbPtr)->setChange(true);
        (*currentActiveDbPtr)->removeEntityFromMap(aName);
        output << "Query OK, "<< theRows.size()+1 << " rows affected (" << Config::getTimer().elapsed()
               << " sec)"
               << "\n";
    }

    return theStatus;
}
// To handle SHOW TABLE
StatusResult SQLProcessor::showTables() {
    std::vector<std::string> theTableVector;
    std::stringstream        ss;
    ss << "Tables_in_" << (*currentActiveDbPtr)->getDbName();
    theTableVector.push_back(ss.str());
    // uint32_t theBlockNum = (*currentActiveDbPtr)->getBlockCount();
    for (auto const &imap : (*currentActiveDbPtr)->getIdxMap()) {
        theTableVector.push_back(imap.first);
    }
    size_t theLongestString = 0;
    for (size_t i = 0; i < theTableVector.size(); i++) {
        theLongestString =
            std::max(theLongestString, theTableVector.at(i).length());
    }
    SQLProcessor::SQLMessageHandler.showTableView(output, theTableVector,
                                                  theLongestString);

    
    output << theTableVector.size() - 1 << " rows in set ("
           << Config::getTimer().elapsed() << " sec.)"
           << "\n";
    
    return StatusResult(Errors::noError);
}

}  // namespace ECE141
