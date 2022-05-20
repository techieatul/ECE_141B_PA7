#pragma once
#include <iostream>
#include "Statement.hpp"
#include "SQLStatement.hpp"
#include <string>
#include "Attribute.hpp"
namespace ECE141{
    class DescribeTableStatement:public SQLStatement{
        public:
        DescribeTableStatement(Keywords aStatementType=Keywords::unknown_kw):SQLStatement::SQLStatement(aStatementType){}
        ~DescribeTableStatement(){};
        // Function to check if tokenized tokens represent DESCRIBE <TABLE_NAME>;
        static bool checkDescribeTable(Tokenizer aTokenizer) {
            Token theDescribeToken{TokenType::keyword, Keywords::describe_kw,
                                   Operators::unknown_op, "describe"};
            Token theTableNameToken{TokenType::identifier, Keywords::unknown_kw,
                                    Operators::unknown_op, "table_name"};
            Token theSemiColonToken{TokenType::punctuation, Keywords::unknown_kw,
                                    Operators::unknown_op, ";"};

            std::vector<Token> SQLVector;
            SQLVector.push_back(theDescribeToken);
            SQLVector.push_back(theTableNameToken);
            SQLVector.push_back(theSemiColonToken);

            for (size_t i = 0; i < SQLVector.size(); ++i) {
                if ((SQLVector.at(i).keyword != aTokenizer.current().keyword) ||
                    (SQLVector.at(i).type != aTokenizer.current().type)) {
                    return false;
                }
                aTokenizer.next();
            }

            return true;
        }
        
        static Statement* describeTableStatement(SQLProcessor* aProc, Tokenizer &aTokenizer){
            DescribeTableStatement *theDescribeTable = new DescribeTableStatement(Keywords::describe_kw);
            aTokenizer.skipTo(TokenType::identifier);
            theDescribeTable->setTableName(aTokenizer.current().data);
            return theDescribeTable;

        }

    };

}