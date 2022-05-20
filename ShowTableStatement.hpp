#pragma once
#include <iostream>
#include "Statement.hpp"
#include "SQLStatement.hpp"
#include <string>
#include "Attribute.hpp"
namespace ECE141{
    class ShowTableStatement:public SQLStatement{
        public:
        ShowTableStatement(Keywords aStatementType=Keywords::unknown_kw):SQLStatement::SQLStatement(aStatementType){}
        ~ShowTableStatement(){};
        // Function to check if tokenized tokens represent SHOW TABLES;
        static bool checkShowTable(Tokenizer aTokenizer) {
            Token theShowToken{TokenType::keyword, Keywords::show_kw,
                               Operators::unknown_op, "show"};
            Token theTablesToken{TokenType::keyword, Keywords::tables_kw,
                                 Operators::unknown_op, "tables"};

            std::vector<Token> SQLVector;
            SQLVector.push_back(theShowToken);
            SQLVector.push_back(theTablesToken);

            for (size_t i = 0; i < SQLVector.size(); ++i) {
                if ((SQLVector.at(i).keyword != aTokenizer.current().keyword) ||
                    (SQLVector.at(i).type != aTokenizer.current().type)) {
                    return false;
                }
                aTokenizer.next();
            }

            return true;
        }

        static Statement* showTableStatement(SQLProcessor* aProc, Tokenizer &aTokenizer){
            ShowTableStatement *theShowTable = new ShowTableStatement(Keywords::show_kw);
            return theShowTable;

        }
    };

}