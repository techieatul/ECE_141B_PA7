#pragma once
#include "DeleteRowStatement.hpp"

#include "Helpers.hpp"

namespace ECE141 {

   StatusResult DeleteRowStatement::parse(Tokenizer &aTokenizer){
       theDBQuery.setEntity(entity);
       aTokenizer.next(); // skip where
       aTokenizer.next();
       StatusResult theStatus = theDBQuery.getFilter().parse(aTokenizer,*entity);
       return theStatus;
    }

    bool DeleteRowStatement::checkDeleteRow(Tokenizer aTokenizer){
    // DELETE FROM Users WHERE
    Token theUpdate = aTokenizer.current();
    Token theFrom = aTokenizer.peek(1);
    Token theTableName = aTokenizer.peek(2);
    Token theWhere = aTokenizer.peek(3);
    if (theUpdate.keyword != Keywords::delete_kw || theFrom.keyword != Keywords::from_kw || theTableName.type != TokenType::identifier || theWhere.keyword != Keywords::where_kw) {
        return false;
    }
    return true;
}

    

}  // namespace ECE141