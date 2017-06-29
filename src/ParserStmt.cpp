#include "../inc/Parser.hpp"

compiler::pStmt compiler::Parser::parseStmt ( SymTable& vTable ) {
  switch (scanner.lex().tag) {
    case (Tag::IF) : return parseIf(vTable);
    case (Tag::WHILE) : return parseWhile(vTable);
    case (Tag::REPEAT) : return parseRepeat(vTable);
    case (Tag::FOR) : return parseFor(vTable);
    case (Tag::SEMICOLON) : return parseEmpty();
    case (Tag::BEGIN) : return parseBlock(vTable);
    case (Tag::BREAK):
      return parseBreak();
    case (Tag::CONTINUE):
      return parseContinue();
    case (Tag::WRITELN):
    case (Tag::WRITE):
    case (Tag::READ):
    case (Tag::READLN):
    case (Tag::IDENTIFIER):
      return parseSimpleStmt(vTable);
    default : break;
  }
  err("`Statement`");
  return pStmt();
}

compiler::pStmt compiler::Parser::parseSimpleStmt ( SymTable& vTable ) {
  Lexeme lexeme = scanner.lex();
  scanner.next();
  pExpr expr = parseIdentifier(lexeme);

  lexeme = scanner.lex();

  if (lexeme.tag == Tag::COLON_EQUALS) {
    std::shared_ptr<StmtAssignment> assignmentStmt(new StmtAssignment);
    assignmentStmt->variable = expr;
    scanner.next();
    lexeme = scanner.lex();

    assignmentStmt->value = parseExpr(Priority::LOWEST);

    pSymType tp1 = evalVarType(assignmentStmt->variable, vTable);
    pSymType tp2 = evalExprType(assignmentStmt->value, vTable);
    if (tp1 != tp2)
      errAssignment(lexeme);

    return assignmentStmt;
  }

  if (expr->exprType != ExprEnum::Function)
    err("`Statement`");
  std::shared_ptr<StmtProcedure> procStmt(new StmtProcedure);
  procStmt->value = expr;

  scanner.next();

  return procStmt;
}

compiler::pStmt compiler::Parser::parseBreak ( void ) {
  std::shared_ptr<StmtBreak> stmt(new StmtBreak);
  Lexeme lexeme;
  scanner.next();
  lexeme = scanner.lex();
  if (lexeme.tag != Tag::SEMICOLON)
    err("';'");
  scanner.next();
  return stmt;
}

compiler::pStmt compiler::Parser::parseContinue ( void ) {
  std::shared_ptr<StmtContinue> stmt(new StmtContinue);
  Lexeme lexeme;
  scanner.next();
  lexeme = scanner.lex();
  if (lexeme.tag != Tag::SEMICOLON)
    err("';'");
  scanner.next();
  return stmt;
}

compiler::pStmt compiler::Parser::parseIf ( SymTable& vTable ) {
  std::shared_ptr<StmtIf> ifStmt(new StmtIf);
  scanner.next();
  Lexeme lexeme = scanner.lex();
  ifStmt->condition = parseExpr(Priority::LOWEST);
  //
  pSymType type = evalExprType(ifStmt->condition, vTable);
  if (type->symType != SymEnum::Scalar || std::dynamic_pointer_cast<TypeScalar>(type)->type != SCALAR_TYPE::BOOLEAN)
    err(lexeme);
  //
  lexeme = scanner.lex();
  if (lexeme.tag != Tag::THEN)
    err("THEN");

  scanner.next();

  ifStmt->ifBody = parseStmt(vTable);

  lexeme = scanner.lex();

  if (lexeme.tag == Tag::ELSE) {
    scanner.next();
    ifStmt->elseBody = parseStmt(vTable);
  }

  return ifStmt;
}

compiler::pStmt compiler::Parser::parseWhile ( SymTable& vTable ) {
  std::shared_ptr<StmtWhile> whileStmt(new StmtWhile);
  scanner.next();
  Lexeme lexeme = scanner.lex();
  whileStmt->condition = parseExpr(Priority::LOWEST);
  //
  pSymType type = evalExprType(whileStmt->condition, vTable);
  if (type->symType != SymEnum::Scalar || std::dynamic_pointer_cast<TypeScalar>(type)->type != SCALAR_TYPE::BOOLEAN)
    err(lexeme);
  //
  lexeme = scanner.lex();
  if (lexeme.tag != Tag::DO)
    err("DO");

  scanner.next();

  whileStmt->body = parseStmt(vTable);

  return whileStmt;
}

compiler::pStmt compiler::Parser::parseRepeat ( SymTable& vTable ) {
  std::shared_ptr<StmtRepeat> repeatStmt(new StmtRepeat);

  scanner.next();

  while (scanner.lex().tag != Tag::UNTIL) {
    pStmt stmt = parseStmt(vTable);
    if (stmt->stmtType == StmtEnum::Empty)
      continue;
    repeatStmt->add(stmt);
    if (scanner.lex().tag == Tag::SEMICOLON)
      scanner.next();
    else if (scanner.lex().tag != Tag::UNTIL)
      err("UNTIL");
  }

  scanner.next();
  Lexeme lexeme = scanner.lex();
  repeatStmt->condition = parseExpr(Priority::LOWEST);
  //
  pSymType type = evalExprType(repeatStmt->condition, vTable);
  if (type->symType != SymEnum::Scalar || std::dynamic_pointer_cast<TypeScalar>(type)->type != SCALAR_TYPE::BOOLEAN)
    err(lexeme);
  //
  return repeatStmt;
}

compiler::pStmt compiler::Parser::parseFor ( SymTable& vTable ) {
  std::shared_ptr<StmtFor> forStmt(new StmtFor);

  scanner.next();
  Lexeme lexeme = scanner.lex();

  if (lexeme.tag != Tag::IDENTIFIER)
    err("`variable`");

  forStmt->variableName = lexeme;

  //
  auto iter = vTable.find(lexeme.name);
  if (iter == vTable.end())
    err(lexeme);
  pSymType tp = evalAlias(iter->second->type);
  if (tp->symType != SymEnum::Scalar || std::dynamic_pointer_cast<TypeScalar>(tp)->type != SCALAR_TYPE::INTEGER)
    err(lexeme);
  //

  scanner.next();
  lexeme = scanner.lex();
  if (lexeme.tag != Tag::COLON_EQUALS)
    err(":=");

  scanner.next();
  forStmt->initVal = parseExpr(Priority::LOWEST);

  //
  tp = evalAlias(evalExprType(forStmt->initVal, vTable));
  if (tp->symType != SymEnum::Scalar || std::dynamic_pointer_cast<TypeScalar>(tp)->type != SCALAR_TYPE::INTEGER)
    err(lexeme);
  //

  lexeme = scanner.lex();
  if (lexeme.tag != Tag::TO && lexeme.tag != Tag::DOWNTO)
    err("TO/DOWNTO");

  forStmt->type = lexeme.tag;

  scanner.next();
  forStmt->finalVal = parseExpr(Priority::LOWEST);

  //
  tp = evalAlias(evalExprType(forStmt->initVal, vTable));
  if (tp->symType != SymEnum::Scalar || std::dynamic_pointer_cast<TypeScalar>(tp)->type != SCALAR_TYPE::INTEGER)
    err(lexeme);
  //

  lexeme = scanner.lex();
  if (lexeme.tag != Tag::DO)
    err("DO");

  scanner.next();

  forStmt->body = parseStmt(vTable);

  return forStmt;
}

compiler::pStmt compiler::Parser::parseEmpty ( void ) {
  pStmt res = pStmt(new StmtEmpty);
  scanner.next();
  return res;
}

compiler::pStmt compiler::Parser::parseBlock ( SymTable& vTable ) {
  std::shared_ptr<StmtBlock> block(new StmtBlock);
  scanner.next();
  while (scanner.lex().tag != Tag::END) {
    pStmt stmt = parseStmt(vTable);
    if (stmt->stmtType == StmtEnum::Empty)
      continue;
    block->add(stmt);
    if (scanner.lex().tag == Tag::SEMICOLON)
      scanner.next();
    else if (scanner.lex().tag != Tag::END)
      err("END");
  }
  scanner.next();
  return block;
}
