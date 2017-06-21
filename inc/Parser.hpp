#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <exception>
#include <sstream>
#include <algorithm>
#include <tuple>

#include "Scanner.hpp"
#include "Expr.hpp"
#include "Stmt.hpp"
#include "Sym.hpp"

#include "CompileTimeFunctions.hpp"

struct ExprException : public std::exception {
  ExprException ( void ) : err("Illegal expression") {};
  ExprException ( const std::string& str ) : err(str) {};
  const char* what() const noexcept {return err.c_str();};
private:
  std::string err;
};

namespace compiler {
//ParseExpr-enum
enum class Priority : unsigned short {
  HIGHEST = 0,
  SECOND = 1,
  THIRD = 2,
  LOWEST = 3
};

enum class InitExpected {
  NO,
  YES,
  MAYBE
};

enum class IdentifierType {
  VARIABLE, FUNCTION, TYPE
};

class Parser {
public:
  Parser ( void );
  Parser ( const std::string& filename );
  void set ( const std::string& filename );
  void parse ( void );
  void parseExpr ( void );
  std::string print ( void );
  std::string printExprs ( void );
  std::string printVarTable ( void );
  std::string printFuncTable ( void );
private:
  //parseExpr
    //Methods
    void setPriorities ( void );
    bool isUnary ( const Tag& tag );

    pExpr parseExpr ( const Priority& priority );
    pExpr parseIdentifier ( Lexeme lexeme );
    pExpr parseFactor ( void );

    Priority upPriority ( const Priority& pr );
    bool checkPriority ( const Priority& pr, const Tag& tag );

    std::vector<pExpr> parseArgs ( void );
    //Variables
    std::unordered_map<Tag, Priority> binaryPriority;
    std::unordered_map<Tag, Priority> unaryPriority;
  //
  //parseBlocks
    //Methods
    //Checking for exception
      void errUndefType ( void );
      void errDuplicated ( void );
      void errConstOp ( const Lexeme& lexeme );
      void checkIdent ( const Lexeme& lexeme, SymTable& vTable, TypeTable& tTable );
      //If res.type <=> src.type || nullptr => res.[value|type] = src.[value|type] else errType();
      void checkType ( pSymVar& res, pSymVar& src );
      void checkFunc ( const std::string& name, IdentifierType type, const std::string& args = "" );

      void checkConst( pExpr& node );
    //

    pStmt parseStmt ( void );

    pStmt parseSimpleStmt ( void );
    pStmt parseIf ( void );
    pStmt parseWhile ( void );
    pStmt parseRepeat ( void );
    pStmt parseFor ( void );
    pStmt parseEmpty ( void );
    pStmt parseBlock ( void );

    pSymType parseType ( compiler::Lexeme& lexeme, SymTable& vTable, TypeTable& tTable, InitExpected init );
    pSymType parseRecord ( void );
    pSymType parseEnum ( void );

    void parseProgramName ( const compiler::Lexeme& program );

    void parseConst ( SymTable& vTable, TypeTable& tTable );
    void parseVar ( SymTable& vTable, TypeTable& tTable );
    //Initialization for Const-decl and Var-decl variables
    void parseConstExpr ( SymTable& vTable, TypeTable& tTable );
    pSymVar evalConstExpr ( pExpr& root, SymTable& vTable, TypeTable& tTable );

    void parseFunction ( bool expectRetVal = true );
    // void parseProcedure ( void );
    std::tuple<pSymTable, std::string> parseParams( void );

    void parseAlias ( SymTable& vTable, TypeTable& tTable );

    void setTypeTable ( void );
    void setVarTable( void );

    //Variables
    SymTable varTable;
    TypeTable typeTable;
    //{first: nameFunc; second: {key: args; value: descriptor of function/procedure;}}
    std::map< std::string, std::map< std::string, pSym> > funcTable;
    //Runtime-functions
    std::map< std::string, CompileTimeFunction> ctFuncTable;

    bool programTokenChecked;
  //
  //Global
    //Methods
      void err ( const std::string& expected_token = "" );//UNEXPECTED <- EXPECTED || EOF
      void err ( const Lexeme& lexeme );
    //Variables
      Scanner scanner;
      pNode root;//for all!(stmts and exprs)
    //
  //
};
};
