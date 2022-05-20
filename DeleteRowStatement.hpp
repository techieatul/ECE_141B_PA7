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
class DeleteRowStatement : public SQLStatement {
   public:
    DeleteRowStatement(Keywords aStatementType,Entity *anEntity)
        : SQLStatement::SQLStatement(aStatementType),entity(anEntity){}
    ~DeleteRowStatement(){};

    StatusResult parse(Tokenizer &aTokenizer) override;
    static bool checkDeleteRow(Tokenizer aTokenizer);
    
   
   protected:
    
    Entity     *entity;
};

}  // namespace ECE141