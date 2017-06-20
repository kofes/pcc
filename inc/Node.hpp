#pragma once

#include "Scanner.hpp"
#include <sstream>
#include <memory>


namespace compiler {

enum class NodeEnum {
  Expr, Sym
};

enum class ExprEnum {
  BinOp, UnOp, Integer, Real, Identifier, Record, Array, Function
};

enum class SymEnum {
  Var, Function, Scalar, Array, Record, Pointer, Alias
};

const unsigned int DEEP_STEP = 3;
const char DEEP_CHAR = ' ';

struct Node : public Lexeme {
  Node ( const Lexeme& lex ) : Lexeme(lex) {};
  virtual std::string print ( unsigned int deep ) {
    std::ostringstream sstream;
    sstream << std::string(deep * DEEP_STEP, DEEP_CHAR) << this->name << std::endl;
    return sstream.str();
  };
  NodeEnum nodeType;
};

typedef std::shared_ptr<Node> pNode;

};
