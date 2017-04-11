#pragma once

#include <memory>
#include <vector>
#include "Node.hpp"
#include <sstream>

namespace compiler {

struct Expr : public Node {
  Expr ( const Lexeme& lex ) : Node(lex) {};
};

typedef std::shared_ptr<Expr> pExpr;

struct ExprTerm : public Expr {
  ExprTerm ( const Lexeme& lex ) : Expr(lex) {};
};

struct ExprBinOp : public Expr {
  ExprBinOp ( const Lexeme& lex, const pExpr& lt, const pExpr& rt ) : Expr(lex), left(lt), right(rt) {};
  std::string print ( unsigned int deep ) override;
  pExpr left, right;
};
struct ExprUnOp : public Expr {
  ExprUnOp ( const Lexeme& lex, const pExpr& chld ) : Expr(lex), arg(chld) {};
  std::string print ( unsigned int deep ) override;
  pExpr arg;
};
struct ExprInteger : public ExprTerm {
  ExprInteger ( const Lexeme& lex ) : ExprTerm(lex) {};
};
struct ExprReal : public ExprTerm {
  ExprReal ( const Lexeme& lex ) : ExprTerm(lex) {};
};
struct ExprIdentifier : public ExprTerm {
  ExprIdentifier ( const Lexeme& lex ) : ExprTerm(lex) {};
};
struct ExprRecordAccess : public ExprBinOp {
  ExprRecordAccess ( const Lexeme& lex, const pExpr& lt, const pExpr& rt ) : ExprBinOp(lex, lt, rt) {};
};
struct ExprArrayIndex : public Expr {
  ExprArrayIndex ( const Lexeme& lex, const pExpr& lt, const std::vector<pExpr>& vec ) : Expr(lex), left(lt), args(vec) {};
  std::string print ( unsigned int deep ) override;
  pExpr left;
  std::vector< pExpr > args;
};
};
