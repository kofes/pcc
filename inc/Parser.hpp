#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <exception>
#include <sstream>
#include "Scanner.hpp"
#include "Expr.hpp"

struct ExprException : public std::exception {
  ExprException() : err("Illegal expression") {};
  ExprException(const std::string& str) : err(str) {};
  const char* what() const noexcept {return err.c_str();};
private:
  std::string err;
};

namespace compiler {

enum class Priority : unsigned short {
  HIGHEST = 0,
  SECOND = 1,
  THIRD = 2,
  LOWEST = 3
};

class Parser {
public:
  Parser ( void );
  Parser ( const std::string& filename );
  void set ( const std::string& filename );
  void parse ( void );
  std::string print ( void );
private:
  void setPriorities ( void );
  compiler::Priority upPriority ( const compiler::Priority& pr );
  bool checkPriority ( const compiler::Priority& pr, const compiler::Tag& tag );

  bool isUnary ( const compiler::Tag& tag );

  void checkParenthesis ( const compiler::Lexeme& lex );

  compiler::pExpr parseExpr ( const compiler::Priority& priority );
  compiler::pExpr parseTerm ( void );
  compiler::pExpr parseFactor ( void );

  std::vector<compiler::pExpr> parseArrayIndex ( void );

  std::unordered_map<compiler::Tag, compiler::Priority> binaryPriority;
  std::unordered_map<compiler::Tag, compiler::Priority> unaryPriority;

  compiler::Scanner scanner;
  compiler::pExpr root;

  long long int diffBracket = 0, diffBrace = 0;
};
};
