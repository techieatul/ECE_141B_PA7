#pragma once
#include <iostream>
#include "Statement.hpp"
#include "SQLStatement.hpp"
#include "SQLProcessor.hpp"
#include "Database.hpp"
#include <string>
#include "Attribute.hpp"
namespace ECE141{
    class DescribeTableStatement:public SQLStatement{
        public:
        DescribeTableStatement(SQLProcessor* aProc,Keywords aStatementType=Keywords::unknown_kw):SQLStatement::SQLStatement(aStatementType),theSQLProcessorPtr(aProc){}
        ~DescribeTableStatement(){};
        // Function to check if tokenized tokens represent DESCRIBE <TABLE_NAME>;
        static bool checkDescribeTable(Tokenizer aTokenizer);
        
        static Statement* describeTableStatement(SQLProcessor* aProc, Tokenizer &aTokenizer);
        SQLProcessor* getSQLProcessor(){return theSQLProcessorPtr;}

        StatusResult run(std::ostream &aStream);

        protected:
        SQLProcessor* theSQLProcessorPtr;
    };

}