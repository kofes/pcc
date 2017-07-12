#pragma once

#include <memory>
#include <vector>
#include <sstream>
#include "Node.hpp"

namespace compiler {

enum class ExprEnum {
  BinOp, UnOp, Integer, Real, Char, String, Identifier, Record, Array, Function
};

struct Expr;
typedef std::shared_ptr<Expr> pExpr;

struct Expr : public Node, public Lexeme {
  Expr ( const Lexeme& lex, ExprEnum expr ) : Lexeme(lex), expr(expr) {};
  virtual std::string print ( unsigned int deep );
  ExprEnum expr;
};
struct ExprTerm : public Expr {
  ExprTerm ( const Lexeme& lex, ExprEnum expr ) : Expr(lex, expr) {};
};

struct ExprOp : public Expr {
  ExprOp ( const Lexeme& lex, ExprEnum expr, const pExpr& arg0 ) : Expr(lex, expr), arg0(arg0) {};
  pExpr arg0;
};
struct ExprBinOp : public ExprOp {
  ExprBinOp ( const Lexeme& lex, const pExpr& arg0, const pExpr& arg1 ) : ExprOp(lex, ExprEnum::BinOp, arg0), arg1(arg1) {};
  std::string print ( unsigned int deep ) override;
  pExpr arg1;
};
struct ExprUnOp : public ExprOp {
  ExprUnOp ( const Lexeme& lex, const pExpr& chld ) : ExprOp(lex, ExprEnum::UnOp, chld) {};
  std::string print ( unsigned int deep ) override;
};
struct ExprRecordAccess : public ExprOp {
  ExprRecordAccess ( const Lexeme& lex, const pExpr& lt, const pExpr& rt ) : ExprOp(lex, ExprEnum::Record, lt), arg1(rt) {};
  std::string print ( unsigned int deep ) override;
  pExpr arg1;
};
struct ExprArrayIndex : public ExprOp {
  ExprArrayIndex ( const Lexeme& lex, const pExpr& lt, const std::vector<pExpr>& vec ) : ExprOp(lex, ExprEnum::Array, lt), args(vec) {};
  std::string print ( unsigned int deep ) override;
  std::vector< pExpr > args;
};

struct ExprInteger : public ExprTerm {
  ExprInteger ( const Lexeme& lex ) : ExprTerm(lex, ExprEnum::Integer) {};
};
struct ExprReal : public ExprTerm {
  ExprReal ( const Lexeme& lex ) : ExprTerm(lex, ExprEnum::Real) {};
};
struct ExprChar : public ExprTerm {
  ExprChar ( const Lexeme& lex ) : ExprTerm(lex, ExprEnum::Char) {};
};
struct ExprString : public ExprTerm {
  ExprString ( const Lexeme& lex ) : ExprTerm(lex, ExprEnum::String) {};
};
struct ExprIdentifier : public ExprTerm {
  ExprIdentifier ( const Lexeme& lex ) : ExprTerm(lex, ExprEnum::Identifier) {};
};

struct ExprFunc : public Expr {
  ExprFunc ( const Lexeme& lex, const pExpr& lt, const std::vector<pExpr>& vec ) : Expr(lex, ExprEnum::Function), left(lt), args(vec) {};
  std::string print ( unsigned int deep ) override;
  pExpr left;
  std::vector< pExpr > args;
};
struct ExprWrite: public ExprFunc {
  ExprWrite ( const Lexeme& lex, const pExpr& lt, const std::vector<pExpr>& vec ) : ExprFunc(lex, lt, vec) {};
};
struct ExprWriteln: public ExprFunc {
  ExprWriteln ( const Lexeme& lex, const pExpr& lt, const std::vector<pExpr>& vec ) : ExprFunc(lex, lt, vec) {};
};
struct ExprRead : public ExprFunc {
  ExprRead ( const Lexeme& lex, const pExpr& lt, const std::vector<pExpr>& vec ) : ExprFunc(lex, lt, vec) {};
};
struct ExprReadln : public ExprFunc {
  ExprReadln ( const Lexeme& lex, const pExpr& lt, const std::vector<pExpr>& vec ) : ExprFunc(lex, lt, vec) {};
};
}
