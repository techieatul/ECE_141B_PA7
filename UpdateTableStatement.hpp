#pragma once
#include <iostream>
#include <string>

#include "Attribute.hpp"
#include "Entity.hpp"
#include "Row.hpp"
#include "SQLProcessor.hpp"
#include "SQLStatement.hpp"
#include "Statement.hpp"

namespace ECE141 {
class UpdateTableStatement : public SQLStatement {
   public:
    UpdateTableStatement(Keywords aStatementType,Entity *anEntity)
        : SQLStatement::SQLStatement(aStatementType),entity(anEntity){}
    ~UpdateTableStatement(){};

    StatusResult parse(Tokenizer &aTokenizer) override;
    StatusResult parseSet(Tokenizer &aTokenizer);
    static bool checkUpdateTable(Tokenizer aTokenizer);
   
   protected:
    
    Entity     *entity;
};

}  // namespace ECE141