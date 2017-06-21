#pragma once

#include "Scanner.hpp"
#include <sstream>
#include <memory>


namespace compiler {

enum class NodeEnum {
  Expr, Sym, Stmt
};

enum class ExprEnum {
  BinOp, UnOp, Integer, Real, Identifier, Record, Array, Function
};

enum class SymEnum {
  Var, Function, Scalar, Array, Record, Pointer, Alias
};

enum class StmtEnum {
  If, While, Repeat, For, Empty, Assignment, Block, Procedure
};

const unsigned int DEEP_STEP = 3;
const char DEEP_CHAR = ' ';

struct Node {
  virtual std::string print ( unsigned int deep ) {
    std::ostringstream sstream;
    sstream << std::string(deep * DEEP_STEP, DEEP_CHAR) << "<ghost node>" << std::endl;
    return sstream.str();
  };
  NodeEnum nodeType;
};

typedef std::shared_ptr<Node> pNode;

};
