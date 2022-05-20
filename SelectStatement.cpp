#pragma once
#include "SelectStatement.hpp"

#include "Filters.hpp"
#include "Helpers.hpp"

namespace ECE141 {
SelectStatement::SelectStatement(Keywords aStatementType, Entity *anEntity) : SQLStatement::SQLStatement(aStatementType), entity(anEntity) {}

StatusResult SelectStatement::parseOperand(Tokenizer &aTokenizer, Operand &anOperand) {
    StatusResult theResult{noError};
    if (aTokenizer.current().type == TokenType::identifier) {
        anOperand.ttype = TokenType::identifier;
        try {
            int index = 0;

            while (theTableAttributes[index].getName() != aTokenizer.current().data) {
                index++;
            }
            anOperand.dtype = theTableAttributes[index].getType();
            anOperand.name = theTableAttributes[index].getName();

        } catch (...) {
            anOperand.ttype = TokenType::string;
            anOperand.value = aTokenizer.current().data;
        }
    } else if (aTokenizer.current().type == TokenType::number) {
        anOperand.ttype = TokenType::number;
        anOperand.value = aTokenizer.current().data;
    } else if (aTokenizer.current().type == TokenType::string) {
        anOperand.ttype = TokenType::string;
        anOperand.value = aTokenizer.current().data;
    } else {
        theResult.error = syntaxError;
    }
    return theResult;
}

StatusResult SelectStatement::parseStatement(Tokenizer &aTokenizer) {
    // First word is SELECT, so skip that
    aTokenizer.next();
    StatusResult theStatus = this->parseSelect(aTokenizer);
    if (!theStatus) {
        return StatusResult(Errors::unknownCommand);
    }

    aTokenizer.next();  // FROM
    aTokenizer.next();  // TableName
    this->theDBQuery.setEntityName(aTokenizer.current().data);
    aTokenizer.next();
    //Filters thefilters;
    while (aTokenizer.more() && aTokenizer.current().data != ";") {
        switch (aTokenizer.current().keyword) {
            case Keywords::where_kw:
                this->theDBQuery.setFilterKey(Keywords::where_kw);
                

                // Skip where
                aTokenizer.next();
                theStatus = this->theDBQuery.getFilter().parse(aTokenizer, *entity);
                if (!theStatus) {
                    return StatusResult(theStatus.error);
                }
                break;

            case Keywords::order_kw:
                this->theDBQuery.setFilterKey(Keywords::order_kw);
                aTokenizer.next();  // by
                if (aTokenizer.current().type == TokenType::keyword && aTokenizer.current().keyword == Keywords::by_kw) {
                    aTokenizer.next();  // should be fields
                    this->theDBQuery.setOrderBy(aTokenizer.current().data);
                    aTokenizer.next();
                } else {
                    return StatusResult(Errors::syntaxError);
                }
                break;
            case Keywords::limit_kw:
                this->theDBQuery.setFilterKey(Keywords::limit_kw);
                aTokenizer.next();
                if (aTokenizer.current().type == TokenType::number) {
                    int thelimit = std::stoi(aTokenizer.current().data);
                    this->theDBQuery.setLimit(thelimit);
                    aTokenizer.next();
                } else {
                    return StatusResult(Errors::syntaxError);
                }
                break;
            default:
                break;
        }
    }
    return StatusResult(Errors::noError);
}

StatusResult SelectStatement::parseSelect(Tokenizer &aTokenizer) {
    // Means we have *
    if (aTokenizer.current().data == "*") {
        this->theDBQuery.setAllField(true);
        return StatusResult(Errors::noError);
    }

    // Means we have atleast one field
    while (aTokenizer.peek(1).keyword != Keywords::from_kw) {
        this->theDBQuery.setAttr(aTokenizer.current().data);
        aTokenizer.next(2);
    }
    // To push the last field
    this->theDBQuery.setAttr(aTokenizer.current().data);

    return StatusResult(Errors::noError);
}

// Function to check if tokenized tokens represent SELECT...;
bool SelectStatement::checkSelectTable(Tokenizer aTokenizer) {
    Token theSelectToken{TokenType::keyword, Keywords::select_kw,
                         Operators::unknown_op, "select"};
    Token theFromToken{TokenType::keyword, Keywords::from_kw,
                       Operators::unknown_op, "from"};
    Token theTableToken{TokenType::identifier, Keywords::unknown_kw,
                        Operators::unknown_op, "table_name"};

    std::vector<Token> SQLVector;
    SQLVector.push_back(theSelectToken);
    SQLVector.push_back(theFromToken);
    SQLVector.push_back(theTableToken);
    // It has to be atleast SELECT * FROM Table_name
    if (aTokenizer.size() < 4) {
        return false;
    }
    bool checkSelect = false;

    for (size_t i = 0; i < SQLVector.size(); ++i) {
        if ((SQLVector.at(i).keyword != aTokenizer.current().keyword) ||
            (SQLVector.at(i).type != aTokenizer.current().type)) {
            return false;
        }

        if (!checkSelect && aTokenizer.peek(1).data == "*") {
            checkSelect = true;
            aTokenizer.next();
        } else {
            // check if we have like this identifier_kw,identifier_kw,identifier_kw
            if (!checkSelect) {
                while (!checkSelect && aTokenizer.more() && aTokenizer.peek(2).data != "from") {
                    aTokenizer.next();
                    if (aTokenizer.current().type != TokenType::identifier) {
                        return false;
                    }
                    aTokenizer.skipTo(',');
                }
                checkSelect = true;
                aTokenizer.next();
            }
        }

        aTokenizer.next();
    }

    return true;
}

} // namespace ECE141


