#pragma once
#include <iostream>
#include "Statement.hpp"
#include "SQLStatement.hpp"
#include "SQLProcessor.hpp"
#include <string>
#include "Attribute.hpp"
namespace ECE141{
    class DropTableStatement:public SQLStatement{
        public:
        DropTableStatement(SQLProcessor* aProc,Keywords aStatementType=Keywords::unknown_kw):SQLStatement::SQLStatement(aStatementType),theSQLProcessorPtr(aProc){}
        ~DropTableStatement(){};

        // Function to check if tokenized tokens represent DROP TABLE <TABLE_NAME>;
        static bool checkDropTable(Tokenizer aTokenizer);
        
        static Statement* dropTableStatement(SQLProcessor* aProc,Tokenizer &aTokenizer);
        
        SQLProcessor* getSQLProcessor(){return theSQLProcessorPtr;}
        StatusResult parse(Tokenizer &aTokenizer);

        StatusResult run(std::ostream &aStream);

        protected:
        SQLProcessor* theSQLProcessorPtr;
    
    };

}