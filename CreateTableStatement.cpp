#include "CreateTableStatement.hpp"
#include "Database.hpp"

namespace ECE141 {

    bool CreateTableStatement::checkCreateTable(Tokenizer aTokenizer) {
        Token theCreateToken{TokenType::keyword, Keywords::create_kw,
                             Operators::unknown_op, "create"};
        Token theTableToken{TokenType::keyword, Keywords::table_kw,
                            Operators::unknown_op, "table"};
        Token theTableNameToken{TokenType::identifier, Keywords::unknown_kw,
                                Operators::unknown_op, "table_name"};
        Token theOpenParanToken{TokenType::punctuation, Keywords::unknown_kw,
                                Operators::unknown_op, "("};

        std::vector<Token> SQLVector;
        SQLVector.push_back(theCreateToken);
        SQLVector.push_back(theTableToken);
        SQLVector.push_back(theTableNameToken);
        SQLVector.push_back(theOpenParanToken);

        for (size_t i = 0; i < SQLVector.size(); ++i) {
            if ((SQLVector.at(i).keyword != aTokenizer.current().keyword) ||
                (SQLVector.at(i).type != aTokenizer.current().type)) {
                return false;
            }
            aTokenizer.next();
        }

        // check if we have ) and ;
        aTokenizer.skipTo(';');

        if (aTokenizer.tokenAt(aTokenizer.getIndex() - 1).data[0] != ')' ||
            aTokenizer.current().data[0] != ';') {
            return false;
        }

        return true;
    }

    Statement* CreateTableStatement::createTableStatement(SQLProcessor* aProc, Tokenizer& aTokenizer){
        CreateTableStatement *theCreateTable = new CreateTableStatement(aProc,Keywords::create_kw);
        StatusResult theStatus = theCreateTable->parse(aTokenizer);
        if(theStatus){
            return theCreateTable;
        }

        return nullptr;

    }

    StatusResult CreateTableStatement::run(std::ostream &aStream) {
        SQLProcessor* theSQLProcessorPtr = getSQLProcessor();
        Database* theDatabase = theSQLProcessorPtr->getDatabaseInUse();
        theDatabase->createTable(this,aStream);

    }

    SQLProcessor* CreateTableStatement::getSQLProcessor(){return theSQLProcessorPtr;}
};