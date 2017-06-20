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
  scanner.nextLex();
  root = parseExpr(compiler::Priority::LOWEST);
};

void compiler::Parser::parse ( void ) {
  programTokenChecked = false;
  scanner.nextLex();
  Lexeme lexeme = scanner.lex();
  while (true) {
    switch (scanner.lex().tag) {
      case (Tag::PROGRAM) : programTokenChecked ? err(scanner.lex()) : parseProgramName(lexeme); break;
      //TODO!
      case (Tag::CONST) : parseConst(varTable, typeTable); break;
      //IN WORK!
      case (Tag::TYPE) : parseAlias(varTable, typeTable); break;
      case (Tag::VAR) : parseVar(varTable, typeTable); break;
      case (Tag::FUNCTION) : parseFunction(); break;
      case (Tag::PROCEDURE) : parseProcedure(); break;
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
    default : return std::dynamic_pointer_cast<Stmt>(std::dynamic_pointer_cast<Node>(parseExpr(Priority::LOWEST)));
  }
  err(scanner.lex());
};

compiler::pStmt compiler::Parser::parseIf ( void ){};
compiler::pStmt compiler::Parser::parseWhile ( void ){};
compiler::pStmt compiler::Parser::parseRepeat ( void ){};
compiler::pStmt compiler::Parser::parseFor ( void ){};
compiler::pStmt compiler::Parser::parseEmpty ( void ){};
compiler::pStmt compiler::Parser::parseBlock ( void ){};

//TODO!
compiler::pSym compiler::Parser::parseType ( compiler::Lexeme& lexeme, SymTable& vTable, TypeTable& tTable, InitExpected init ) {
  pSym type;
  std::shared_ptr<TypeArray> vArray = nullptr;
  switch (lexeme.tag) {
    case (Tag::ARRAY) :
      type = pSym(new TypeArray(lexeme));
      while (lexeme.tag == Tag::ARRAY) {

        scanner.nextLex();
        lexeme = scanner.lex();
        //ONLY STATIC ARRAY!
        if (lexeme.tag != Tag::LEFT_BRACKET)
          err("[");

        do {
          if (vArray == nullptr)
            vArray = std::dynamic_pointer_cast<TypeArray>(type);
          else {
            lexeme.name = "ARRAY";
            lexeme.tag = Tag::ARRAY;
            lexeme.token = Token::IDENTIFIER;
            vArray->elemType = pSym(new TypeArray(lexeme));
            vArray = std::dynamic_pointer_cast<TypeArray>(vArray->elemType);
          }
          //DEBUG
          std::cout << "ARRAY:" << vArray->name << std::endl;
          //
          scanner.nextLex();
          lexeme = scanner.lex();

          //Parse LOW
          pExpr local_root = parseExpr(Priority::LOWEST);
          checkConst(local_root);
          pSymVar tmp = pSymVar(evalConstExpr(local_root, vTable, tTable));
          if (tmp->tag != Tag::INTEGER)
            err("integer literal/const integer");
          vArray->low = std::strtoll(tmp->name.c_str(), nullptr, 10);

          if (scanner.lex().tag != Tag::DOUBLE_DOT)
            err("..");

          scanner.nextLex();
          lexeme = scanner.lex();
          //Parse HIGH
          local_root = parseExpr(Priority::LOWEST);
          checkConst(local_root);
          tmp = pSymVar(evalConstExpr(local_root, vTable, tTable));
          if (tmp->tag != Tag::INTEGER)
            err("integer literal/const integer");
          vArray->high = std::strtoll(tmp->name.c_str(), nullptr, 10);

          if (vArray->high <= vArray->low)
            throw ExprException("array 'high' <= 'low' in pos(" + std::to_string(vArray->row) + ", " + std::to_string(vArray->column) + ");");
        } while (scanner.lex().tag == Tag::COMMA);

        //ONLY STATIC ARRAY!
        if (scanner.lex().tag != Tag::RIGHT_BRACKET)
          err("]");

        scanner.nextLex();
        lexeme = scanner.lex();

        if (lexeme.tag != Tag::OF)
          err("OF");

        scanner.nextLex();
        lexeme = scanner.lex();

      }
    break;
    // case (Tag::SET) :
    default :
      auto iter = typeTable.find(lexeme.name);
      if (iter == typeTable.end())
        errUndefType();
      type = iter->second;
  }

  scanner.nextLex();
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
compiler::pSym compiler::Parser::parseRecord ( void ) {};
compiler::pSym compiler::Parser::parseEnum ( void ) {};

void compiler::Parser::parseProgramName ( const compiler::Lexeme& program ) {
  scanner.nextLex();
  compiler::Lexeme lexeme = scanner.lex();
  if (lexeme.token != Token::IDENTIFIER || lexeme.tag != Tag::UNDEFINED)
    err("PROGRAM NAME");

  std::shared_ptr<compiler::SymVar> var = std::shared_ptr<compiler::SymVar>(new SymVar(program));
  var->glob = compiler::GLOB::GLOBAL;
  var->type = std::shared_ptr<compiler::SymType>(new SymType(program));
  varTable[lexeme.name] = var;

  scanner.nextLex();
  lexeme = scanner.lex();
  if (lexeme.tag != Tag::SEMICOLON)
    err("SEMICOLON");
  scanner.nextLex();
};

void compiler::Parser::parseConst ( SymTable& vTable, TypeTable& tTable ) {
  scanner.nextLex();
  Lexeme lexeme = scanner.lex();

  for (; lexeme.token == Token::IDENTIFIER && lexeme.tag == Tag::UNDEFINED;
         scanner.nextLex(), lexeme = scanner.lex()) {

    checkIdent(lexeme, vTable, tTable);
    checkFunc(lexeme, IdentifierType::VARIABLE);

    pSymVar var = pSymVar(new SymVar(lexeme));

    scanner.nextLex();
    lexeme = scanner.lex();

    var->type = nullptr;

    switch (lexeme.tag) {
      case(Tag::COLON) :
        var->type = parseType(lexeme, vTable, tTable, InitExpected::YES);
      case (Tag::EQUALS) :
      break;
      default :
        err("=");
      break;
    }
    //NOW SCANNER == '='
    scanner.nextLex();
    lexeme = scanner.lex();

    //NOW PARSE CONST EXPR
    pExpr local_root = parseExpr(Priority::LOWEST);
    checkConst(local_root);
    pSymVar tmp = pSymVar(evalConstExpr(local_root, vTable, tTable));
    checkType(var, tmp);

    //NOW scanner.lex().name is ';'

    var->glob = compiler::GLOB::CONST;
    vTable[var->name] = var;//TO UPPERCASE!

    scanner.nextLex();
    lexeme = scanner.lex();
  }
};

void compiler::Parser::parseVar ( SymTable& vTable, TypeTable& tTable ) {
  std::vector<pSymVar> var;

  scanner.nextLex();
  Lexeme lexeme = scanner.lex();

  for (;lexeme.token == Token::IDENTIFIER && lexeme.tag == Tag::UNDEFINED;
    scanner.nextLex(), lexeme = scanner.lex()) {
    checkIdent(lexeme, vTable, tTable);
    checkFunc(lexeme, IdentifierType::VARIABLE);

    var.push_back(pSymVar(new SymVar(lexeme)));

    scanner.nextLex();
    lexeme = scanner.lex();

    for (;lexeme.tag == Tag::COMMA; scanner.nextLex(), lexeme = scanner.lex()) {
      scanner.nextLex();
      lexeme = scanner.lex();

      checkIdent(lexeme, vTable, tTable);
      checkFunc(lexeme, IdentifierType::VARIABLE);
      var.push_back(pSymVar(new SymVar(lexeme)));
    }

    if (lexeme.tag != Tag::COLON)
      err("':'");

    scanner.nextLex();
    lexeme = scanner.lex();

    compiler::pSym type = parseType(lexeme, vTable, tTable, InitExpected::MAYBE);

    if (scanner.lex().tag == Tag::EQUALS) {
      if (var.size() != 1)
        err("';'");
      scanner.nextLex();
      lexeme = scanner.lex();
      pExpr local_root = parseExpr(Priority::LOWEST);
      // checkConst(local_root);
      // pSymVar tmp = pSymVar(evalConstExpr(local_root, vTable, tTable));
      // checkType(var.front(), tmp);
    }

    for (pSymVar& elem : var) {
      elem->type = type;
      elem->glob = compiler::GLOB::GLOBAL;
      vTable[elem->name] = elem;
    }
    var.clear();
    if (scanner.lex().tag != Tag::SEMICOLON)
      err("';'");
  }
};

void compiler::Parser::parseFunction ( void ) {
  scanner.nextLex();
  Lexeme lexeme = scanner.lex();

  //Pascal feature: function result variable has name like a name of function
  checkIdent(lexeme, varTable, typeTable);

  std::shared_ptr<SymFunc> func(new SymFunc(lexeme));

  scanner.nextLex();
  lexeme = scanner.lex();
  std::string args;
  func->params = nullptr;
  if (lexeme.tag == Tag::LEFT_PARENTHESIS) {
    std::tie(func->params, args) = parseParams();
    //NOW scanner.lex().name == ")"
    scanner.nextLex();
    lexeme = scanner.lex();
  }

  checkFunc(*(func.get()), IdentifierType::FUNCTION, args);

  if (lexeme.tag != Tag::COLON)
    err("':'");

  scanner.nextLex();
  lexeme = scanner.lex();

  func->retType = parseType(lexeme, varTable, typeTable, InitExpected::NO);

  //NOW scanner.lex().name == ";"

  scanner.nextLex();
  lexeme = scanner.lex();

  while (lexeme.tag != Tag::BEGIN) {
    switch (lexeme.tag) {
      case (Tag::TYPE) : parseAlias(func->varTable, func->typeTable); break;
      case (Tag::VAR) : parseVar(func->varTable, func->typeTable); break;
      case (Tag::CONST) : parseConst(func->varTable, func->typeTable); break;
      default: err("Begin");
    }
  }

  //NOW scanner.lex().name == "BEGIN"

  func->body = parseBlock();

  //NOW scanner.lex().name == "END"

  scanner.nextLex();
  lexeme = scanner.lex();

  if (lexeme.tag != Tag::SEMICOLON)
    err("';'");

  funcTable[func->name][args] = func;

  scanner.nextLex();
  lexeme = scanner.lex();
};

void compiler::Parser::parseProcedure ( void ) {
  scanner.nextLex();
  Lexeme lexeme = scanner.lex();

  //Pascal feature: function result variable has name like a name of function
  checkIdent(lexeme, varTable, typeTable);

  std::shared_ptr<SymFunc> func(new SymFunc(lexeme));

  scanner.nextLex();
  lexeme = scanner.lex();

  std::string args;
  func->params = nullptr;
  if (lexeme.tag == Tag::LEFT_PARENTHESIS) {
    std::tie(func->params, args) = parseParams();
    //NOW scanner.lex().name == ")"
    scanner.nextLex();
    lexeme = scanner.lex();
  }

  checkFunc(*(func.get()), IdentifierType::FUNCTION, args);

  if (lexeme.tag != Tag::SEMICOLON)
    err("';'");

  scanner.nextLex();
  lexeme = scanner.lex();

  while (lexeme.tag != Tag::BEGIN) {
    switch (lexeme.tag) {
      case (Tag::TYPE) : parseAlias(func->varTable, func->typeTable); break;
      case (Tag::VAR) : parseVar(func->varTable, func->typeTable); break;
      case (Tag::CONST) : parseConst(func->varTable, func->typeTable); break;
      default: err("Begin");
    }
  }

  //NOW scanner.lex().name == "BEGIN"

  func->body = parseBlock();

  //NOW scanner.lex().name == "END"

  scanner.nextLex();
  lexeme = scanner.lex();

  if (lexeme.tag != Tag::SEMICOLON)
    err("';'");

  funcTable[func->name][args] = func;

  scanner.nextLex();
  lexeme = scanner.lex();
};

void compiler::Parser::parseAlias ( SymTable& vTable, TypeTable& tTable ) {
  scanner.nextLex();
  compiler::Lexeme lexeme = scanner.lex();
  // checkIdent(lexeme, vTable, tTable);

  while (lexeme.token == Token::IDENTIFIER && lexeme.tag == Tag::UNDEFINED) {

    checkIdent(lexeme, vTable, tTable);
    checkFunc(lexeme, IdentifierType::TYPE);

    Lexeme ident = lexeme;
    pSym type;

    scanner.nextLex();
    lexeme = scanner.lex();

    if (lexeme.tag != Tag::EQUALS)
      err("'='");

    scanner.nextLex();
    lexeme = scanner.lex();

    switch (lexeme.tag) {
      case (Tag::RECORD): type = parseRecord();
      case (Tag::LEFT_PARENTHESIS): type = parseEnum();
      default: type = parseType(lexeme, vTable, tTable, InitExpected::NO);
    }

    //NOW scanner.lex().name == ";"

    tTable[ident.name] = type;

    scanner.nextLex();
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

      scanner.nextLex();
    return tmpExpr;
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
        scanner.nextLex();
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
    scanner.nextLex();
  }
  return left;
};

std::string compiler::Parser::print ( void ) {
  std::ostringstream out;

  out << printFuncTable() << printVarTable() << printExprs();

  return out.str();
};

std::string compiler::Parser::printExprs ( void ) {
  // if (scanner.lex().token != Token::END_OF_FILE) err(scanner.lex());//Program's end is 'end.'

  std::ostringstream out;
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

std::vector<compiler::pExpr> compiler::Parser::parseArgs ( void ) {
  std::vector<compiler::pExpr> expr;
  compiler::Lexeme lexeme;
  scanner.nextLex();
  expr.push_back(parseExpr(compiler::Priority::LOWEST));
  lexeme = scanner.lex();
  while (lexeme.tag == Tag::COMMA) {
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
  typeTable[tagBook.at(Tag::POINTER)] = compiler::pSym(new TypeScalar(SCALAR_TYPE::POINTER));
  typeTable[tagBook.at(Tag::INTEGER)] = compiler::pSym(new TypeScalar(SCALAR_TYPE::INTEGER));
  typeTable[tagBook.at(Tag::REAL)] = compiler::pSym(new TypeScalar(SCALAR_TYPE::REAL));
  typeTable[tagBook.at(Tag::CHAR)] = compiler::pSym(new TypeScalar(SCALAR_TYPE::CHAR));
  typeTable[tagBook.at(Tag::BOOLEAN)] = compiler::pSym(new TypeScalar(SCALAR_TYPE::BOOLEAN));
};

void compiler::Parser::setVarTable ( void ) {
  pSymVar pi(new SymVar(Lexeme()));
  pi->name = "PI";
  pi->token = Token::IDENTIFIER;
  pi->tag = Tag::UNDEFINED;
  pi->glob = GLOB::CONST;
  pi->value = "3.1415926535897932385";
  pi->type = typeTable[tagBook.at(Tag::REAL)];
  varTable[pi->name] = pi;
};

//TODO!
compiler::pSymVar compiler::Parser::evalConstExpr ( pExpr& root, SymTable& vTable, TypeTable& tTable ) {

};

//TODO!
std::tuple<compiler::pSymTable, std::string> compiler::Parser::parseParams( void ) {

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
void compiler::Parser::checkFunc ( const Lexeme& lexeme, IdentifierType type, const std::string& args ) {

};
