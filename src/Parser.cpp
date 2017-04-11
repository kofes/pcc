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
      case (compiler::Tag::PROGRAM) : programTokenChecked ? err(scanner.lex()) : parseProgramName(); break;
      case (compiler::Tag::TYPE) : parseAlias(); break;
      case (compiler::Tag::CONST) : parseConst(); break;
      case (compiler::Tag::VAR) : parseVar(); break;
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
        if (lexeme.token == compiler::Tag::LITERAL && lexeme.tag == compiler::Tag::INTEGER)
          vArray.low = std::strtoll(lexeme.name.c_str(), nullptr, 10);
        else if (iter != varTable.end() && iter.second->glob == compiler::GLOB::GLOBAL && iter.second->pSym->tag == INTEGER) {//TODO: CHECK CONST_INTEGER!
          vArray.low = std::strtoll(iter.second->value.c_str(), nullptr, 10);//TEST VALUE!
        } else
          err("integer literal/const integer");

        scanner.nextLex();
        lexeme = scanner.lex();
        if (lexeme.tag != compiler::Tag::DOUBLE_DOT)
          err("..");

        scanner.nextLex();
        lexeme = scanner.lex();
        //LITERAL OR CONST_INTEGER!
        auto iter = varTable.find(lexeme.name);
        if (lexeme.token == compiler::Tag::LITERAL && lexeme.tag == compiler::Tag::INTEGER)
          vArray.high = std::strtoll(lexeme.name.c_str(), nullptr, 10);
        else if (iter != varTable.end() && iter.second->glob == compiler::GLOB::GLOBAL && iter.second->pSym->tag == INTEGER) {//TODO: CHECK CONST_INTEGER!
          vArray.high = std::strtoll(iter.second->value.c_str(), nullptr, 10);//TEST VALUE!
        } else
          err("integer literal/const integer");
        if (vArray.high <= vArray.low)
          throw ExprException("array 'high' <= 'low' in pos(" + std::to_string(vArray.row) + ", " + std::tostring(vArray.column) + ");");

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
  else {
    std::shared_ptr<compiler::SymVar> var = std::shared_ptr<compiler::SymVar>(new SymVar(programName));
    var->glob = compiler::GLOB::GLOBAL;
    var->type = std::shared_ptr<compiler::SymType>(new SymType(programName));
    varTable[lexeme.name] = var;
  }
  scanner.nextLex();
  lexeme = scanner.lex();
  if (lexeme.tag != compiler::Tag::SEMICOLON)
    err("SEMICOLON");
  scanner.nextLex();
};

void compiler::Parser::parseConst ( void ) {
  scanner.nextLex();
  compiler::Lexeme lexeme = scanner.lex();

  if (lexeme.token != compiler::Token::IDENTIFIER || lexeme.tag != compiler::Tag::UNDEFINED)
    err("IDENTIFIER");

    for (;lexeme.token == compiler::Token::IDENTIFIER && lexeme.tag == compiler::Tag::UNDEFINED;
         scanner.nextLex(), lexeme = scanner.lex()) {

      if (varTable.find(lexeme.name) != varTable.end())
        errDuplicated();

      compiler::pSymVar var = pSymVar(new SymVar(lexeme));
      var->glob = compiler::GLOB::CONST;
      scanner.nextLex();
      lexeme = scanner.lex();

      switch (lexeme.tag) {
        case (compiler::Tag::EQUALS) :
          scanner.nextLex();
          lexeme = scanner.lex();
          auto iter = varTable.find(lexeme.name);
          if (lexeme.token == compiler::Token::LITERAL) {
            switch (lexeme.tag) {
              case (compiler::Tag::NIL) : var->type = compiler::pSym(new TypeScalar(compiler::Tag::POINTER)); break;
              case (compiler::Tag::INTEGER) : var->type = compiler::pSym(new TypeScalar(lexeme.tag)); break;
              case (compiler::Tag::FLOAT) : var->type = compiler::pSym(new TypeScalar(compiler::Tag::REAL)); break;
              case (compiler::Tag::CHARACTER) : var->type = compiler::pSym(new TypeScalar(compiler::Tag::CHAR)); break;
              case (compiler::Tag::B_TRUE) : var->type = compiler::pSym(new TypeScalar(compiler::Tag::BOOLEAN)); break;
              case (compiler::Tag::B_FALSE) : var->type = compiler::pSym(new TypeScalar(compiler::Tag::BOOLEAN)); break;
              default : err("supported literal");
            }
          } else if (iter != varTable.end() && iter.second->glob == compiler::GLOB::GLOBAL) {//TODO: CHECK CONST!
            var->type = iter->second->type;
          } else
            err("literal with ordinary type/const");
        break;
        case (compiler::Tag::COLON) ://TODO: LIKE VARIABLLE!
          scanner.nextLex();
          lexeme = scanner.lex();
          var->type = parseType(lexeme, true);
        break;
        default : err("=");
      }
      varTable[var->name] = var;

      scanner.nextLex();
      lexeme = scanner.lex();

      if (lexeme.tag != compiler::Tag::SEMICOLON)
        err("';'");
    }
};

void compiler::Parser::parseVar ( void ) {

  scanner.nextLex();
  compiler::Lexeme lexeme = scanner.lex();

  if (lexeme.token != compiler::Token::IDENTIFIER || lexeme.tag != compiler::Tag::UNDEFINED)
    err("IDENTIFIER");

  for (;lexeme.token == compiler::Token::IDENTIFIER && lexeme.tag == compiler::Tag::UNDEFINED;
       scanner.nextLex(), lexeme = scanner.lex()) {

    if (varTable.find(lexeme.name) != varTable.end())
      errDuplicated();

    std::vector<pSymVar> var;

    var.push_back(pSymVar(new SymVar(lexeme)));
    var.back()->glob = compiler::GLOB::GLOBAL;
    scanner.nextLex();
    lexeme = scanner.lex();

    for (;lexeme.tag == compiler::Tag::COMMA; scanner.nextLex(), lexeme = scanner.lex()) {
      scanner.nextLex();
      lexeme = scanner.lex();

      if (lexeme.token != compiler::Token::IDENTIFIER || lexeme.tag != compiler::Tag::UNDEFINED)
        err("IDENTIFIER");

      if (varTable.find(lexeme.name) != varTable.end())
        errDuplicated();

      var.push_back(pSymVar(new SymVar(lexeme)));
      var.back()->glob = compiler::GLOB::GLOBAL;
    }

    if (lexeme.tag != compiler::Tag::COLON)
      err("':'");

    scanner.nextLex();
    compiler::Lexeme lexeme = scanner.lex();
    compiler::pSym type = parseType(lexeme, false);

    for (auto elem : var) {
      elem->type = type;
      varTable[elem->name] = elem;
    }

    scanner.nextLex();
    lexeme = scanner.lex();

    if (lexeme.tag != compiler::Tag::SEMICOLON)
      err("';'");
  }
};

void compiler::Parser::parseFunction ( void ){};
void compiler::Parser::parseProcedure ( void ){};
void compiler::Parser::parseAlias ( void ){};

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
