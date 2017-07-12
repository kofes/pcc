#pragma once

#include "Scanner.hpp"
#include "Asm.hpp"
#include <sstream>
#include <memory>
#include <exception>

namespace compiler {

struct Node;
typedef std::shared_ptr<Node> pNode;

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
}
};
}
