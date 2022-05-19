#ifndef DBQuery_hpp
#define DBQuery_hpp

#include <stdio.h>
#include <string>
#include <optional>
#include <vector>
#include "Attribute.hpp"
#include "Row.hpp"
#include "Entity.hpp"
#include "Tokenizer.hpp"
#include "Filters.hpp"
#include "keywords.hpp"
using FilterKeyword = std::vector<ECE141::Keywords>;
namespace ECE141
{
  class DBQuery
  {
  public:
    DBQuery();
    DBQuery(const DBQuery &aCopy);
    ~DBQuery();
    DBQuery &operator=(const DBQuery &aCopy);

   
    bool        getAllField() { return theAllField; };
    StringList  getAttr() { return theAttr; };
    std::string getEntityName() { return theEntityName; };
    Entity*     getEntity() { return theEntity; };
    std::string&    getOrderBy(){ return theOrderBy;}
    bool         getIsAscending(){return isAscending;}
    Filters&     getFilter(){return theFilter;}
    int          getLimit(){return theLimit;}
    FilterKeyword getFilterKey(){return theFilterKey;}
    Value& getKeyValue(const std::string &aKey);
    void getUpdateKeys(StringList &aKeyToUpdate);
    DBQuery& setAllField(bool aValue);
    DBQuery& setAttr(const std::string &aField);
    DBQuery& setAttrPrimary(const std::string &aField);
    DBQuery& setEntityName(const std::string &aName);
    DBQuery& setEntity(Entity *anEntity);
    DBQuery& setOrderBy(std::string &anOrderBy);
    DBQuery& setIsAcending(bool &anAsc);
    DBQuery& setLimit(int &aLimit);
    DBQuery& setFilterKey(const Keywords &aKey);
    DBQuery& setKeyValue(const std::string &aKey, Value &aVal);

  protected:
    std::string theEntityName;
    Entity*     theEntity;
    StringList  theAttr;
    FilterKeyword theFilterKey;
    bool        theAllField;
    std::string theOrderBy;
    bool        isAscending;
    Filters     theFilter;
    int         theLimit;
    KeyValues   theKeyValue;
  };
} // namespace ECE141


#endif /* DBQuery_hpp */