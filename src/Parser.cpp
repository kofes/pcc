#include "Parser.hpp"
//TODO: RM iostream!
#include <iostream>

compiler::Parser::Parser ( void ) {
  setPriorities();
  setTypeTable();
};

compiler::Parser::Parser ( const std::string& filename ) : scanner(filename) {
  setPriorities();
  setTypeTable();
};

void compiler::Parser::set ( const std::string& filename ) {
  scanner.open(filename);
};

void compiler::Parser::parseExpr ( void ) {
  scanner.nextLex();
  root = parseExpr(compiler::Priority::LOWEST);
};

void compiler::Parser::parse ( void ) {
  programTokenChecked = false;
  scanner.nextLex();
  while (true) {
    switch (scanner.lex().tag) {
      //CHECKED
      case (compiler::Tag::PROGRAM) : programTokenChecked ? err(scanner.lex()) : parseProgramName(); break;
      //TODO!
      case (compiler::Tag::TYPE) : parseAlias(varTable, typeTable); break;
      //IN WORK!
      case (compiler::Tag::CONST) : parseConst(varTable, typeTable); break;
      case (compiler::Tag::VAR) : parseVar(varTable, typeTable); break;
      case (compiler::Tag::FUNCTION) : parseFunction(); break;
      case (compiler::Tag::PROCEDURE) : parseProcedure(); break;
      case (compiler::Tag::BEGIN) : root = parseBlock(); return;
      default : err("BEGIN STATEMENT");
    }
    programTokenChecked = true;
  }
};

compiler::pStmt compiler::Parser::parseStmt ( void ) {
  switch (scanner.lex().tag) {
    case (compiler::Tag::IF) : return parseIf();
    case (compiler::Tag::WHILE) : return parseWhile();
    case (compiler::Tag::REPEAT) : return parseRepeat();
    case (compiler::Tag::FOR) : return parseFor();
    case (compiler::Tag::SEMICOLON) : return parseEmpty();
    case (compiler::Tag::BEGIN) : return parseBlock();
    default : return std::dynamic_pointer_cast<compiler::Stmt>(std::dynamic_pointer_cast<compiler::Node>(parseExpr(compiler::Priority::LOWEST)));
  }
  err(scanner.lex());
};

compiler::pStmt compiler::Parser::parseIf ( void ){};
compiler::pStmt compiler::Parser::parseWhile ( void ){};
compiler::pStmt compiler::Parser::parseRepeat ( void ){};
compiler::pStmt compiler::Parser::parseFor ( void ){};
compiler::pStmt compiler::Parser::parseEmpty ( void ){};
compiler::pStmt compiler::Parser::parseBlock ( void ){};

compiler::pSym compiler::Parser::parseType ( compiler::Lexeme& lexeme, bool eq ) {

  switch (lexeme.tag) {
    case (compiler::Tag::ARRAY) :
      while (lexeme.tag == compiler::Tag::ARRAY) {
        compiler::TypeArray vArray(lexeme);

        scanner.nextLex();
        lexeme = scanner.lex();
        //ONLY STATIC ARRAY!
        if (lexeme.tag != compiler::Tag::LEFT_BRACKET)
          err("[");

        scanner.nextLex();
        lexeme = scanner.lex();
        //LITERAL OR CONST_INTEGER!
        auto iter = varTable.find(lexeme.name);
        if (lexeme.token == compiler::Token::LITERAL && lexeme.tag == compiler::Tag::INTEGER)
          vArray.low = std::strtoll(lexeme.name.c_str(), nullptr, 10);
        else if (iter != varTable.end() && iter->second->glob == compiler::GLOB::CONST && iter->second->tag == compiler::Tag::INTEGER) {//TODO: CHECK CONST_INTEGER!
          vArray.low = std::strtoll(iter->second->value.c_str(), nullptr, 10);//TEST VALUE!
        } else
          err("integer literal/const integer");

        scanner.nextLex();
        lexeme = scanner.lex();
        if (lexeme.tag != compiler::Tag::DOUBLE_DOT)
          err("..");

        scanner.nextLex();
        lexeme = scanner.lex();
        //LITERAL OR CONST_INTEGER!
        iter = varTable.find(lexeme.name);
        if (lexeme.token == compiler::Token::LITERAL && lexeme.tag == compiler::Tag::INTEGER)
          vArray.high = std::strtoll(lexeme.name.c_str(), nullptr, 10);
        else if (iter != varTable.end() && iter->second->glob == compiler::GLOB::CONST && iter->second->tag == compiler::Tag::INTEGER) {//TODO: CHECK CONST_INTEGER!
          vArray.high = std::strtoll(iter->second->value.c_str(), nullptr, 10);//TEST VALUE!
        } else
          err("integer literal/const integer");
        if (vArray.high <= vArray.low)
          throw ExprException("array 'high' <= 'low' in pos(" + std::to_string(vArray.row) + ", " + std::to_string(vArray.column) + ");");

        scanner.nextLex();
        lexeme = scanner.lex();
        //ONLY STATIC ARRAY!
        if (lexeme.tag != compiler::Tag::RIGHT_BRACKET)
          err("]");

        scanner.nextLex();
        lexeme = scanner.lex();
        if (lexeme.tag != compiler::Tag::OF)
          err("of");

        //TODO: CIRCLING 'ARRAY[...] OF' AND '[a..b,c..d,...,N..M]' - check!
      }
    break;
    // case (compiler::Tag::RECORD) :
    // break;
    // case (compiler::Tag::SET) :
    default :
      auto iter = typeTable.find(lexeme.name);
      if (iter == typeTable.end())
        errUndefType();
  }

};

void compiler::Parser::parseProgramName ( void ){
  compiler::Lexeme programName = scanner.lex();
  scanner.nextLex();
  compiler::Lexeme lexeme = scanner.lex();
  if (lexeme.token != compiler::Token::IDENTIFIER || lexeme.tag != compiler::Tag::UNDEFINED)
    err("PROGRAM NAME");

  std::shared_ptr<compiler::SymVar> var = std::shared_ptr<compiler::SymVar>(new SymVar(programName));
  var->glob = compiler::GLOB::GLOBAL;
  var->type = std::shared_ptr<compiler::SymType>(new SymType(programName));
  varTable[lexeme.name] = var;

  scanner.nextLex();
  lexeme = scanner.lex();
  if (lexeme.tag != compiler::Tag::SEMICOLON)
    err("SEMICOLON");
  scanner.nextLex();
};

void compiler::Parser::parseConst ( SymTable& vTable, TypeTable& tTable ) {
  scanner.nextLex();
  compiler::Lexeme lexeme = scanner.lex();

  checkIdent(lexeme, vTable, tTable);;

  for (;lexeme.token == compiler::Token::IDENTIFIER && lexeme.tag == compiler::Tag::UNDEFINED;
       scanner.nextLex(), lexeme = scanner.lex()) {

    checkIdent(lexeme, vTable, tTable);

    compiler::pSymVar var = pSymVar(new SymVar(lexeme));

    scanner.nextLex();
    lexeme = scanner.lex();

    var->type = nullptr;

    if (lexeme.tag == compiler::Tag::COLON)
      var->type = parseType(lexeme, compiler::Init::YES);
    //NOW scanner.lex().name == '='

    scanner.nextLex();
    lexeme = scanner.lex();

    var->value = parseInit(type);

    //NOW scanner.lex().name == ';'

    // SymTable::iterator iter;
    // switch (lexeme.tag) {
    //   case (compiler::Tag::EQUALS) :
    //     scanner.nextLex();
    //     lexeme = scanner.lex();
    //     iter = varTable.find(lexeme.name);
    //     if (lexeme.token == compiler::Token::LITERAL) {
    //       switch (lexeme.tag) {
    //         case (compiler::Tag::NIL) : var->type = compiler::pSym(new TypeScalar(SCALAR_TYPE::POINTER)); break;
    //         case (compiler::Tag::INTEGER) : var->type = compiler::pSym(new TypeScalar(SCALAR_TYPE::INTEGER)); break;
    //         case (compiler::Tag::FLOAT) : var->type = compiler::pSym(new TypeScalar(SCALAR_TYPE::REAL)); break;
    //         case (compiler::Tag::CHARACTER) : var->type = compiler::pSym(new TypeScalar(SCALAR_TYPE::CHAR)); break;
    //         case (compiler::Tag::B_TRUE) : var->type = compiler::pSym(new TypeScalar(SCALAR_TYPE::BOOLEAN)); break;
    //         case (compiler::Tag::B_FALSE) : var->type = compiler::pSym(new TypeScalar(SCALAR_TYPE::BOOLEAN)); break;
    //         default : err("supported literal");
    //       }
    //     } else if (iter != varTable.end() && iter->second->glob == compiler::GLOB::CONST) {//TODO: CHECK CONST!
    //       var->type = iter->second->type;
    //     } else
    //       err("literal with ordinary type/const");
    //   break;
    //   default : err("=");
    // }

    var->glob = compiler::GLOB::CONST;
    vTable[var->name] = var;

    scanner.nextLex();
    lexeme = scanner.lex();
  }
};

void compiler::Parser::parseVar ( SymTable& vTable, TypeTable& tTable ) {
  std::vector<pSymVar> var;

  scanner.nextLex();
  compiler::Lexeme lexeme = scanner.lex();

  checkIdent(lexeme, vTable, tTable);

  for (;lexeme.token == compiler::Token::IDENTIFIER && lexeme.tag == compiler::Tag::UNDEFINED;
       scanner.nextLex(), lexeme = scanner.lex()) {

    checkIdent(lexeme, vTable, tTable);

    var.push_back(pSymVar(new SymVar(lexeme)));

    scanner.nextLex();
    lexeme = scanner.lex();

    for (;lexeme.tag == compiler::Tag::COMMA; scanner.nextLex(), lexeme = scanner.lex()) {
      scanner.nextLex();
      lexeme = scanner.lex();

      checkIdent(lexeme, vTable, tTable);
      var.push_back(pSymVar(new SymVar(lexeme)));
    }

    if (lexeme.tag != compiler::Tag::COLON)
      err("':'");

    scanner.nextLex();
    lexeme = scanner.lex();

    compiler::pSym type = parseType(lexeme, compiler::Init::MAYBE);

    if (scanner.lex().tag == compiler::Tag::COLON) {
      if (var.size() != 1)
        err("';'");
      var.front()->value = parseInit(type);
    }

    for (auto elem : var) {
      elem->type = type;
      elem->glob = compiler::GLOB::GLOBAL;
      vTable[elem->name] = elem;
    }
  }
};

void compiler::Parser::parseFunction ( void ) {
  scanner.next();
  compiler::Lexeme lexeme = scanner.lex();
  //Pascal feature: function result variable has name like a name of function
  checkIdent(lexeme, varTable, typeTable);

  std::shared_ptr<compiler::SymFunc> func = new compiler::SymFunc(lexeme);

  scanner.next();
  lexeme = scanner.lex();

  if (lexeme.tag != compiler::Tag::LEFT_PARENTHESIS)
    err("'('");

  func->params = parseParams();

  //NOW scanner.lex().name == ")"

  scanner.nextLex();
  lexeme = scanner.lex();

  if (lexeme.tag != compiler::Tag::COLON)
    err("':'");

  scanner.next();
  lexeme = scanner.lex();

  func->retType = parseType(lexeme, compiler::Init::NO);

  //NOW scanner.lex().name == ";"

  scanner.next();
  lexeme = scanner.lex();

  while (lexeme.tag != compiler::Tag::BEGIN) {
    switch (lexeme.tag) {
      case (compiler::Tag::TYPE) : parseType(func->varTable, func->typeTable);
      case (compiler::Tag::VAR) : parseVar(func->varTable, func->typeTable);
      case (compiler::Tag::Const) : parseConst(func->varTable, func->typeTable);
      default: err("Begin");
    }
  }

  //NOW scanner.lex().name == "BEGIN"

  func->body = parseBlock();

  //NOW scanner.lex().name == "END"

  scanner.nextLex();
  lexeme = scanner.lex();

  if (lexeme.tag != compiler::Tag::SEMICOLON)
    err("';'");

  funcTable[func->name] = func;

  scanner.nextLex();
  lexeme = scanner.lex();
};

void compiler::Parser::parseProcedure ( void ) {
  scanner.next();
  compiler::Lexeme lexeme = scanner.lex();

  //Pascal feature: function result variable has name like a name of function

  checkIdent(lexeme, varTable, typeTable);

  std::shared_ptr<compiler::SymFunc> func = new compiler::SymFunc(lexeme);

  scanner.next();
  lexeme = scanner.lex();

  if (lexeme.tag != compiler::Tag::LEFT_PARENTHESIS)
    err("'('");

  func->params = parseParams();

  //NOW scanner.lex().name == ")"

  scanner.nextLex();
  lexeme = scanner.lex();

  if (lexeme.tag != compiler::Tag::SEMICOLON)
    err("';'");

  scanner.next();
  lexeme = scanner.lex();

  while (lexeme.tag != compiler::Tag::BEGIN) {
    switch (lexeme.tag) {
      case (compiler::Tag::TYPE) : parseType(func->varTable, func->typeTable);
      case (compiler::Tag::VAR) : parseVar(func->varTable, func->typeTable);
      case (compiler::Tag::Const) : parseConst(func->varTable, func->typeTable);
      default: err("Begin");
    }
  }

  //NOW scanner.lex().name == "BEGIN"

  func->body = parseBlock();

  //NOW scanner.lex().name == "END"

  scanner.nextLex();
  lexeme = scanner.lex();

  if (lexeme.tag != compiler::Tag::SEMICOLON)
    err("';'");

  funcTable[func->name] = func;

  scanner.nextLex();
  lexeme = scanner.lex();
};

void compiler::Parser::parseAlias ( SymTable& vTable, TypeTable& tTable ) {
  scanner.nextLex();
  compiler::Lexeme lexeme = scanner.lex();

  checkIdent(lexeme, vTable, tTable);

  while (lexeme.token == compiler::Token::IDENTIFIER && lexeme.tag == compiler::Tag::UNDEFINED) {
    compiler::Lexeme ident = lexeme;
    SymType type;

    checkIdent(lexeme, vTable, tTable);

    scanner.nextLex();
    lexeme = scanner.lex();

    if (lexeme.tag != compiler::Tag::EQUALS)
      err("'='");

    scanner.next();
    lexeme = scanner.lex();

    switch (lexeme.tag) {
      case (compiler::Tag::RECORD): type = parseRecord();
      case (compiler::Tag::LEFT_PARENTHESIS): type = parseEnum();
      default: type = parseType(lexeme, compiler::Init::NO);
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
    scanner.nextLex();
    compiler::pExpr right = parseExpr(upPriority(priority));
    left = compiler::pExpr(new compiler::ExprBinOp(lexeme, left, right));
    lexeme = scanner.lex();
  }
  return left;
};

compiler::pExpr compiler::Parser::parseFactor ( void ) {
  compiler::pExpr tmpExpr;
  compiler::Lexeme lexeme = scanner.lex();
  scanner.nextLex();

  if (lexeme.token == compiler::Token::IDENTIFIER && lexeme.tag == compiler::Tag::UNDEFINED)
    return parseIdentifier(lexeme);
  if (isUnary(lexeme.tag))
    return compiler::pExpr(new ExprUnOp(lexeme, parseExpr(Priority::HIGHEST)));

  switch (lexeme.tag) {
    case (compiler::Tag::INTEGER) : return compiler::pExpr(new ExprInteger(lexeme));
    case (compiler::Tag::FLOAT) : return compiler::pExpr(new ExprReal(lexeme));
    case (compiler::Tag::LEFT_PARENTHESIS) :
      tmpExpr = parseExpr(compiler::Priority::LOWEST);
      lexeme = scanner.lex();

      if (lexeme.tag != compiler::Tag::RIGHT_PARENTHESIS) {
        if (lexeme.token == compiler::Token::END_OF_FILE) err();
        err(")");
      }

      scanner.nextLex();
    return tmpExpr;
    default: break;
  }

  if (lexeme.token == compiler::Token::END_OF_FILE) err();
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
      case (compiler::Tag::DOT):
        tmpLex = lexeme;
        scanner.nextLex();
        lexeme = scanner.lex();
        if (lexeme.token == compiler::Token::END_OF_FILE) err();
        if (lexeme.token != compiler::Token::IDENTIFIER || lexeme.tag != compiler::Tag::UNDEFINED) err("IDENTIFIER");
        right = compiler::pExpr(new ExprIdentifier(lexeme));
        left = compiler::pExpr(new ExprRecordAccess(tmpLex, left, right));
      break;
      case (compiler::Tag::LEFT_BRACKET):
        tmpLex = lexeme;
        args = parseArrayIndex();
        left = compiler::pExpr(new ExprArrayIndex(tmpLex, left, args));
        lexeme = scanner.lex();
        if (lexeme.token == compiler::Token::END_OF_FILE) err();
        if (lexeme.tag != compiler::Tag::RIGHT_BRACKET) err("]");
      break;
      case (compiler::Tag::POINTER):
        left = compiler::pExpr(new ExprUnOp(lexeme, left));
      break;
      default: return left;
    }
    scanner.nextLex();
  }
  return left;
};

std::string compiler::Parser::print ( void ) {

  if (scanner.lex().token != compiler::Token::END_OF_FILE) err(scanner.lex());//Program's end is 'end.'

  std::ostringstream out;
  out << root->print(0);
  return out.str();
};

std::vector<compiler::pExpr> compiler::Parser::parseArrayIndex ( void ) {
  std::vector<compiler::pExpr> expr;
  compiler::Lexeme lexeme;
  scanner.nextLex();
  expr.push_back(parseExpr(compiler::Priority::LOWEST));
  lexeme = scanner.lex();
  while (lexeme.tag == compiler::Tag::COMMA) {
    scanner.nextLex();
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

bool compiler::Parser::checkPriority ( const compiler::Priority& pr, const compiler::Tag& tag ) {
  auto tmp = binaryPriority.find(tag);
  return tmp != binaryPriority.end() && tmp->second == pr;
};

bool compiler::Parser::isUnary ( const compiler::Tag& tag ) {
  return unaryPriority.find(tag) != unaryPriority.end();
};

void compiler::Parser::err ( const std::string& expected_token ) {
  compiler::Lexeme lexeme = scanner.lex();
  if (expected_token == "")
    throw ExprException("Unexpected end of file in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + ");");
  if (lexeme.token == compiler::Token::END_OF_FILE)
    throw ExprException("Unexpected end of file in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + "); expected '" + expected_token + "';");
  throw ExprException("Unexpected token '" + lexeme.name + "' in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + "); expected '" + expected_token + "';");
};

void compiler::Parser::err ( const compiler::Lexeme& lexeme ) {
  if (lexeme.token == compiler::Token::END_OF_FILE)
    throw ExprException("Unexpected end of file in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + ");");
  throw ExprException("Unexpected token '" + lexeme.name + "' in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + ");");
};

void compiler::Parser::errUndefType ( void ) {
  compiler::Lexeme lexeme = scanner.lex();
  if (lexeme.token == compiler::Token::END_OF_FILE)
    throw ExprException("Unexpected end of file in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + "); expected type name;");
  throw ExprException("Undefined type '" + lexeme.name + "' in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + ");");
};

void compiler::Parser::errDuplicated ( void ) {
  compiler::Lexeme lexeme = scanner.lex();
  throw ExprException("Duplicate identifier \"" + lexeme.name + "\" in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + ");");
};

void compiler::Parser::checkIdent ( const Lexeme& lexeme, SymTable& vTable, TypeTable& tTable ) {
  if (lexeme.token != compiler::Token::IDENTIFIER || lexeme.tag != compiler::Tag::UNDEFINED)
    err("Identifier");
  if (varTable.find(lexeme.name) != varTable.end())
    errDuplicated();
  if (vTable.find(lexeme.name) != vTable.end())
    errDuplicated();
  if (typeTable.find(lexeme.name) != typeTable.end())
    errDuplicated();
  if (tTable.find(lexeme.name) != tTable.end())
    errDuplicated();
};

void compiler::Parser::setPriorities ( void ) {
  binaryPriority = {
    {compiler::Tag::EQUALS, compiler::Priority::LOWEST},
    {compiler::Tag::MIS, compiler::Priority::LOWEST},
    {compiler::Tag::GREATER, compiler::Priority::LOWEST},
    {compiler::Tag::LESS, compiler::Priority::LOWEST},
    {compiler::Tag::GREATER_EQ, compiler::Priority::LOWEST},
    {compiler::Tag::LESS_EQ, compiler::Priority::LOWEST},
    {compiler::Tag::IN, compiler::Priority::LOWEST},

    {compiler::Tag::ADD, compiler::Priority::THIRD},
    {compiler::Tag::SUB, compiler::Priority::THIRD},
    {compiler::Tag::OR, compiler::Priority::THIRD},
    {compiler::Tag::XOR, compiler::Priority::THIRD},

    {compiler::Tag::MUL, compiler::Priority::SECOND},
    {compiler::Tag::DIV_FLOAT, compiler::Priority::SECOND},
    {compiler::Tag::DIV_INT, compiler::Priority::SECOND},
    {compiler::Tag::MOD, compiler::Priority::SECOND},
    {compiler::Tag::AND, compiler::Priority::SECOND},
    {compiler::Tag::SHL, compiler::Priority::SECOND},
    {compiler::Tag::SHR, compiler::Priority::SECOND}
  };

  unaryPriority = {
    {compiler::Tag::NOT, compiler::Priority::HIGHEST},
    {compiler::Tag::SUB, compiler::Priority::HIGHEST},
    {compiler::Tag::ADD, compiler::Priority::HIGHEST},
    {compiler::Tag::ADDRESS, compiler::Priority::HIGHEST}
  };
};

void compiler::Parser::setTypeTable ( void ) {
  typeTable[tagBook.at(compiler::Tag::POINTER)] = compiler::pSym(new TypeScalar(compiler::SCALAR_TYPE::POINTER));
  typeTable[tagBook.at(compiler::Tag::INTEGER)] = compiler::pSym(new TypeScalar(compiler::SCALAR_TYPE::INTEGER));
  typeTable[tagBook.at(compiler::Tag::REAL)] = compiler::pSym(new TypeScalar(compiler::SCALAR_TYPE::REAL));
  typeTable[tagBook.at(compiler::Tag::CHAR)] = compiler::pSym(new TypeScalar(compiler::SCALAR_TYPE::CHAR));
  typeTable[tagBook.at(compiler::Tag::BOOLEAN)] = compiler::pSym(new TypeScalar(compiler::SCALAR_TYPE::BOOLEAN));
};
