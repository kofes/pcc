#pragma once

#include <memory>
#include <vector>
#include "Node.hpp"
#include <sstream>

namespace compiler {

enum class ExprEnum {
  BinOp, UnOp, Integer, Real, Identifier, Record, Array, Function
};

struct Expr : public Node {
  Expr ( const Lexeme& lex ) : Node(lex) {};
  ExprEnum exprType;
};

typedef std::shared_ptr<Expr> pExpr;

struct ExprTerm : public Expr {
  ExprTerm ( const Lexeme& lex ) : Expr(lex) {};
};

struct ExprBinOp : public Expr {
  ExprBinOp ( const Lexeme& lex, const pExpr& lt, const pExpr& rt ) : Expr(lex), left(lt), right(rt) { exprType = ExprEnum::BinOp; };
  std::string print ( unsigned int deep ) override;
  pExpr left, right;
};
struct ExprUnOp : public Expr {
  ExprUnOp ( const Lexeme& lex, const pExpr& chld ) : Expr(lex), arg(chld) { exprType = ExprEnum::UnOp; };
  std::string print ( unsigned int deep ) override;
  pExpr arg;
};
struct ExprInteger : public ExprTerm {
  ExprInteger ( const Lexeme& lex ) : ExprTerm(lex) { exprType = ExprEnum::Integer; };
};
struct ExprReal : public ExprTerm {
  ExprReal ( const Lexeme& lex ) : ExprTerm(lex) { exprType = ExprEnum::Real; };
};
struct ExprIdentifier : public ExprTerm {
  ExprIdentifier ( const Lexeme& lex ) : ExprTerm(lex) { exprType = ExprEnum::Identifier; };
};
struct ExprRecordAccess : public ExprBinOp {
  ExprRecordAccess ( const Lexeme& lex, const pExpr& lt, const pExpr& rt ) : ExprBinOp(lex, lt, rt) { exprType = ExprEnum::Record; };
};
struct ExprArrayIndex : public Expr {
  ExprArrayIndex ( const Lexeme& lex, const pExpr& lt, const std::vector<pExpr>& vec ) : Expr(lex), left(lt), args(vec) { exprType = ExprEnum::Array; };
  std::string print ( unsigned int deep ) override;
  pExpr left;
  std::vector< pExpr > args;
};
struct ExprFunc : public Expr {
  ExprFunc ( const Lexeme& lex, const pExpr& lt, const std::vector<pExpr>& vec ) : Expr(lex), left(lt), args(vec) { exprType = ExprEnum::Function; };
  std::string print ( unsigned int deep ) override;
  pExpr left;
  std::vector< pExpr > args;
};
};
