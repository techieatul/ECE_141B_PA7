#pragma once
#include <iostream>
#include <string>

#include "Attribute.hpp"
#include "Statement.hpp"

namespace ECE141 {
    class DBStatement : public Statement {
        public:
        DBStatement(Keywords aStatementType);
        ~DBStatement(){};
    };
}