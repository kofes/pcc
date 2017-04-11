#pragma once

#include <memory>
#include <vector>
#include <sstream>
#include "Expr.hpp"
#include "Sym.hpp"
#include "Scanner.hpp"

namespace compiler {

struct Stmt : public Node {
  Stmt ( const Lexeme& lex ) : Node(lex) {};
};

typedef std::shared_ptr<Stmt> pStmt;

struct StmtIf : public Stmt {
  StmtIf ( const Lexeme& lex, const pExpr& cnd, const pNode& bd ) : Stmt(lex), condition(cnd), body(bd) {};
  std::string print ( unsigned int deep ) override;
  pExpr condition;
  pNode body;
};

struct StmtWhile : public Stmt {
  StmtWhile ( const Lexeme& lex, const pExpr& cnd, const pNode& bd ) : Stmt(lex), condition(cnd), body(bd) {};
  std::string print ( unsigned int deep ) override;
  pExpr condition;
  pNode body;
};

struct StmtRepeat : public Stmt {
  StmtRepeat ( const Lexeme& lex, const pNode& bd, const pExpr& cnd ) : Stmt(lex), body(bd), condition(cnd) {};
  std::string print ( unsigned int deep ) override;
  pNode body;
  pExpr condition;
};

struct StmtFor : public Stmt {
  StmtFor ( const Lexeme& lex, const pNode& var, const Lexeme& init, const Lexeme& fin, const Tag& tp, const pNode& bd ) : Stmt(lex), initVal(init), finalVal(fin), type(tp), body(bd) {};
  std::string print ( unsigned int deep ) override;
  pNode var;
  Lexeme initVal, finalVal;
  Tag type;
  pNode body;
};

struct StmtEmpty : public Stmt {
  StmtEmpty ( const Lexeme& lex ) : Stmt(lex) {};
};

struct StmtAssignment : public Stmt {
  StmtAssignment ( const Lexeme& lex, const pSymTable& tbl ) : Stmt(lex), table(tbl) {};
  std::string print ( unsigned int deep ) override;
  pSymTable table;
};

struct StmtBlock : public Stmt {
  StmtBlock ( const Lexeme& lex ) : Stmt(lex) {};
  void add ( const pNode& nd );
  std::string print ( unsigned int deep ) override;
  std::vector<pNode> node;
};
};
