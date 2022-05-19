//
//  CmdProcessor.hpp
//  Database
//
//  Created by rick gessner on 3/17/19.
//  Copyright © 2019 rick gessner. All rights reserved.
//

#ifndef CmdProcessor_hpp
#define CmdProcessor_hpp

#include <stdio.h>
#include <string>

#include "Statement.hpp"
#include "Tokenizer.hpp"
using recognizeVisitor = bool (*)(ECE141::Tokenizer aTokenizer);
namespace ECE141 {


  class CmdProcessor { //processor interface
  public:
            CmdProcessor(std::ostream &anOutput);
            ~CmdProcessor();
            
    virtual CmdProcessor* recognizes(Tokenizer &aTokenizer)=0;
    virtual Statement*    makeStatement(Tokenizer &aTokenizer,
                                        StatusResult &aResult)=0;
    virtual StatusResult  run(Statement *aStmt)=0;
        
    std::ostream &output;
  };
   
}

#endif /* CmdProcessor */
