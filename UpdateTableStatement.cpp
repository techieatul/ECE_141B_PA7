#pragma once
#include "UpdateTableStatement.hpp"

#include "Helpers.hpp"

namespace ECE141 {

   StatusResult UpdateTableStatement::parse(Tokenizer &aTokenizer){
       theDBQuery.setEntity(entity);
       aTokenizer.skipTo(Keywords::set_kw);
       aTokenizer.next();
       StatusResult theStatus = parseSet(aTokenizer);
       if(!theStatus){
           return theStatus;
       }
       // parse where
       if(aTokenizer.current().keyword==Keywords::where_kw){
           aTokenizer.next(); // skip where
           theStatus = theDBQuery.getFilter().parse(aTokenizer,*entity);
       }

       if(!theStatus){
           return theStatus;
       }

       return StatusResult(Errors::noError);

    }

    StatusResult UpdateTableStatement::parseSet(Tokenizer &aTokenizer){
       
       while(aTokenizer.current().keyword!=Keywords::where_kw && aTokenizer.current().data!=";"){
           Token theKey = aTokenizer.current();
           Token theOpr = aTokenizer.peek(1);
           Token theData = aTokenizer.peek(2);
           aTokenizer.next(3);

           const Attribute* theAttr = entity->getAttribute(theKey.data);
           if(theAttr==nullptr){
               return StatusResult(Errors::unknownAttribute);
           }

           DataTypes theAttrType = theAttr->getType();
           Value theVal;
           switch (theAttrType){
               case DataTypes::int_type:
                    theVal = Helpers::convertStrToInt(theData.data);
                    break;

               case DataTypes::float_type:
                    theVal = Helpers::convertStrToDouble(theData.data);
                    break;

               case DataTypes::varchar_type:
                    theVal = std::string(theData.data);
                    break;

               case DataTypes::bool_type:
                    theVal = Helpers::convertStrToBool(theData.data);
                    break;
           }
           
           this->theDBQuery.setKeyValue(theKey.data,theVal);
           if(aTokenizer.current().data==","){
               aTokenizer.next();
           }
       }
       return StatusResult(Errors::noError);

    }

    bool UpdateTableStatement::checkUpdateTable(Tokenizer aTokenizer){
        Token theUpdate = aTokenizer.current();
        Token theTableName = aTokenizer.peek(1);
        Token theSetToken = aTokenizer.peek(2);
        if (theUpdate.keyword != Keywords::update_kw || theTableName.type != TokenType::identifier || theSetToken.keyword != Keywords::set_kw) {
            return false;
        }
        aTokenizer.next(3);
        // Now check if we have identifier = data,identifier = data ....
        while (aTokenizer.current().type != TokenType::punctuation && aTokenizer.current().keyword != Keywords::where_kw) {
            Token theIdentifier = aTokenizer.current();
            Token theEqualOpr = aTokenizer.peek(1);
            Token theData = aTokenizer.peek(2);
            if (theIdentifier.type != TokenType::identifier || theEqualOpr.data != "=" || (theData.type != TokenType::number && theData.type != TokenType::string && theData.type != TokenType::timedate)) {
                return false;
            }
            aTokenizer.next(3);
            if (aTokenizer.current().data == ",") {
                aTokenizer.next();
            }
        }
        return true;
    }

}  // namespace ECE141