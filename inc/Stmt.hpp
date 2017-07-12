#pragma once

#include <memory>
#include <vector>
#include <sstream>
#include "Expr.hpp"
#include "Sym.hpp"
#include "Scanner.hpp"

namespace compiler {

enum class StmtEnum {
  If, While, Repeat, For, Empty, Assignment, Block, Procedure, Break, Continue
};

struct Stmt;
typedef std::shared_ptr<Stmt> pStmt;

struct Stmt : public Node {
  Stmt ( StmtEnum stmt ) : stmt(stmt) {};
  StmtEnum stmt;
};
struct StmtIf : public Stmt {
  StmtIf ( void ) : Stmt(StmtEnum::If) {};
  std::string print ( unsigned int deep ) override;
  pExpr condition;
  pNode ifBody, elseBody;
};
struct StmtWhile : public Stmt {
  StmtWhile ( void ) : Stmt(StmtEnum::While) {};
  std::string print ( unsigned int deep ) override;
  pExpr condition;
  pNode body;
};
struct StmtRepeat : public Stmt {
  StmtRepeat ( void ) : Stmt(StmtEnum::Repeat) {};
  void push ( const pNode& node );
  std::string print ( unsigned int deep ) override;
  pExpr condition;
  std::vector<pNode> body;
};
struct StmtFor : public Stmt {
  StmtFor ( void ) : Stmt(StmtEnum::For) {};
  std::string print ( unsigned int deep ) override;
  Lexeme variableName;
  Tag type;
  pExpr initVal, finalVal;
  pNode body;
};
struct StmtEmpty : public Stmt {
  StmtEmpty ( void ) : Stmt(StmtEnum::Empty) {};
};
struct StmtAssignment : public Stmt {
  StmtAssignment ( void ) : Stmt(StmtEnum::Assignment) {};
  std::string print ( unsigned int deep ) override;
  pExpr variable, value;
};
struct StmtProcedure : public Stmt {
  StmtProcedure ( void ) : Stmt(StmtEnum::Procedure) {};
  std::string print ( unsigned int deep ) override;
  pExpr value;
};
struct StmtBreak : public Stmt {
  StmtBreak ( void ) : Stmt(StmtEnum::Break) {};
  std::string print ( unsigned int deep ) override;
};
struct StmtContinue : public Stmt {
  StmtContinue ( void ) : Stmt(StmtEnum::Continue) {};
  std::string print ( unsigned int deep ) override;
};
struct StmtBlock : public Stmt {
  StmtBlock ( void ) : Stmt(StmtEnum::Block) {};
  void push ( const pNode& node );
  std::string print ( unsigned int deep ) override;
  std::vector<pNode> body;
};
}
