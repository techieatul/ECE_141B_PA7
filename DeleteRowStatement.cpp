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

    

}  // namespace ECE141