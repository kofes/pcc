#pragma once

#include "Scanner.hpp"
#include "Asm.hpp"
#include <sstream>
#include <memory>
#include <exception>

namespace compiler {

enum class NodeEnum {
  Expr, Sym, Stmt
};

enum class ExprEnum {
  BinOp, UnOp, Integer, Real, Char, String, Identifier, Record, Array, Function
};

enum class SymEnum {
  Var, Function, Scalar, Array, Record, Pointer, Alias
};

enum class StmtEnum {
  If, While, Repeat, For, Empty, Assignment, Block, Procedure, Break, Continue
};

const unsigned int DEEP_STEP = 3;
const char DEEP_CHAR = ' ';

struct ExprException : public std::exception {
  ExprException ( void ) : err("Illegal expression") {};
  ExprException ( const std::string& str ) : err(str) {};
  const char* what() const noexcept {return err.c_str();};
private:
  std::string err;
};

struct Node {
  virtual std::string print ( unsigned int deep ) {
    std::ostringstream sstream;
    sstream << std::string(deep * DEEP_STEP, DEEP_CHAR) << "<ghost node>" << std::endl;
    return sstream.str();
  };
  // virtual void generate(compiler::Generator& asmGenerator);
  NodeEnum nodeType;
};

typedef std::shared_ptr<Node> pNode;

};
