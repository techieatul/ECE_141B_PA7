//
//  DBProcessor.cpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//

#include "DBProcessor.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

#include "Application.hpp"
#include "Config.hpp"
#include "DBProcessor.hpp"
#include "Database.hpp"
#include "FolderReader.hpp"
#include "FolderView.hpp"
#include "Helpers.hpp"
#include "ShowsTablesView.hpp"
#include "DBStatement.hpp"

namespace ECE141 {

// like a typedef!!!
using StmtFactory = Statement *(*)(DBProcessor *aProc,
                                   Tokenizer   &aTokenize);

DBProcessor::DBProcessor(std::ostream &anOutput, Database **aCurrentActiveDbPtr)
    : CmdProcessor(anOutput), DBMessageHandler(), currentActiveDbPtr(aCurrentActiveDbPtr) {
}

DBProcessor::~DBProcessor() {
}

// Function to check if tokenized tokens represent CREATE DATABASE DB_NAME;
bool checkCreateDB(Tokenizer aTokenizer) {
    Token              theCreateToken{TokenType::keyword, Keywords::create_kw, Operators::unknown_op, "create"};
    Token              theDBToken{TokenType::keyword, Keywords::database_kw, Operators::unknown_op, "database"};
    Token              theDBNameToken{TokenType::identifier, Keywords::unknown_kw, Operators::unknown_op, "DB_NAME"};
    Token              theSemiColonToken{TokenType::punctuation, Keywords::unknown_kw, Operators::unknown_op, ";"};
    std::vector<Token> DBVector;
    DBVector.push_back(theCreateToken);
    DBVector.push_back(theDBToken);
    DBVector.push_back(theDBNameToken);
    DBVector.push_back(theSemiColonToken);
    size_t theVecSize = DBVector.size();

    size_t theCount = 0;
    // Lambda function to count tokens till ;
    std::function<void()> theCountTillSemi = [&theCount, &aTokenizer]() {
        size_t theCurrIdx = aTokenizer.getIndex();
        while (aTokenizer.more() && aTokenizer.tokenAt(theCurrIdx).data != ";") {
            theCount++;
            theCurrIdx++;
        }
    };

    // Calling the function
    theCountTillSemi();

    // If size unequal then return false
    if (theVecSize != theCount + 1) {
        return false;
    }
    // bool theReturnVal = true;
    for (size_t i = 0; i < theVecSize; ++i) {
        if ((DBVector.at(i).keyword != aTokenizer.current().keyword) || (DBVector.at(i).type != aTokenizer.current().type)) {
            return false;
        }
        aTokenizer.next();
    }

    return true;
}

// Function to check if tokenized tokens represent SHOW DATABASE;
bool checkShowDB(Tokenizer aTokenizer) {
    Token              theShowToken{TokenType::keyword, Keywords::show_kw, Operators::unknown_op, "show"};
    Token              theDBToken{TokenType::keyword, Keywords::databases_kw, Operators::unknown_op, "databases"};
    std::vector<Token> theTokenVector;
    theTokenVector.push_back(theShowToken);
    theTokenVector.push_back(theDBToken);
    size_t theVecSize = theTokenVector.size();

    size_t theCount = 0;
    // Lambda function to count tokens till ;
    std::function<void()> theCountTillSemi = [&theCount, &aTokenizer]() {
        size_t theCurrIdx = aTokenizer.getIndex();
        while (theCurrIdx<aTokenizer.size() && aTokenizer.tokenAt(theCurrIdx).data != ";") {
            theCount++;
            theCurrIdx++;
        }
    };

    // Calling the function
    theCountTillSemi();

    // If size unequal then return false
    if (theVecSize != theCount) {
        return false;
    }

    // bool theReturnVal = true;
    for (size_t i = 0; i < theVecSize; ++i) {
        if ((theTokenVector.at(i).keyword != aTokenizer.current().keyword) || (theTokenVector.at(i).type != aTokenizer.current().type)) {
            return false;
        }
        aTokenizer.next();
    }

    return true;
}

// Function to check if tokenized tokens represent USE DB_NAME;
bool checkUseDB(Tokenizer aTokenizer) {
    Token              theUseToken{TokenType::keyword, Keywords::use_kw, Operators::unknown_op, "use"};
    Token              theDBNameToken{TokenType::identifier, Keywords::unknown_kw, Operators::unknown_op, "DB_NAME"};
    Token              theSemiColonToken{TokenType::punctuation, Keywords::unknown_kw, Operators::unknown_op, ";"};
    std::vector<Token> DBVector;
    DBVector.push_back(theUseToken);
    DBVector.push_back(theDBNameToken);
    DBVector.push_back(theSemiColonToken);
    size_t theVecSize = DBVector.size();

    size_t theCount = 0;
    // Lambda function to count tokens till ;
    std::function<void()> theCountTillSemi = [&theCount, &aTokenizer]() {
        size_t theCurrIdx = aTokenizer.getIndex();
        while (aTokenizer.more() && aTokenizer.tokenAt(theCurrIdx).data != ";") {
            theCount++;
            theCurrIdx++;
        }
    };

    // Calling the function
    theCountTillSemi();

    // If size unequal then return false
    if (theVecSize != theCount + 1) {
        return false;
    }

    // bool theReturnVal = true;
    for (size_t i = 0; i < theVecSize; ++i) {
        if ((DBVector.at(i).keyword != aTokenizer.current().keyword) || (DBVector.at(i).type != aTokenizer.current().type)) {
            return false;
        }
        aTokenizer.next();
    }

    return true;
}

// Function to check if tokenized tokens represent DROP DATABASE DB_NAME;
bool checkDropDB(Tokenizer aTokenizer) {
    Token              theDropToken{TokenType::keyword, Keywords::drop_kw, Operators::unknown_op, "drop"};
    Token              theDBToken{TokenType::keyword, Keywords::database_kw, Operators::unknown_op, "database"};
    Token              theDBNameToken{TokenType::identifier, Keywords::unknown_kw, Operators::unknown_op, "DB_NAME"};
    Token              theSemiColonToken{TokenType::punctuation, Keywords::unknown_kw, Operators::unknown_op, ";"};
    std::vector<Token> DBVector;
    DBVector.push_back(theDropToken);
    DBVector.push_back(theDBToken);
    DBVector.push_back(theDBNameToken);
    DBVector.push_back(theSemiColonToken);
    size_t theVecSize = DBVector.size();

    size_t theCount = 0;
    // Lambda function to count tokens till ;
    std::function<void()> theCountTillSemi = [&theCount, &aTokenizer]() {
        size_t theCurrIdx = aTokenizer.getIndex();
        while (aTokenizer.more() && aTokenizer.tokenAt(theCurrIdx).data != ";") {
            theCount++;
            theCurrIdx++;
        }
    };

    // Calling the function
    theCountTillSemi();

    // If size unequal then return false
    if (theVecSize != theCount + 1) {
        return false;
    }

    // bool theReturnVal = true;
    for (size_t i = 0; i < theVecSize; ++i) {
        if ((DBVector.at(i).keyword != aTokenizer.current().keyword) || (DBVector.at(i).type != aTokenizer.current().type)) {
            return false;
        }
        aTokenizer.next();
    }

    return true;
}

// Function to check if given input matches DUMp DATABASE <DBNAME>
bool checkDumpDB(Tokenizer aTokenizer) {
    Token              theDumpToken{TokenType::keyword, Keywords::dump_kw, Operators::unknown_op, "dump"};
    Token              theDBToken{TokenType::keyword, Keywords::database_kw, Operators::unknown_op, "database"};
    Token              theDBNameToken{TokenType::identifier, Keywords::unknown_kw, Operators::unknown_op, "DB_NAME"};
    Token              theSemiColonToken{TokenType::punctuation, Keywords::unknown_kw, Operators::unknown_op, ";"};
    std::vector<Token> DBVector;
    DBVector.push_back(theDumpToken);
    DBVector.push_back(theDBToken);
    DBVector.push_back(theDBNameToken);
    DBVector.push_back(theSemiColonToken);
    size_t theVecSize = DBVector.size();

    size_t theCount = 0;
    // Lambda function to count tokens till ;
    std::function<void()> theCountTillSemi = [&theCount, &aTokenizer]() {
        size_t theCurrIdx = aTokenizer.getIndex();
        while (aTokenizer.more() && aTokenizer.tokenAt(theCurrIdx).data != ";") {
            theCount++;
            theCurrIdx++;
        }
    };

    // Calling the function
    theCountTillSemi();

    // If size unequal then return false
    if (theVecSize != theCount + 1) {
        return false;
    }
    // bool theReturnVal = true;
    for (size_t i = 0; i < theVecSize; ++i) {
        if ((DBVector.at(i).keyword != aTokenizer.current().keyword) || (DBVector.at(i).type != aTokenizer.current().type)) {
            return false;
        }
        aTokenizer.next();
    }

    return true;
}


// To dispact respectivve recognize function
std::map<Keywords,recognizeVisitor> theRecognizeMap{
    {Keywords::create_kw,checkCreateDB},
    {Keywords::show_kw,checkShowDB},
    {Keywords::use_kw,checkUseDB},
    {Keywords::drop_kw,checkDropDB},
    {Keywords::dump_kw,checkDumpDB},
  };

// CmdProcessor interface ...
CmdProcessor *DBProcessor::recognizes(Tokenizer &aTokenizer) {
    // Code to recognise DB commands
    Keywords theKw = aTokenizer.current().keyword;
    if(theRecognizeMap.find(theKw)!=theRecognizeMap.end()){
        if(theRecognizeMap[theKw](aTokenizer)){
            DBProcessor::keywordStatement = theKw;
            return this;
        }
    }
    return nullptr;
}
// Function to run the main logic of CREATE DATABASE <DBNAME>
StatusResult DBProcessor::createDatabase(Statement *aStmt){
    if (dbExists(aStmt->getDBName())) {
        return StatusResult(Errors::databaseExists);
    }
    const std::string theName = aStmt->getDBName();
    delete (*currentActiveDbPtr);
    (*currentActiveDbPtr) = new Database(theName, CreateDB{});
    output << "Query OK, 1 rows affected";
    output << " (" << Config::getTimer().elapsed() << " secs)" << std::endl;
    return StatusResult(Errors::noError);

}
// Function to run the main logic of SHOW DATABASES
StatusResult DBProcessor::showDatabases(Statement *aStmt){
    std::vector<std::string> theDBVector;
    theDBVector.push_back("Database");
    size_t       theLongestString = theDBVector.at(0).length();
    std::string  thePath = Config::getStoragePath();
    FolderReader theFr(thePath.c_str());
    theFr.each(".db", [&theDBVector, &theLongestString](const std::string &aName) {
        theDBVector.push_back(aName);
        theLongestString = std::max(theLongestString, aName.length());
        return true;
    });
    theLongestString += 10;
    DBProcessor::DBMessageHandler.showTableView(output, theDBVector, theLongestString);

    std::stringstream theMessage;
    theMessage << theDBVector.size() - 1 << " rows in set";  
    std::string theMsg = std::string(theMessage.str());
    DBProcessor::DBMessageHandler.setMessage(theMsg);
    DBProcessor::DBMessageHandler.show(output);
    return StatusResult(Errors::noError);


}
// Function to run the main logic of USE <DBNAME>
StatusResult DBProcessor::useDatabase(Statement *aStmt){
    if(!dbExists(aStmt->getDBName())){
        return StatusResult(Errors::unknownDatabase);

    }
    delete (*DBProcessor::currentActiveDbPtr);
    const std::string theName = aStmt->getDBName();
    (*DBProcessor::currentActiveDbPtr) = new Database(theName, OpenDB{});
    output << "Database changed" << std::endl;
    return StatusResult{ECE141::noError};
}

// Function to run the main logic of DROP DATABASE <DBNAME>
StatusResult DBProcessor::dropDatabase(Statement *aStmt){
    if(!dbExists(aStmt->getDBName())){
        return StatusResult(Errors::unknownDatabase);
    }
    std::filesystem::remove(Config::getDBPath(aStmt->getDBName()));

    // Delete the activeDB if it is the one to be dropped
    if ((*currentActiveDbPtr) != nullptr && (*currentActiveDbPtr)->getDbName() == aStmt->getDBName()) {
        delete (*currentActiveDbPtr);
        (*currentActiveDbPtr) = nullptr;
    }
    // Atul added to take use of Views class
    std::string theMessage = "Query OK, 0 rows affected";
    DBProcessor::DBMessageHandler.setMessage(theMessage);
    DBProcessor::DBMessageHandler.show(output);
    return StatusResult(Errors::noError);


}

// Function to run the main logic of DUMP DATABASE <DBNAME>
StatusResult DBProcessor::dumpDatabase(Statement *aStmt){
    const std::string theName = aStmt->getDBName();
    std::fstream theDumpStream;
    std::string  theDbPath = Config::getDBPath(theName);
    theDumpStream.open(theDbPath, std::ios::in);
    theDumpStream.seekg(0, std::ios::end);
    size_t theFileSize = theDumpStream.tellg();
    size_t theNumBlocks = theFileSize / ECE141::kBlockSize;
    theDumpStream.seekg(0, std::ios::beg);
    Block theHeader;
    // Table View class
    ShowsTablesView theTb(true);
    theTb.addToRow("Type");
    theTb.addToRow("id");
    theTb.addToRow("Extra");
    theTb.endOfRow();

    for (size_t i = 0; i < theNumBlocks; i++) {
        theDumpStream.read(reinterpret_cast<char *>(&theHeader), ECE141::kBlockSize);
        char   c = theHeader.header.type;
        size_t id = theHeader.header.theBlockId;
        std::string theName = std::string(theHeader.header.theTitle);
        switch (c) {
            case 'M':
                theTb.addToRow("Meta");
                break;
            case 'E':
                theTb.addToRow("Entity");
                break;
            case 'D':
                theTb.addToRow("Data");
                break;
            case 'F':
                theTb.addToRow("Free");
                break;
        }
        theTb.addToRow(std::to_string(id));
        if(c=='D' || c == 'E'){
            theTb.addToRow(theName);
        }else{
            theTb.addToRow("");
        }
        
        theTb.endOfRow();
        theDumpStream.seekg((i + 1) * ECE141::kBlockSize, std::ios::beg);
    }

    theDumpStream.close();
    theTb.show(output);
    output << theNumBlocks << " rows in set (" << Config::getTimer().elapsed() << " sec)\n";

    return StatusResult(Errors::noError);

}
// Function to dispatch the run logic of each command
StatusResult DBProcessor::run(Statement *aStmt) {
    
    switch (aStmt->getType()){
        case Keywords::create_kw:
             return createDatabase(aStmt);
        case Keywords::show_kw:
             return showDatabases(aStmt);
        case Keywords::drop_kw:
             return dropDatabase(aStmt);
        case Keywords::dump_kw:
             return dumpDatabase(aStmt);
        case Keywords::use_kw:
             return useDatabase(aStmt);
        default:
             return StatusResult(Errors::unknownCommand);
        
    } 
}
// USE: Get DB names and do other stuff based on Statement
void handleDB(Tokenizer &aTokenizer, Statement *aDBStatement) {
    switch (aDBStatement->getType()) {
        case Keywords::create_kw:
            aTokenizer.skipTo(TokenType::identifier);
            aDBStatement->setDBName(aTokenizer.current().data);
            break;
        case Keywords::show_kw:
            break;
        case Keywords::drop_kw:
            aTokenizer.skipTo(TokenType::identifier);
            aDBStatement->setDBName(aTokenizer.current().data);
            break;
        case Keywords::use_kw:
            aTokenizer.skipTo(TokenType::identifier);
            aDBStatement->setDBName(aTokenizer.current().data);
            break;
        case Keywords::dump_kw:
            aTokenizer.skipTo(TokenType::identifier);
            aDBStatement->setDBName(aTokenizer.current().data);
            break;
    }
    aTokenizer.skipTo(TokenType::punctuation);
    aTokenizer.next();
}
// USE: retrieve a statement based on given text input...
Statement *DBProcessor::makeStatement(Tokenizer    &aTokenizer,
                                      StatusResult &aResult) {
    DBStatement *theDBStatement = new DBStatement(DBProcessor::keywordStatement);
    handleDB(aTokenizer, theDBStatement);
    DBProcessor::keywordStatement = Keywords::unknown_kw;
    return theDBStatement;

}

Database *DBProcessor::getDatabaseInUse() {
    return *currentActiveDbPtr;
}
// Check if DB already exists
bool DBProcessor::dbExists(const std::string &aDBName) {
    // Atul added
    std::string theDBPath = Config::getDBPath(aDBName);
    return std::filesystem::exists(theDBPath);
}

}  // namespace ECE141
