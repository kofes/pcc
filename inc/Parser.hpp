#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <exception>
#include <sstream>
#include "Scanner.hpp"
#include "Expr.hpp"
#include "Stmt.hpp"
#include "Sym.hpp"

struct ExprException : public std::exception {
  ExprException ( void ) : err("Illegal expression") {};
  ExprException ( const std::string& str ) : err(str) {};
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
  void parseExpr ( void );
  std::string print ( void );
private:
  void setPriorities ( void );
  Priority upPriority ( const Priority& pr );
  bool checkPriority ( const Priority& pr, const Tag& tag );

  bool isUnary ( const Tag& tag );

  void err ( const std::string& expected_token = "" );//UNEXPECTED <- EXPECTED || EOF
  void err ( const Lexeme& lexeme );

  pExpr parseExpr ( const Priority& priority );
  pExpr parseIdentifier ( Lexeme lexeme );
  pExpr parseFactor ( void );

  pStmt parseStmt ( void );

  pStmt parseIf ( void );
  pStmt parseWhile ( void );
  pStmt parseRepeat ( void );
  pStmt parseFor ( void );
  pStmt parseEmpty ( void );
  pStmt parseBlock ( void );

  void parseProgramName ( void );
  void parseConst ( void );
  void parseVar ( void );
  void parseFunction ( void );
  void parseProcedure ( void );
  void parseType ( void );

  std::vector<pExpr> parseArrayIndex ( void );

//Variables
  std::unordered_map<Tag, Priority> binaryPriority;
  std::unordered_map<Tag, Priority> unaryPriority;

  Scanner scanner;
  pNode root;//for all!(stmts and exprs)

  SymTable baseVar;
  TypeTable baseType;

  bool programTokenChecked, varTokenChecked;
};
};
