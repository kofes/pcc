#include "../inc/Parser.hpp"

//DEBUG
#include <iostream>
//

compiler::Parser::Parser ( void ) {
  setPriorities();
  setTypeTable();
  setVarTable();
}

compiler::Parser::Parser ( const std::string& filename ) : scanner(filename) {
  setPriorities();
  setTypeTable();
  setVarTable();
}

void compiler::Parser::set ( const std::string& filename ) {
  scanner.open(filename);
}

void compiler::Parser::parseExpr ( void ) {
  scanner.next();
  root = parseExpr(compiler::Priority::LOWEST);
  if (scanner.lex().token != Token::END_OF_FILE)
    err(scanner.lex());
}

void compiler::Parser::parseTable ( void ) {
  programTokenChecked = false;
  scanner.next();
  while (true) {
    switch (scanner.lex().tag) {
      case (Tag::PROGRAM) : programTokenChecked ? err(scanner.lex()) : parseProgramName(scanner.lex()); break;
      case (Tag::CONST) : parseConst(varTable, typeTable); break;
      case (Tag::TYPE) : parseAlias(varTable, typeTable); break;
      case (Tag::VAR) : parseVar(varTable, typeTable); break;
      case (Tag::FUNCTION) : parseFunction(); break;
      case (Tag::PROCEDURE) : parseFunction(false); break;
      default :
        if (scanner.lex().token == Token::END_OF_FILE) return;
        err("`table statement`");
    }
    programTokenChecked = true;
  }
}

void compiler::Parser::parse ( void ) {
  programTokenChecked = false;
  scanner.next();
  while (true) {
    switch (scanner.lex().tag) {
      case (Tag::PROGRAM) : programTokenChecked ? err(scanner.lex()) : parseProgramName(scanner.lex()); break;
      case (Tag::CONST) : parseConst(varTable, typeTable); break;
      case (Tag::TYPE) : parseAlias(varTable, typeTable); break;
      case (Tag::VAR) : parseVar(varTable, typeTable); break;
      case (Tag::FUNCTION) : parseFunction(); break;
      case (Tag::PROCEDURE) : parseFunction(false); break;
      case (Tag::BEGIN) : root = parseBlock(varTable); return;
      default : err("`begin statement`");
    }
    programTokenChecked = true;
  }
}

std::string compiler::Parser::print ( void ) {
  std::ostringstream out;

  out << "<function table>\n"
      << printFuncTable()
      << "</function table>\n"
      << "<type table>\n"
      << printTypeTable()
      << "</type table>\n"
      << "<var table>\n"
      << printVarTable()
      << "</var table>\n"
      << printExprs();

  return out.str();
}

std::string compiler::Parser::printExprs ( void ) {
  // if (scanner.lex().token != Token::END_OF_FILE) err(scanner.lex());//Program's end is 'end.'

  std::ostringstream out;
  if (root != nullptr)
    out << root->print(0);

  return out.str();
}

std::string compiler::Parser::printVarTable ( void ) {
  std::ostringstream out;
  for (const std::pair< std::string, pSymVar >& elem : varTable)
    out << std::string(compiler::DEEP_STEP, compiler::DEEP_CHAR)
        << "<var>" << elem.second->print(0) << "</var>\n";

  return out.str();
}

std::string compiler::Parser::printFuncTable ( void ) {
  std::ostringstream out;
  for (const std::pair< std::string, std::map< std::string, pSym> >& functions : funcTable)
    for (const std::pair< std::string, pSym>& elem : functions.second)
      out << elem.second->print(1) << '\n';

  return out.str();
}

std::string compiler::Parser::printTypeTable ( void ) {
  std::ostringstream out;
  for (const std::pair< std::string, pSymType >& type : typeTable) {
    if (type.second->symType == SymEnum::Alias &&
      std::dynamic_pointer_cast<TypeAlias>(type.second)->type->symType == SymEnum::Record)
        out << std::string(compiler::DEEP_STEP, compiler::DEEP_CHAR)
            << "<type>\n" << type.second->print(2) << '\n'
            << std::string(compiler::DEEP_STEP, compiler::DEEP_CHAR)
            << "</type>\n";
    else
      out << std::string(compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<type>" << type.second->print(0) << "</type>\n";
  }

  return out.str();
}

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
}

compiler::Priority compiler::Parser::upPriority ( const compiler::Priority& pr ) {
  if (pr == compiler::Priority::HIGHEST)
    return compiler::Priority::HIGHEST;
  return (compiler::Priority)((unsigned short)pr - 1);
}

bool compiler::Parser::checkPriority ( const compiler::Priority& pr, const Tag& tag ) {
  auto tmp = binaryPriority.find(tag);
  return tmp != binaryPriority.end() && tmp->second == pr;
}

bool compiler::Parser::isUnary ( const Tag& tag ) {
  return unaryPriority.find(tag) != unaryPriority.end();
}

void compiler::Parser::err ( const std::string& expected_token ) {
  compiler::Lexeme lexeme = scanner.lex();
  if (expected_token == "")
    throw ExprException("Unexpected end of file in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + ");");
  if (lexeme.token == Token::END_OF_FILE)
    throw ExprException("Unexpected end of file in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + "); expected '" + expected_token + "';");
  throw ExprException("Unexpected token '" + lexeme.name + "' in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + "); expected '" + expected_token + "';");
}

void compiler::Parser::err ( const compiler::Lexeme& lexeme ) {
  if (lexeme.token == Token::END_OF_FILE)
    throw ExprException("Unexpected end of file in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + ");");
  throw ExprException("Unexpected token '" + lexeme.name + "' in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + ");");
}

void compiler::Parser::errUndefType ( void ) {
  compiler::Lexeme lexeme = scanner.lex();
  if (lexeme.token == Token::END_OF_FILE)
    throw ExprException("Unexpected end of file in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + "); expected `type name`;");
  throw ExprException("Undefined type '" + lexeme.name + "' in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + ");");
}

void compiler::Parser::errDuplicated ( void ) {
  compiler::Lexeme lexeme = scanner.lex();
  throw ExprException("Duplicate identifier \"" + lexeme.name + "\" in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + ");");
}

void compiler::Parser::errAssignment ( const Lexeme& lexeme ) {
  throw ExprException("Bad assignment at line: " + std::to_string(lexeme.row) + ";");
}

void compiler::Parser::errHighLow ( const compiler::Lexeme& lexeme ) {
  throw ExprException("Array 'high' < 'low' from pos ("+std::to_string(lexeme.row)+", "+std::to_string(lexeme.column)+");");
}

void compiler::Parser::errDecl ( const compiler::Lexeme& lexeme ) {
  throw ExprException("Identifier "+lexeme.name+" at pos ("+std::to_string(lexeme.row)+", "+std::to_string(lexeme.column)+") wasn't declared;");
}

void compiler::Parser::errConst ( const Lexeme& lexeme ) {
  throw ExprException("Const "+lexeme.name+" at pos ("+std::to_string(lexeme.row)+", "+std::to_string(lexeme.column)+") assignment;");
}

void compiler::Parser::checkIdent ( const Lexeme& lexeme, SymTable& vTable, TypeTable& tTable ) {
  if (lexeme.tag != Tag::IDENTIFIER)
    err("`identifier`");
  if (varTable.find(lexeme.name) != varTable.end()  ||
        vTable.find(lexeme.name) != vTable.end()    ||
     typeTable.find(lexeme.name) != typeTable.end() ||
        tTable.find(lexeme.name) != tTable.end()   ||
     lexeme.name == "write" || lexeme.name == "writeln" ||
     lexeme.name == "read" || lexeme.name == "readln")
    errDuplicated();
}

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
}

void compiler::Parser::setTypeTable ( void ) {
  // typeTable[tagBook.at(Tag::POINTER)] = compiler::pSymType(new TypeScalar(SCALAR_TYPE::POINTER));
  typeTable[tagBook.at(Tag::INTEGER)] = compiler::pSymType(new TypeScalar(SCALAR_TYPE::INTEGER));
  typeTable[tagBook.at(Tag::REAL)] = compiler::pSymType(new TypeScalar(SCALAR_TYPE::REAL));
  typeTable[tagBook.at(Tag::CHAR)] = compiler::pSymType(new TypeScalar(SCALAR_TYPE::CHAR));
  typeTable[tagBook.at(Tag::BOOLEAN)] = compiler::pSymType(new TypeScalar(SCALAR_TYPE::BOOLEAN));
}

void compiler::Parser::setVarTable ( void ) {
  pSymVar pi(new SymVar("PI"));
  pi->glob = GLOB::CONST;
  pi->value = "3.1415926535897932385";
  pi->type = typeTable[tagBook.at(Tag::REAL)];
  varTable[pi->name] = pi;
}

std::tuple<compiler::pSymTable, std::string> compiler::Parser::parseParams( void ) {
  Lexeme lexeme = scanner.lex();
  pSymTable resTable(new SymTable);
  std::ostringstream sstream;
  size_t countArgs = 0;
  pSymType type = nullptr;
  while ((lexeme.tag == Tag::IDENTIFIER) ||
         lexeme.tag == Tag::CONST ||
         lexeme.tag == Tag::VAR ||
         lexeme.tag == Tag::OUT) {
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
    if (lexeme.tag != Tag::IDENTIFIER)
      err("`parameter`");
    std::vector<pSymVar> vars;
    vars.push_back(pSymVar(new SymVar(lexeme.name)));
    scanner.next();
    lexeme = scanner.lex();
    for (;lexeme.tag == Tag::COMMA; scanner.next(), lexeme = scanner.lex()) {
      scanner.next();
      lexeme = scanner.lex();
      vars.push_back(pSymVar(new SymVar(lexeme.name)));
    }
    pSymType type_tmp = nullptr;
    switch (glob) {
        case(GLOB::CONST_PARAM):
            if (lexeme.tag == Tag::COLON) {
              scanner.next();
              type_tmp = parseType(varTable, typeTable);
              lexeme = scanner.lex();
              //NOW scanner.lex().tag == ';'/'='/')'
              if (lexeme.tag != Tag::EQUALS && lexeme.tag != Tag::SEMICOLON && lexeme.tag != Tag::RIGHT_PARENTHESIS)
                err("')'");
            } else type_tmp = nullptr;
            if (lexeme.tag == Tag::EQUALS) {
              if (vars.size() != 1)  err("')'");
              scanner.next();
              pExpr local_root = parseExpr(Priority::LOWEST);
              pExpr tmp = evalConstExpr(local_root, varTable, typeTable);
              vars.front() = checkType(vars.front(), type_tmp, tmp, varTable, typeTable);
            }
        break;
        case (GLOB::VAL_PARAM):
            if (lexeme.tag != Tag::COLON) err("':'");
            scanner.next();
            type_tmp = parseType(varTable, typeTable);
            lexeme = scanner.lex();
            //NOW scanner.lex().tag == ';'/'=/')''
            if (lexeme.tag != Tag::EQUALS && lexeme.tag != Tag::SEMICOLON && lexeme.tag != Tag::RIGHT_PARENTHESIS)
              err("')'");
            if (lexeme.tag == Tag::EQUALS) {
              if (vars.size() > 1)  err("')'");
              scanner.next();
              pExpr local_root = parseExpr(Priority::LOWEST);
              pExpr tmp = evalConstExpr(local_root, varTable, typeTable);
              vars.front() = checkType(vars.front(), type_tmp, tmp, varTable, typeTable);
            }
        break;
        case (GLOB::OUT_PARAM):
        case (GLOB::VAR_PARAM):
            if (lexeme.tag == Tag::COLON) {
              scanner.next();
              type_tmp = parseType(varTable, typeTable);
              lexeme = scanner.lex();
              if (lexeme.tag != Tag::SEMICOLON && lexeme.tag != Tag::RIGHT_PARENTHESIS)
                err("')'");
            }
        break;
        default: err("'parameter's modificator'");
    }
    //resTable < vars, resStr < str
    //type equals over!
    if (type_tmp != nullptr)
      type_tmp = evalAlias(type_tmp);
    if (!countArgs || type == type_tmp) {
      countArgs += vars.size();
    } else {
      sstream << countArgs;
      if (type != nullptr) sstream << ':' << type->name;
      sstream << ';';
      countArgs = vars.size();
    }
    type = type_tmp;
    for (pSymVar& elem : vars) {
      elem->type = type_tmp;
      elem->glob = glob;
      (*resTable)[elem->name] = elem;
    }
    vars.clear();
    lexeme = scanner.lex();
    if (lexeme.tag == Tag::SEMICOLON) {
      scanner.next();
      lexeme = scanner.lex();
      if (lexeme.tag == Tag::RIGHT_PARENTHESIS)
          err("`parameter`");
    }
  }
  if (lexeme.tag != Tag::RIGHT_PARENTHESIS)
    err(")");
  if (countArgs) {
    sstream << countArgs;
    if (type != nullptr) sstream << ':' << type->print(0);
    sstream << ';';
    countArgs = 0;
  }
  return std::make_tuple(resTable, sstream.str());
}

compiler::pSymVar compiler::Parser::checkType ( pSymVar res, pSymType type, pExpr src, SymTable& vTable, TypeTable& tTable ) {
  if (res == nullptr)
    res = pSymVar(new SymVar);
  if (type == nullptr) {
    switch (src->exprType) {
      case (ExprEnum::Integer):
        res->type = pSymType(new TypeScalar(SCALAR_TYPE::INTEGER));
      break;
      case (ExprEnum::Real):
        res->type = pSymType(new TypeScalar(SCALAR_TYPE::REAL));
      break;
      case (ExprEnum::Char):
        res->type = pSymType(new TypeScalar(SCALAR_TYPE::CHAR));
      break;
      case (ExprEnum::String):
      break;
      case (ExprEnum::Identifier):
        if (src->tag == Tag::B_FALSE || src->tag == Tag::B_TRUE)
          res->type = pSymType(new TypeScalar(SCALAR_TYPE::BOOLEAN));
        else if (src->tag == Tag::NIL) {
          res->type = pSymType(new TypePointer("NIL"));
        }
        else err(*src);
      break;
      default: err(*src);
    }
    res->value = src->name;
    return res;
  }
  if (type->symType == SymEnum::Pointer) {
    if (src->tag != Tag::NIL) err(*src);
    res->value = "NIL";
    return res;
  }
  // Now only alias/scalar must be
  while (type->symType == SymEnum::Alias && std::dynamic_pointer_cast<TypeAlias>(type)->type != nullptr)
    type = std::dynamic_pointer_cast<TypeAlias>(type)->type;

  if (type->symType != SymEnum::Scalar)
    err(*src);
  switch (std::dynamic_pointer_cast<TypeScalar>(type)->type) {
    case (SCALAR_TYPE::BOOLEAN):
      if (src->tag != Tag::B_FALSE && src->tag != Tag::B_TRUE)
        err(*src);
      res->type = pSymType(new TypeScalar(SCALAR_TYPE::BOOLEAN));
    break;
    case (SCALAR_TYPE::CHAR):
      if (src->exprType != ExprEnum::Char)
        err(*src);
      res->type = pSymType(new TypeScalar(SCALAR_TYPE::CHAR));
    break;
    case (SCALAR_TYPE::INTEGER):
      if (src->exprType != ExprEnum::Integer)
        err(*src);
      res->type = pSymType(new TypeScalar(SCALAR_TYPE::INTEGER));
    break;
    case (SCALAR_TYPE::REAL):
      if (src->exprType != ExprEnum::Integer && src->exprType != ExprEnum::Real)
        err(*src);
      res->type = pSymType(new TypeScalar(SCALAR_TYPE::REAL));
    break;
  }
  res->value = src->name;

  return res;
}

void compiler::Parser::checkFunc ( const Lexeme& lexeme, IdentifierType type, const std::string& args ) {
    auto iter = funcTable.find(lexeme.name);
    if (iter == funcTable.end())
        return;
    if (iter->second.find(args) == iter->second.end())
        return;
    throw ExprException("Duplication of function \"" + lexeme.name + "\" in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + ");");
}

void compiler::Parser::generate ( void ) {
  // for (auto elem : varTable)
  //   elem.second->generate(asmGenerator);
  // for (auto arr : funcTable)
  //   for (auto func : arr.second)
  //     func.second->generate(asmGenerator);
  // asmGenerator.addLabel("main");
  // root->generate(asmGenerator);
}

std::string compiler::Parser::printAsm ( void ) {
  return "";
}
