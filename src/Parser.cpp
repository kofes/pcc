#include "../inc/Parser.hpp"

//DEBUG
#include <iostream>
//

compiler::Parser::Parser ( void ) {
  setPriorities();
  setTypeTable();
  setVarTable();
};

compiler::Parser::Parser ( const std::string& filename ) : scanner(filename) {
  setPriorities();
  setTypeTable();
  setVarTable();
};

void compiler::Parser::set ( const std::string& filename ) {
  scanner.open(filename);
};

void compiler::Parser::parseExpr ( void ) {
  scanner.next();
  root = parseExpr(compiler::Priority::LOWEST);
};

void compiler::Parser::parse ( void ) {
  programTokenChecked = false;
  scanner.next();
  Lexeme lexeme = scanner.lex();
  while (true) {
    switch (scanner.lex().tag) {
      case (Tag::PROGRAM) : programTokenChecked ? err(scanner.lex()) : parseProgramName(lexeme); break;
      case (Tag::CONST) : parseConst(varTable, typeTable); break;
      case (Tag::TYPE) : parseAlias(varTable, typeTable); break;
      case (Tag::VAR) : parseVar(varTable, typeTable); break;
      case (Tag::FUNCTION) : parseFunction(); break;
      case (Tag::PROCEDURE) : parseFunction(false); break;
      case (Tag::BEGIN) : root = parseBlock(); return;
      // default : err("BEGIN STATEMENT");
      default : return;
    }
    programTokenChecked = true;
  }
};

compiler::pStmt compiler::Parser::parseStmt ( void ) {
  switch (scanner.lex().tag) {
    case (Tag::IF) : return parseIf();
    case (Tag::WHILE) : return parseWhile();
    case (Tag::REPEAT) : return parseRepeat();
    case (Tag::FOR) : return parseFor();
    case (Tag::SEMICOLON) : return parseEmpty();
    case (Tag::BEGIN) : return parseBlock();
    case (Tag::WRITELN):
    case (Tag::WRITE):
    case (Tag::READ):
    case (Tag::READLN):
    case (Tag::BREAK):
    case (Tag::CONTINUE):
    case (Tag::UNDEFINED) :
      if (scanner.lex().token != Token::IDENTIFIER) break;
      return parseSimpleStmt();
    default : break;
  }
  err("`Statement`");
  return pStmt();
};

compiler::pStmt compiler::Parser::parseSimpleStmt ( void ) {
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

    return assignmentStmt;
  }

  if (expr->exprType != ExprEnum::Function)
    err("`Statement`");
  std::shared_ptr<StmtProcedure> procStmt(new StmtProcedure);
  procStmt->value = expr;

  scanner.next();

  return procStmt;
};

compiler::pStmt compiler::Parser::parseIf ( void ) {
  std::shared_ptr<StmtIf> ifStmt(new StmtIf);

  scanner.next();
  ifStmt->condition = parseExpr(Priority::LOWEST);

  Lexeme lexeme = scanner.lex();
  if (lexeme.tag != Tag::THEN)
    err("THEN");

  scanner.next();

  ifStmt->ifBody = parseStmt();

  lexeme = scanner.lex();

  if (lexeme.tag == Tag::ELSE) {
    scanner.next();
    ifStmt->elseBody = parseStmt();
  }

  return ifStmt;
};

compiler::pStmt compiler::Parser::parseWhile ( void ) {
  std::shared_ptr<StmtWhile> whileStmt(new StmtWhile);

  scanner.next();
  whileStmt->condition = parseExpr(Priority::LOWEST);

  Lexeme lexeme = scanner.lex();
  if (lexeme.tag != Tag::DO)
    err("DO");

  scanner.next();

  whileStmt->body = parseStmt();

  return whileStmt;
};

compiler::pStmt compiler::Parser::parseRepeat ( void ) {
  std::shared_ptr<StmtRepeat> repeatStmt(new StmtRepeat);

  scanner.next();

  while (scanner.lex().tag != Tag::UNTIL) {
    pStmt stmt = parseStmt();
    if (stmt->stmtType == StmtEnum::Empty)
      continue;
    repeatStmt->add(stmt);
    if (scanner.lex().tag == Tag::SEMICOLON)
      scanner.next();
    else if (scanner.lex().tag != Tag::UNTIL)
      err("UNTIL");
  }

  scanner.next();

  repeatStmt->condition = parseExpr(Priority::LOWEST);

  return repeatStmt;
};

compiler::pStmt compiler::Parser::parseFor ( void ) {
  std::shared_ptr<StmtFor> forStmt(new StmtFor);

  scanner.next();
  Lexeme lexeme = scanner.lex();

  if (lexeme.tag != Tag::UNDEFINED || lexeme.token != Token::IDENTIFIER)
    err("`Variable`");

  forStmt->variableName = lexeme;

  scanner.next();
  lexeme = scanner.lex();
  if (lexeme.tag != Tag::COLON_EQUALS)
    err(":=");

  scanner.next();
  forStmt->initVal = parseExpr(Priority::LOWEST);

  lexeme = scanner.lex();
  if (lexeme.tag != Tag::TO && lexeme.tag != Tag::DOWNTO)
    err("TO/DOWNTO");

  forStmt->type = lexeme.tag;

  scanner.next();
  forStmt->finalVal = parseExpr(Priority::LOWEST);

  lexeme = scanner.lex();
  if (lexeme.tag != Tag::DO)
    err("DO");

  scanner.next();

  forStmt->body = parseStmt();

  return forStmt;
};

compiler::pStmt compiler::Parser::parseEmpty ( void ) {
  pStmt res = pStmt(new StmtEmpty);
  scanner.next();
  return res;
};

compiler::pStmt compiler::Parser::parseBlock ( void ) {
  std::shared_ptr<StmtBlock> block(new StmtBlock);
  scanner.next();
  while (scanner.lex().tag != Tag::END) {
    pStmt stmt = parseStmt();
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
};

compiler::pSymType compiler::Parser::parseType ( SymTable& vTable, TypeTable& tTable, InitExpected init ) {
  pSymType type = nullptr;
  Lexeme lexeme = scanner.lex();
  switch (lexeme.tag) {
    case (Tag::ARRAY) :
      type = parseArray(vTable, tTable);
    break;
    // case (Tag::SET) :
    default :
      auto iter = typeTable.find(lexeme.name);
      if (iter == typeTable.end())
        errUndefType();
      type = iter->second;
  }

  scanner.next();
  lexeme = scanner.lex();

  switch (lexeme.tag) {
    case (Tag::EQUALS):
      if (init == InitExpected::NO)
        err(";");
    break;
    case (Tag::SEMICOLON):
      if (init == InitExpected::YES)
        err("=");
    break;
    default: err("=/;");//TODO:REFACTERING!
  };

  return type;
};

compiler::pSymType compiler::Parser::parseArray ( SymTable& vTable, TypeTable& tTable ) {
  std::shared_ptr<TypeArray> vArray = nullptr;
  Lexeme lexeme = scanner.lex();
  pSymType type = pSymType(new TypeArray(lexeme.name));
  while (lexeme.tag == Tag::ARRAY) {
    scanner.next();
    lexeme = scanner.lex();
    //ONLY STATIC ARRAY!
    if (lexeme.tag == Tag::LEFT_BRACKET) {
      do {
        if (vArray == nullptr)
          vArray = std::dynamic_pointer_cast<TypeArray>(type);
        else {
          lexeme.name = "ARRAY";
          lexeme.tag = Tag::ARRAY;
          lexeme.token = Token::IDENTIFIER;
          vArray->elemType = pSym(new TypeArray(lexeme.name));
          vArray = std::dynamic_pointer_cast<TypeArray>(vArray->elemType);
        }
        Lexeme dbg_lex = lexeme;

        scanner.next();
        lexeme = scanner.lex();

        //Parse LOW
        pExpr local_root = parseExpr(Priority::LOWEST);
        checkConst(local_root);
        pSymVar tmp = pSymVar(evalConstExpr(local_root, vTable, tTable));
        if (tmp->name != "INTEGER") err("integer literal/const integer");
        vArray->low = std::strtoll(tmp->name.c_str(), nullptr, 10);

        if (scanner.lex().tag != Tag::DOUBLE_DOT)
          err("..");

        scanner.next();
        lexeme = scanner.lex();
        //Parse HIGH
        local_root = parseExpr(Priority::LOWEST);
        checkConst(local_root);
        tmp = pSymVar(evalConstExpr(local_root, vTable, tTable));
        if (tmp->name != "INTEGER") err("integer literal/const integer");
        vArray->high = std::strtoll(tmp->name.c_str(), nullptr, 10);
        if (vArray->high <= vArray->low)
          errHighLow(dbg_lex);
      } while (scanner.lex().tag == Tag::COMMA);

      //ONLY STATIC ARRAY!
      if (scanner.lex().tag != Tag::RIGHT_BRACKET) err("]");
      scanner.next();
      lexeme = scanner.lex();
    } else {
      vArray = std::dynamic_pointer_cast<TypeArray>(type);
      vArray->high = 0;
      vArray->low = 1;
    }
    if (lexeme.tag != Tag::OF) err("OF");
    scanner.next();
    lexeme = scanner.lex();
  }
  auto iter = typeTable.find(lexeme.name);
  if (iter == typeTable.end())
    errUndefType();
  vArray->elemType = iter->second;

  return type;
};

compiler::pSymType compiler::Parser::parseRecord ( void ) {};
compiler::pSymType compiler::Parser::parseEnum ( void ) {};

compiler::pSymType compiler::Parser::parsePointer ( void ) {
    std::shared_ptr<TypePointer> type(new TypePointer(scanner.lex().name));
    scanner.next();
    Lexeme lexeme = scanner.lex();
    auto iter = typeTable.find(lexeme.name);
    if (iter == typeTable.end())
      errUndefType();
    type->elemType = iter->second;
    scanner.next();
    lexeme = scanner.lex();
    if (lexeme.tag != Tag::SEMICOLON)
      err("';'");

    return type;
};

void compiler::Parser::parseProgramName ( const compiler::Lexeme& program ) {
  scanner.next();
  compiler::Lexeme lexeme = scanner.lex();
  if (lexeme.token != Token::IDENTIFIER || lexeme.tag != Tag::UNDEFINED)
    err("PROGRAM NAME");

  std::shared_ptr<compiler::SymVar> var = std::shared_ptr<compiler::SymVar>(new SymVar(program.name));
  var->glob = compiler::GLOB::CONST;
  var->type = std::shared_ptr<compiler::SymType>(new SymType(program.name));
  varTable[lexeme.name] = var;

  scanner.next();
  lexeme = scanner.lex();
  if (lexeme.tag != Tag::SEMICOLON)
    err("SEMICOLON");
  scanner.next();
};

void compiler::Parser::parseConst ( SymTable& vTable, TypeTable& tTable ) {
  scanner.next();
  Lexeme lexeme = scanner.lex();

  for (; lexeme.token == Token::IDENTIFIER && lexeme.tag == Tag::UNDEFINED;
         scanner.next(), lexeme = scanner.lex()) {

    checkIdent(lexeme, vTable, tTable);
    checkFunc(lexeme.name, IdentifierType::VARIABLE);

    pSymVar var = pSymVar(new SymVar(lexeme.name));

    scanner.next();
    lexeme = scanner.lex();

    var->type = nullptr;

    switch (lexeme.tag) {
      case(Tag::COLON) :
        var->type = parseType(vTable, tTable, InitExpected::YES);
      case (Tag::EQUALS) :
      break;
      default :
        err("=");
      break;
    }
    //NOW SCANNER == '='
    scanner.next();
    lexeme = scanner.lex();

    //NOW PARSE CONST EXPR
    pExpr local_root = parseExpr(Priority::LOWEST);
    checkConst(local_root);
    pSymVar tmp = pSymVar(evalConstExpr(local_root, vTable, tTable));
    checkType(var, tmp);

    //NOW scanner.lex().name is ';'

    var->glob = compiler::GLOB::CONST;
    vTable[var->name] = var;//TO UPPERCASE!

    scanner.next();
    lexeme = scanner.lex();
  }
};

void compiler::Parser::parseVar ( SymTable& vTable, TypeTable& tTable ) {
  std::vector<pSymVar> var;

  scanner.next();
  Lexeme lexeme = scanner.lex();

  for (;lexeme.token == Token::IDENTIFIER && lexeme.tag == Tag::UNDEFINED;
    scanner.next(), lexeme = scanner.lex()) {
    checkIdent(lexeme, vTable, tTable);
    checkFunc(lexeme.name, IdentifierType::VARIABLE);

    var.push_back(pSymVar(new SymVar(lexeme.name)));

    scanner.next();
    lexeme = scanner.lex();

    for (;lexeme.tag == Tag::COMMA; scanner.next(), lexeme = scanner.lex()) {
      scanner.next();
      lexeme = scanner.lex();

      checkIdent(lexeme, vTable, tTable);
      checkFunc(lexeme.name, IdentifierType::VARIABLE);
      var.push_back(pSymVar(new SymVar(lexeme.name)));
    }

    if (lexeme.tag != Tag::COLON)
      err("':'");

    scanner.next();
    lexeme = scanner.lex();

    pSymType type = parseType(vTable, tTable, InitExpected::MAYBE);

    if (scanner.lex().tag == Tag::EQUALS) {
      if (var.size() != 1)
        err("';'");
      scanner.next();
      lexeme = scanner.lex();
      pExpr local_root = parseExpr(Priority::LOWEST);
      checkConst(local_root);
      pSymVar tmp = pSymVar(evalConstExpr(local_root, vTable, tTable));
      checkType(var.front(), tmp);
    }

    for (pSymVar& elem : var) {
      elem->type = type;
      elem->glob = compiler::GLOB::VAR;
      vTable[elem->name] = elem;
    }
    var.clear();
    if (scanner.lex().tag != Tag::SEMICOLON)
      err("';'");
  }
};

void compiler::Parser::parseFunction ( bool expectRetVal ) {
  scanner.next();
  Lexeme lexeme = scanner.lex();

  //Pascal feature: function result variable has name like a name of function
  checkIdent(lexeme, varTable, typeTable);

  std::shared_ptr<SymFunc> func(new SymFunc(lexeme.name));

  scanner.next();
  lexeme = scanner.lex();
  std::string args;
  func->params = nullptr;
  if (lexeme.tag == Tag::LEFT_PARENTHESIS) {
    std::tie(func->params, args) = parseParams();
    //NOW scanner.lex().name == ")"
    scanner.next();
    lexeme = scanner.lex();
  }

  checkFunc(func->name, IdentifierType::FUNCTION, args);

  //
  if (expectRetVal) {
    if (lexeme.tag != Tag::COLON)
      err("':'");

    scanner.next();
    lexeme = scanner.lex();

    func->retType = parseType(varTable, typeTable, InitExpected::NO);
  } else
    if (lexeme.tag != Tag::SEMICOLON)
      err("';'");
  //

  //NOW scanner.lex().name == ";"

  scanner.next();
  lexeme = scanner.lex();

  while (lexeme.tag != Tag::BEGIN) {
    switch (lexeme.tag) {
      case (Tag::TYPE) : parseAlias(func->varTable, func->typeTable); break;
      case (Tag::VAR) : parseVar(func->varTable, func->typeTable); break;
      case (Tag::CONST) : parseConst(func->varTable, func->typeTable); break;
      default: err("`BEGIN`");
    }
  }

  //NOW scanner.lex().name == "BEGIN"

  func->body = parseBlock();

  //NOW scanner.lex().name == "END"

  scanner.next();
  lexeme = scanner.lex();

  if (lexeme.tag != Tag::SEMICOLON)
    err("';'");

  funcTable[func->name][args] = func;

  scanner.next();
  lexeme = scanner.lex();
};

void compiler::Parser::parseAlias ( SymTable& vTable, TypeTable& tTable ) {
  scanner.next();
  compiler::Lexeme lexeme = scanner.lex();

  while (lexeme.token == Token::IDENTIFIER && lexeme.tag == Tag::UNDEFINED) {

    checkIdent(lexeme, vTable, tTable);
    checkFunc(lexeme.name, IdentifierType::TYPE);

    Lexeme ident = lexeme;
    pSymType type;

    scanner.next();
    lexeme = scanner.lex();

    if (lexeme.tag != Tag::EQUALS)
      err("'='");

    scanner.next();
    lexeme = scanner.lex();

    switch (lexeme.tag) {
      case (Tag::RECORD): type = parseRecord(); break;
      case (Tag::LEFT_PARENTHESIS): type = parseEnum(); break;
      case (Tag::POINTER): type = parsePointer(); break;
      default: type = parseType(vTable, tTable, InitExpected::NO); break;
    }

    //NOW scanner.lex().name == ";"

    tTable[ident.name] = type;

    scanner.next();
    lexeme = scanner.lex();
  }
};

compiler::pExpr compiler::Parser::parseExpr ( const compiler::Priority& priority ) {
  if (priority == Priority::HIGHEST)
    return parseFactor();
  compiler::pExpr left = parseExpr(upPriority(priority));
  compiler::Lexeme lexeme = scanner.lex();
  while (checkPriority(priority, lexeme.tag)) {
    scanner.next();
    compiler::pExpr right = parseExpr(upPriority(priority));
    left = compiler::pExpr(new compiler::ExprBinOp(lexeme, left, right));
    lexeme = scanner.lex();
  }
  return left;
};

compiler::pExpr compiler::Parser::parseFactor ( void ) {
  compiler::pExpr tmpExpr;
  compiler::Lexeme lexeme = scanner.lex();
  scanner.next();

  if (lexeme.token == Token::IDENTIFIER && lexeme.tag == Tag::UNDEFINED)
    return parseIdentifier(lexeme);
  if (isUnary(lexeme.tag))
    return compiler::pExpr(new ExprUnOp(lexeme, parseExpr(Priority::HIGHEST)));

  switch (lexeme.tag) {
    case (Tag::INTEGER) : return compiler::pExpr(new ExprInteger(lexeme));
    case (Tag::FLOAT) : return compiler::pExpr(new ExprReal(lexeme));
    case (Tag::LEFT_PARENTHESIS) :
      tmpExpr = parseExpr(compiler::Priority::LOWEST);
      lexeme = scanner.lex();

      if (lexeme.tag != Tag::RIGHT_PARENTHESIS) {
        if (lexeme.token == Token::END_OF_FILE) err();
        err(")");
      }

      scanner.next();
    return tmpExpr;
    case (Tag::B_TRUE):
    case (Tag::B_FALSE):
      return pExpr(new ExprIdentifier(lexeme));
    default: break;
  }

  if (lexeme.token == Token::END_OF_FILE) err();
  err(lexeme);
  return compiler::pExpr(new Expr(lexeme));
};

compiler::pExpr compiler::Parser::parseIdentifier ( compiler::Lexeme lexeme ) {
  compiler::pExpr left = compiler::pExpr(new ExprIdentifier(lexeme));

  while (true) {
    lexeme = scanner.lex();
    compiler::Lexeme tmpLex;
    std::vector< compiler::pExpr > args;
    compiler::pExpr right;
    switch (lexeme.tag) {
      case (Tag::DOT):
        tmpLex = lexeme;
        scanner.next();
        lexeme = scanner.lex();
        if (lexeme.token == Token::END_OF_FILE) err();
        if (lexeme.token != Token::IDENTIFIER || lexeme.tag != Tag::UNDEFINED) err("IDENTIFIER");
        right = compiler::pExpr(new ExprIdentifier(lexeme));
        left = compiler::pExpr(new ExprRecordAccess(tmpLex, left, right));
      break;
      case (Tag::LEFT_BRACKET):
        tmpLex = lexeme;
        args = parseArgs();
        left = compiler::pExpr(new ExprArrayIndex(tmpLex, left, args));
        lexeme = scanner.lex();
        if (lexeme.token == Token::END_OF_FILE) err();
        if (lexeme.tag != Tag::RIGHT_BRACKET) err("]");
      break;
      case (Tag::POINTER):
        left = compiler::pExpr(new ExprUnOp(lexeme, left));
      break;
      case (Tag::LEFT_PARENTHESIS):
        tmpLex = lexeme;
        args = parseArgs();
        left = compiler::pExpr(new ExprFunc(tmpLex, left, args));
        lexeme = scanner.lex();
        if (lexeme.token == Token::END_OF_FILE) err();
        if (lexeme.tag != Tag::RIGHT_PARENTHESIS) err(")");
      break;
      default: return left;
    }
    scanner.next();
  }
  return left;
};

std::string compiler::Parser::print ( void ) {
  std::ostringstream out;

  out << "<FUNC TABLE>\n"
      << printFuncTable()
      << "<TYPE TABLE>\n"
      << printTypeTable()
      << "<VAR TABLE>\n"
      << printVarTable()
      << "<MAIN BLOCK>\n"
      << printExprs();

  return out.str();
};

std::string compiler::Parser::printExprs ( void ) {
  // if (scanner.lex().token != Token::END_OF_FILE) err(scanner.lex());//Program's end is 'end.'

  std::ostringstream out;
  if (root != nullptr)
    out << root->print(0);

  return out.str();
};

std::string compiler::Parser::printVarTable ( void ) {
  std::ostringstream out;
  for (const std::pair< std::string, pSymVar >& elem : varTable)
    out << elem.second->print(0) << '\n';

  return out.str();
};

std::string compiler::Parser::printFuncTable ( void ) {
  std::ostringstream out;
  for (const std::pair< std::string, std::map< std::string, pSym> >& functions : funcTable)
    for (const std::pair< std::string, pSym>& elem : functions.second)
      out << elem.second->print(0) << '\n';

  return out.str();
};

std::string compiler::Parser::printTypeTable ( void ) {
    std::ostringstream out;
    for (const std::pair< std::string, pSymType >& type : typeTable)
        out << type.first << ':' << type.second->print(0) << '\n';

    return out.str();
};

std::vector<compiler::pExpr> compiler::Parser::parseArgs ( void ) {
  std::vector<compiler::pExpr> expr;
  compiler::Lexeme lexeme;
  scanner.next();
  expr.push_back(parseExpr(compiler::Priority::LOWEST));
  lexeme = scanner.lex();
  while (lexeme.tag == Tag::COMMA) {
    scanner.next();
    expr.push_back(parseExpr(compiler::Priority::LOWEST));
    lexeme = scanner.lex();
  }
  return expr;
};

compiler::Priority compiler::Parser::upPriority ( const compiler::Priority& pr ) {
  if (pr == compiler::Priority::HIGHEST)
    return compiler::Priority::HIGHEST;
  return (compiler::Priority)((unsigned short)pr - 1);
};

bool compiler::Parser::checkPriority ( const compiler::Priority& pr, const Tag& tag ) {
  auto tmp = binaryPriority.find(tag);
  return tmp != binaryPriority.end() && tmp->second == pr;
};

bool compiler::Parser::isUnary ( const Tag& tag ) {
  return unaryPriority.find(tag) != unaryPriority.end();
};

void compiler::Parser::err ( const std::string& expected_token ) {
  compiler::Lexeme lexeme = scanner.lex();
  if (expected_token == "")
    throw ExprException("Unexpected end of file in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + ");");
  if (lexeme.token == Token::END_OF_FILE)
    throw ExprException("Unexpected end of file in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + "); expected '" + expected_token + "';");
  throw ExprException("Unexpected token '" + lexeme.name + "' in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + "); expected '" + expected_token + "';");
};

void compiler::Parser::err ( const compiler::Lexeme& lexeme ) {
  if (lexeme.token == Token::END_OF_FILE)
    throw ExprException("Unexpected end of file in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + ");");
  throw ExprException("Unexpected token '" + lexeme.name + "' in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + ");");
};

void compiler::Parser::errUndefType ( void ) {
  compiler::Lexeme lexeme = scanner.lex();
  if (lexeme.token == Token::END_OF_FILE)
    throw ExprException("Unexpected end of file in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + "); expected type name;");
  throw ExprException("Undefined type '" + lexeme.name + "' in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + ");");
};

void compiler::Parser::errDuplicated ( void ) {
  compiler::Lexeme lexeme = scanner.lex();
  throw ExprException("Duplicate identifier \"" + lexeme.name + "\" in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + ");");
};

void compiler::Parser::errHighLow ( const compiler::Lexeme& lexeme ) {
  throw ExprException("array 'high' <= 'low' from pos(" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + ");");
}

void compiler::Parser::checkIdent ( const Lexeme& lexeme, SymTable& vTable, TypeTable& tTable ) {
  if (lexeme.token != Token::IDENTIFIER || lexeme.tag != Tag::UNDEFINED)
    err("Identifier");
  if (varTable.find(lexeme.name) != varTable.end()  ||
        vTable.find(lexeme.name) != vTable.end()    ||
     typeTable.find(lexeme.name) != typeTable.end() ||
        tTable.find(lexeme.name) != tTable.end())
    errDuplicated();
};

void compiler::Parser::setPriorities ( void ) {
  binaryPriority = {
    {Tag::EQUALS, compiler::Priority::LOWEST},
    {Tag::MIS, compiler::Priority::LOWEST},
    {Tag::GREATER, compiler::Priority::LOWEST},
    {Tag::LESS, compiler::Priority::LOWEST},
    {Tag::GREATER_EQ, compiler::Priority::LOWEST},
    {Tag::LESS_EQ, compiler::Priority::LOWEST},
    {Tag::IN, compiler::Priority::LOWEST},

    {Tag::ADD, compiler::Priority::THIRD},
    {Tag::SUB, compiler::Priority::THIRD},
    {Tag::OR, compiler::Priority::THIRD},
    {Tag::XOR, compiler::Priority::THIRD},

    {Tag::MUL, compiler::Priority::SECOND},
    {Tag::DIV_FLOAT, compiler::Priority::SECOND},
    {Tag::DIV_INT, compiler::Priority::SECOND},
    {Tag::MOD, compiler::Priority::SECOND},
    {Tag::AND, compiler::Priority::SECOND},
    {Tag::SHL, compiler::Priority::SECOND},
    {Tag::SHR, compiler::Priority::SECOND}
  };

  unaryPriority = {
    {Tag::NOT, compiler::Priority::HIGHEST},
    {Tag::SUB, compiler::Priority::HIGHEST},
    {Tag::ADD, compiler::Priority::HIGHEST},
    {Tag::ADDRESS, compiler::Priority::HIGHEST}
  };
};

void compiler::Parser::setTypeTable ( void ) {
  typeTable[tagBook.at(Tag::POINTER)] = compiler::pSymType(new TypeScalar(SCALAR_TYPE::POINTER));
  typeTable[tagBook.at(Tag::INTEGER)] = compiler::pSymType(new TypeScalar(SCALAR_TYPE::INTEGER));
  typeTable[tagBook.at(Tag::REAL)] = compiler::pSymType(new TypeScalar(SCALAR_TYPE::REAL));
  typeTable[tagBook.at(Tag::CHAR)] = compiler::pSymType(new TypeScalar(SCALAR_TYPE::CHAR));
  typeTable[tagBook.at(Tag::BOOLEAN)] = compiler::pSymType(new TypeScalar(SCALAR_TYPE::BOOLEAN));
};

void compiler::Parser::setVarTable ( void ) {
  pSymVar pi(new SymVar("PI"));
  pi->glob = GLOB::CONST;
  pi->value = "3.1415926535897932385";
  pi->type = typeTable[tagBook.at(Tag::REAL)];
  varTable[pi->name] = pi;
};

//TODO!
compiler::pSymVar compiler::Parser::evalConstExpr ( pExpr& root, SymTable& vTable, TypeTable& tTable ) {};

//TODO!
std::tuple<compiler::pSymTable, std::string> compiler::Parser::parseParams( void ) {
  Lexeme lexeme = scanner.lex();
  pSymTable resTable(new SymTable);
  std::ostringstream sstream;

  while ((lexeme.token == Token::IDENTIFIER && lexeme.tag == Tag::UNDEFINED) ||
         lexeme.tag == Tag::CONST ||
         lexeme.tag == Tag::VAR) {
    GLOB glob;
    glob = GLOB::VAL_PARAM;
    if (lexeme.tag == Tag::CONST) {
      glob = GLOB::CONST_PARAM;
      scanner.next();
    }
    if (lexeme.tag == Tag::VAR) {
      glob = GLOB::VAR_PARAM;
      scanner.next();
    }
    if (lexeme.tag == Tag::OUT) {
      glob = GLOB::OUT_PARAM;
      scanner.next();
    }
    lexeme = scanner.lex();
    if (lexeme.token != Token::IDENTIFIER || lexeme.tag != Tag::UNDEFINED)
      err("`parameter`");
    std::vector<pSymVar> vars;
    do {
      vars.push_back(pSymVar(new SymVar(lexeme.name)));
      scanner.next();
      lexeme = scanner.lex();
    } while (lexeme.tag == Tag::COMMA);
    pSymType type;
    //CONST
    if (glob == GLOB::CONST_PARAM) {
      if (lexeme.tag == Tag::COLON) {
        scanner.next();
        type = parseType(varTable, typeTable, InitExpected::MAYBE);
        lexeme = scanner.lex();
        if (lexeme.tag == Tag::EQUALS) {
          if (vars.size() > 1)  err("';'");
          scanner.next();
          pExpr local_root = parseExpr(Priority::LOWEST);
          checkConst(local_root);
          pSymVar tmp = pSymVar(evalConstExpr(local_root, varTable, typeTable));
          checkType(vars.front(), tmp);
        }
      }
    }
    //VALUE
    if (glob == GLOB::VAL_PARAM) {
      if (lexeme.tag != Tag::COLON) err("':'");
      scanner.next();
      type = parseType(varTable, typeTable, InitExpected::MAYBE);
      lexeme = scanner.lex();
      if (lexeme.tag == Tag::EQUALS) {
        if (vars.size() > 1)  err("';'");
        scanner.next();
        pExpr local_root = parseExpr(Priority::LOWEST);
        checkConst(local_root);
        pSymVar tmp = pSymVar(evalConstExpr(local_root, varTable, typeTable));
        checkType(vars.front(), tmp);
      }
    }
    //VAR/OUT
    if (lexeme.tag == Tag::VAR || lexeme.tag == Tag::OUT) {
      if (lexeme.tag == Tag::COLON) {
        scanner.next();
        type = parseType(varTable, typeTable, InitExpected::NO);
      }
    }
    //resTable < vars, resStr < str
    sstream << vars.size() << ':' << type->name << ';';
    for (pSymVar& elem : vars) {
      elem->type = type;
      elem->glob = glob;
      (*resTable)[elem->name] = elem;
    }
    vars.clear();
    lexeme = scanner.lex();
    if (lexeme.tag == Tag::SEMICOLON)
      scanner.next();
  }
  if (lexeme.tag != Tag::RIGHT_PARENTHESIS)
    err(")");

  return std::make_tuple(resTable, sstream.str());
};

//TODO!
void compiler::Parser::checkType ( pSymVar& res, pSymVar& src ) {
  pSym type = res->type;

  while (type != nullptr && type->symType == SymEnum::Alias)
    type = std::dynamic_pointer_cast<TypeAlias>(res->type)->type;

  if (res->type != nullptr && res->type->name != src->type->name)
    err("MISMATCH OF TYPES");

  res->value = src->name;
};

//TODO!
void compiler::Parser::checkConst( pExpr& node ) {
//
//   switch (node->exprType) {
//     case (ExprEnum::Array):
//       errConstOp(*node);
//     break;
//     case (ExprEnum::BinOp):
//       if (node->tag == Tag::IN)
//         errConstOp(*node);
//       checkConst(std::dynamic_pointer_cast<ExprBinOp>(node)->left);
//       checkConst(std::dynamic_pointer_cast<ExprBinOp>(node)->right);
//     break;
//     case (ExprEnum::Function):
//       //Check in ctFunctions!
//     case (ExprEnum::Identifier):
//       //Check glob(must to be const)
//     break;
//     case (ExprEnum::Integer):
//     case (ExprEnum::Real):
//     break;
//     case (ExprEnum::UnOp):
//       if (node->tag == Tag::ADDRESS) errConstOp(*node);
//       checkConst(std::dynamic_pointer_cast<ExprUnOp>(node)->arg);
//     break;
//     case (ExprEnum::Record):
//       errConstOp(*node);
//     break;
//   }
};

//TODO!
void compiler::Parser::checkFunc ( const std::string& name, IdentifierType type, const std::string& args ) {};
