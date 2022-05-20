#pragma once
#include <iostream>
#include "Statement.hpp"
#include "SQLStatement.hpp"
#include <string>
#include "Attribute.hpp"
namespace ECE141{
    class DropTableStatement:public SQLStatement{
        public:
        DropTableStatement(Keywords aStatementType=Keywords::unknown_kw):SQLStatement::SQLStatement(aStatementType){}
        ~DropTableStatement(){};

        // Function to check if tokenized tokens represent DROP TABLE <TABLE_NAME>;
        static bool checkDropTable(Tokenizer aTokenizer) {
            Token theDropToken{TokenType::keyword, Keywords::drop_kw,
                               Operators::unknown_op, "drop"};
            Token theTableToken{TokenType::keyword, Keywords::table_kw,
                                Operators::unknown_op, "table"};
            Token theTableNameToken{TokenType::identifier, Keywords::unknown_kw,
                                    Operators::unknown_op, "table_name"};
            Token theSemiColonToken{TokenType::punctuation, Keywords::unknown_kw,
                                    Operators::unknown_op, ";"};

            std::vector<Token> SQLVector;
            SQLVector.push_back(theDropToken);
            SQLVector.push_back(theTableToken);
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
        
        static Statement* dropTableStatement(SQLProcessor* aProc,Tokenizer &aTokenizer){
            DropTableStatement *theDropTable = new DropTableStatement(Keywords::drop_kw);
            if(theDropTable->parse(aTokenizer)){return theDropTable;}
            return nullptr;

        }

        StatusResult parse(Tokenizer &aTokenizer){
            aTokenizer.skipTo(TokenType::identifier);
            this->setTableName(aTokenizer.current().data);
            return StatusResult(Errors::noError);

        } 

    
    
    };

}