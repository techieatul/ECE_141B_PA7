#pragma once
#include <iostream>
#include <string>

#include "Attribute.hpp"
#include "DBQuery.hpp"
#include "SQLStatement.hpp"
#include "Statement.hpp"
namespace ECE141 {

struct selectField {
    selectField(std::string &aTable, std::string aField):table(aTable),field(aField){}
    ~selectField(){};
    std::string table;
    std::string field;
};
class SelectStatement : public SQLStatement {
   public:
    SelectStatement(Keywords aStatementType, Entity *anEntity);
    ~SelectStatement(){};
    StatusResult parseStatement(Tokenizer &aTokenizer);
    StatusResult parseSelect(Tokenizer &aTokenizer);
    StatusResult parseOperand(Tokenizer &aTokenizer, Operand &anOperand);

   protected:
    Entity *entity;
};

}  // namespace ECE141