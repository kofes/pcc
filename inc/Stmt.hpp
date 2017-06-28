#pragma once

#include <memory>
#include <vector>
#include <sstream>
#include "Expr.hpp"
#include "Sym.hpp"
#include "Scanner.hpp"

namespace compiler {

struct Stmt : public Node {
  Stmt ( void ) { nodeType = NodeEnum::Stmt; }
  StmtEnum stmtType;
};

typedef std::shared_ptr<Stmt> pStmt;

struct StmtIf : public Stmt {
  StmtIf ( void ) { stmtType = StmtEnum::If; };
  std::string print ( unsigned int deep ) override;
  // void generate(Generator& asmGenerator) override;
  pExpr condition;
  pNode ifBody, elseBody;
};

struct StmtWhile : public Stmt {
  StmtWhile ( void ) { stmtType = StmtEnum::While; };
  std::string print ( unsigned int deep ) override;
  // void generate(Generator& asmGenerator) override;
  pExpr condition;
  pNode body;
};

struct StmtRepeat : public Stmt {
  StmtRepeat ( void ) { stmtType = StmtEnum::Repeat; };
  void add ( const pNode& nd );
  std::string print ( unsigned int deep ) override;
  // void generate(Generator& asmGenerator) override;
  std::vector<pNode> body;
  pExpr condition;
};

struct StmtFor : public Stmt {
  StmtFor ( void ) { stmtType = StmtEnum::For; };
  std::string print ( unsigned int deep ) override;
  // void generate(Generator& asmGenerator) override;
  Lexeme variableName;
  pExpr initVal, finalVal;
  Tag type;
  pNode body;
};

struct StmtEmpty : public Stmt {
  StmtEmpty ( void ) { stmtType = StmtEnum::Empty; };
};

struct StmtAssignment : public Stmt {
  StmtAssignment ( void ) { stmtType = StmtEnum::Assignment; };
  std::string print ( unsigned int deep ) override;
  pExpr variable;
  pExpr value;
};

struct StmtProcedure : public Stmt {
  StmtProcedure ( void ) { stmtType = StmtEnum::Procedure; };
  std::string print ( unsigned int deep ) override;
  pExpr value;
};

struct StmtBreak : public Stmt {
  StmtBreak ( void ) { stmtType = StmtEnum::Break; };
  std::string print ( unsigned int deep ) override;
  // void generate(Generator& asmGenerator) override;
};

struct StmtContinue : public Stmt {
  StmtContinue ( void ) { stmtType = StmtEnum::Continue; };
  std::string print ( unsigned int deep ) override;
  // void generate(Generator& asmGenerator) override;
};

struct StmtBlock : public Stmt {
  StmtBlock ( void ) { stmtType = StmtEnum::Block; };
  void add ( const pNode& nd );
  std::string print ( unsigned int deep ) override;
  // void generate(Generator& asmGenerator) override;
  std::vector<pNode> node;
};
};
