#pragma once

#include "Scanner.hpp"
#include <sstream>
#include <memory>



namespace compiler {

const unsigned int DEEP_STEP = 3;
const char DEEP_CHAR = ' ';

struct Node : public Lexeme {
  Node ( const Lexeme& lex ) : Lexeme(lex) {};
  virtual std::string print ( unsigned int deep ) {
    std::ostringstream sstream;
    sstream << std::string(deep * DEEP_STEP, DEEP_CHAR) << this->name << std::endl;
    return sstream.str();
  };
};

typedef std::shared_ptr<Node> pNode;

};
