#include "../inc/Parser.hpp"

compiler::pSymType compiler::Parser::parseType ( void ) {
  pSymType type = nullptr;
  pSymTable table;
  Lexeme lexeme = scanner.lex();
  switch (lexeme.tag) {
    case (Tag::ARRAY) :
      type = parseArray();
    break;
    // case (Tag::SET) :
    case (Tag::POINTER) :
      type = parsePointer();
    break;
    default :
      table = symStack.find(lexeme.name);
      pSym elem = table->find(lexeme.name);
      if
      type = evalAlias(iter->second);
  }

  scanner.next();
  lexeme = scanner.lex();

  return type;
}

compiler::pSymType compiler::Parser::parseArray ( SymTable& vTable, TypeTable& tTable ) {
  std::shared_ptr<TypeArray> vArray = nullptr;
  pSymType type = pSymType(new TypeArray("ARRAY"));
  Lexeme lexeme = scanner.lex();
  while (lexeme.tag == Tag::ARRAY) {
    scanner.next();
    lexeme = scanner.lex();
    //Checking for static array
    if (lexeme.tag == Tag::LEFT_BRACKET) {
      do {
        if (vArray == nullptr)
          vArray = std::dynamic_pointer_cast<TypeArray>(type);
        else {
          lexeme.name = "ARRAY";
          lexeme.tag = Tag::ARRAY;
          lexeme.token = Token::IDENTIFIER;
          vArray->elemType = pSymType(new TypeArray(lexeme.name));
          vArray = std::dynamic_pointer_cast<TypeArray>(vArray->elemType);
        }
        Lexeme dbg_lex = lexeme;

        scanner.next();
        lexeme = scanner.lex();

        //Parse LOW
        pExpr local_root = parseExpr(Priority::LOWEST);
        pExpr tmp = evalConstExpr(local_root, vTable, tTable);
        if (tmp->exprType != ExprEnum::Integer)
          throw ExprException("Unexpected token '" + tmp->name + "' in pos ("
          + std::to_string(tmp->row) + ", " + std::to_string(tmp->column) +
          "); expected 'integer literal/const integer expected';");
        vArray->low = std::strtoll(tmp->name.c_str(), nullptr, 10);
        //
        if (scanner.lex().tag != Tag::DOUBLE_DOT)
          err("'..'");
        scanner.next();
        lexeme = scanner.lex();
        //Parse HIGH
        local_root = parseExpr(Priority::LOWEST);
        tmp = evalConstExpr(local_root, vTable, tTable);
        if (tmp->exprType != ExprEnum::Integer)
          throw ExprException("Unexpected token '" + tmp->name + "' in pos ("
          + std::to_string(tmp->row) + ", " + std::to_string(tmp->column) +
          "); expected 'integer literal/const integer expected';");
        vArray->high = std::strtoll(tmp->name.c_str(), nullptr, 10);
        //
        if (vArray->high < vArray->low)
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
  vArray->elemType = evalAlias(iter->second);

  return type;
}

compiler::pSymType compiler::Parser::parseRecord ( const std::string& name, SymTable& vTable, TypeTable& tTable ) {
  std::shared_ptr<TypeRecord> vRecord(new TypeRecord(name));
  std::vector<pSymVar> var;
  vRecord->field = pSymTable(new SymTable);

  scanner.next();
  Lexeme lexeme = scanner.lex();

  if (lexeme.tag != Tag::IDENTIFIER)
    err("`identifier`");
  for (;lexeme.tag == Tag::IDENTIFIER; scanner.next(), lexeme = scanner.lex()) {
    //Check in field
    vRecord->checkIdent(lexeme);
    var.push_back(pSymVar(new SymVar(lexeme.name)));

    scanner.next();
    lexeme = scanner.lex();

    for (;lexeme.tag == Tag::COMMA; scanner.next(), lexeme = scanner.lex()) {
      scanner.next();
      lexeme = scanner.lex();

      vRecord->checkIdent(lexeme);
      var.push_back(pSymVar(new SymVar(lexeme.name)));
    }

    if (lexeme.tag != Tag::COLON)
      err("':'");

    scanner.next();
    pSymType type = evalAlias(parseType(vTable, tTable));
    lexeme = scanner.lex();
    if (lexeme.tag != Tag::EQUALS && lexeme.tag != Tag::SEMICOLON)
      err("';'");
    if (scanner.lex().tag == Tag::EQUALS) {
      if (var.size() != 1)
        err("';'");
      if (type->symType == SymEnum::Array) {
        if (std::dynamic_pointer_cast<TypeArray>(type)->elemType->symType == SymEnum::Array ||
            std::dynamic_pointer_cast<TypeArray>(type)->low >
            std::dynamic_pointer_cast<TypeArray>(type)->high)
          err("';'");
        //Now only 1D massives
        scanner.next();
        lexeme = scanner.lex();
        if (lexeme.tag != Tag::LEFT_PARENTHESIS)
          err("'('");
        scanner.next();
        lexeme = scanner.lex();
        std::shared_ptr<TypeArray> arr = std::dynamic_pointer_cast<TypeArray>(type);
        //allocate only for 1D massive for initizalization!
        if (!std::dynamic_pointer_cast<TypeArray>(arr)->values.size())
          arr->values.resize(arr->high - arr->low + 1);
        for (unsigned long long i = arr->low; i <= arr->high; ++i, scanner.next(), lexeme = scanner.lex()) {
          pExpr local_root = parseExpr(Priority::LOWEST);
          pExpr tmp = evalConstExpr(local_root, vTable, tTable);
          //TODO: check type of tmp end evaluate to array
          arr->values[i-arr->low] = checkType(arr->values[i-arr->low], arr->elemType, tmp, vTable, tTable);
          lexeme = scanner.lex();
          if (i < arr->high && lexeme.tag != Tag::COMMA)
            err("','");
          if (i == arr->high && lexeme.tag != Tag::RIGHT_PARENTHESIS)
            err("')'");
        }
      } else {
        scanner.next();
        lexeme = scanner.lex();
        pExpr local_root = parseExpr(Priority::LOWEST);
        pExpr tmp = evalConstExpr(local_root, vTable, tTable);
        var.front() = checkType(var.front(), var.front()->type, tmp, vTable, tTable);
      }
    }

    for (pSymVar& elem : var) {
      elem->type = evalAlias(type);
      elem->glob = compiler::GLOB::VAR;
      vRecord->field->emplace(elem->name, elem);
    }
    var.clear();
    if (scanner.lex().tag != Tag::SEMICOLON)
      err("';'");
  }
  if (scanner.lex().tag != Tag::END)
    err("END");
  scanner.next();
  return vRecord;
}

compiler::pSymType compiler::Parser::parseEnum ( void ) { return pSymType(); }

compiler::pSymType compiler::Parser::parsePointer ( SymTable& vTable, TypeTable& tTable ) {
    std::shared_ptr<TypePointer> type(new TypePointer(scanner.lex().name));
    scanner.next();
    Lexeme lexeme = scanner.lex();
    auto iter = tTable.find(lexeme.name);
    if (iter == tTable.end()) {
      iter = typeTable.find(lexeme.name);
      if (iter == typeTable.end())
        errUndefType();
    }
    type->elemType = iter->second;

    return type;
}

void compiler::Parser::parseProgramName ( const compiler::Lexeme& program ) {
  scanner.next();
  compiler::Lexeme lexeme = scanner.lex();
  if (lexeme.tag != Tag::IDENTIFIER)
    err("`program name`");

  std::shared_ptr<compiler::SymVar> var = std::shared_ptr<compiler::SymVar>(new SymVar(program.name));
  var->glob = compiler::GLOB::CONST;
  var->type = std::shared_ptr<compiler::SymType>(new SymType(program.name));
  varTable[lexeme.name] = var;

  scanner.next();
  lexeme = scanner.lex();
  if (lexeme.tag != Tag::SEMICOLON)
    err("';'");
  scanner.next();
}

void compiler::Parser::parseConst ( SymTable& vTable, TypeTable& tTable ) {
  scanner.next();
  Lexeme lexeme = scanner.lex();
  pExpr local_root, tmp;
  if (lexeme.tag != Tag::IDENTIFIER)
    err("`identifier`");
  for (; lexeme.tag == Tag::IDENTIFIER; scanner.next(), lexeme = scanner.lex()) {

    checkIdent(lexeme, vTable, tTable);
    checkFunc(lexeme, IdentifierType::VARIABLE);

    pSymVar var = pSymVar(new SymVar(lexeme.name));

    scanner.next();
    lexeme = scanner.lex();

    var->type = nullptr;

    switch (lexeme.tag) {
      case(Tag::COLON) :
        scanner.next();
        var->type = parseType(vTable, tTable);
        //NOW scanner.lex().tag == ';'/'='
        lexeme = scanner.lex();
        if (lexeme.tag != Tag::EQUALS && lexeme.tag != Tag::SEMICOLON)
          err("';'");
        var->glob = GLOB::VAR;
      break;
      case (Tag::EQUALS) :
        var->glob = compiler::GLOB::CONST;
        scanner.next();
        lexeme = scanner.lex();
        //NOW PARSE CONST EXPR
        local_root = parseExpr(Priority::LOWEST);
        tmp = evalConstExpr(local_root, vTable, tTable);
        var = checkType(var, var->type, tmp, vTable, tTable);
      break;
      default :
        err("'='");
      break;
    }

    if (scanner.lex().tag != Tag::SEMICOLON)
      err("';'");

    vTable[var->name] = var;//TO UPPERCASE!
  }
}

void compiler::Parser::parseVar ( SymTable& vTable, TypeTable& tTable ) {
  std::vector<pSymVar> var;

  scanner.next();
  Lexeme lexeme = scanner.lex();
  if (lexeme.tag != Tag::IDENTIFIER)
    err("`identifier`");

  for (;lexeme.tag == Tag::IDENTIFIER; scanner.next(), lexeme = scanner.lex()) {
    checkIdent(lexeme, vTable, tTable);
    checkFunc(lexeme, IdentifierType::VARIABLE);

    var.push_back(pSymVar(new SymVar(lexeme.name)));

    scanner.next();
    lexeme = scanner.lex();

    for (;lexeme.tag == Tag::COMMA; scanner.next(), lexeme = scanner.lex()) {
      scanner.next();
      lexeme = scanner.lex();

      checkIdent(lexeme, vTable, tTable);
      checkFunc(lexeme, IdentifierType::VARIABLE);
      var.push_back(pSymVar(new SymVar(lexeme.name)));
    }

    if (lexeme.tag != Tag::COLON)
      err("':'");

    scanner.next();
    pSymType type = parseType(vTable, tTable);
    lexeme = scanner.lex();
    if (lexeme.tag != Tag::EQUALS && lexeme.tag != Tag::SEMICOLON)
      err("';'");
    if (scanner.lex().tag == Tag::EQUALS) {
      if (var.size() != 1)
        err("';'");
      if (type->symType == SymEnum::Array) {
        if (std::dynamic_pointer_cast<TypeArray>(type)->elemType->symType == SymEnum::Array ||
            std::dynamic_pointer_cast<TypeArray>(type)->low >
            std::dynamic_pointer_cast<TypeArray>(type)->high)
          err("';'");
        //Now only 1D massives
        scanner.next();
        lexeme = scanner.lex();
        if (lexeme.tag != Tag::LEFT_PARENTHESIS)
          err("'('");
        scanner.next();
        lexeme = scanner.lex();
        std::shared_ptr<TypeArray> arr = std::dynamic_pointer_cast<TypeArray>(type);
        //allocate only for 1D massive for initizalization!
        if (!std::dynamic_pointer_cast<TypeArray>(arr)->values.size())
          arr->values.resize(arr->high - arr->low + 1);
        for (unsigned long long i = arr->low; i <= arr->high; ++i, scanner.next(), lexeme = scanner.lex()) {
          pExpr local_root = parseExpr(Priority::LOWEST);
          pExpr tmp = evalConstExpr(local_root, vTable, tTable);
          //TODO: check type of tmp end evaluate to array
          arr->values[i-arr->low] = checkType(arr->values[i-arr->low], arr->elemType, tmp, vTable, tTable);
          lexeme = scanner.lex();
          if (i < arr->high && lexeme.tag != Tag::COMMA)
            err("','");
          if (i == arr->high && lexeme.tag != Tag::RIGHT_PARENTHESIS)
            err("')'");
        }
      } else {
        scanner.next();
        lexeme = scanner.lex();
        pExpr local_root = parseExpr(Priority::LOWEST);
        pExpr tmp = evalConstExpr(local_root, vTable, tTable);
        var.front() = checkType(var.front(), type, tmp, vTable, tTable);
      }
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
}

void compiler::Parser::parseFunction ( bool expectRetVal ) {
  scanner.next();
  Lexeme lexeme = scanner.lex();
  Lexeme funcName = lexeme;
  //Pascal feature: function result variable has name like a name of function
  checkIdent(funcName, varTable, typeTable);

  std::shared_ptr<SymFunc> func(new SymFunc(funcName.name));

  scanner.next();
  lexeme = scanner.lex();
  std::string args;
  func->params = nullptr;
  if (lexeme.tag == Tag::LEFT_PARENTHESIS) {
    scanner.next();
    std::tie(func->params, args) = parseParams();
    //NOW scanner.lex().name == ")"
    scanner.next();
    lexeme = scanner.lex();
  }

  checkFunc(funcName, IdentifierType::FUNCTION, args);

  //
  if (expectRetVal) {
    if (lexeme.tag != Tag::COLON)
      err("':'");

    scanner.next();
    func->retType = parseType(varTable, typeTable);
    lexeme = scanner.lex();
    if (lexeme.tag != Tag::SEMICOLON)
      err("';'");
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
      default: err("`begin statement`");
    }
  }

  //NOW scanner.lex().name == "BEGIN"

  func->body = parseBlock(func->varTable);

  //NOW scanner.lex().name == ";"

  if (scanner.lex().tag != Tag::SEMICOLON)
    err("';'");

  funcTable[func->name][args] = func;

  scanner.next();
  lexeme = scanner.lex();
}

void compiler::Parser::parseAlias ( SymTable& vTable, TypeTable& tTable ) {
  scanner.next();
  compiler::Lexeme lexeme = scanner.lex();
  if (lexeme.tag != Tag::IDENTIFIER)
    err("`identifier`");
  while (lexeme.tag == Tag::IDENTIFIER) {

    checkIdent(lexeme, vTable, tTable);
    checkFunc(lexeme, IdentifierType::TYPE);

    Lexeme ident = lexeme;
    std::shared_ptr<TypeAlias> alias(new TypeAlias(ident.name));

    scanner.next();
    lexeme = scanner.lex();

    if (lexeme.tag != Tag::EQUALS)
      err("'='");

    scanner.next();
    lexeme = scanner.lex();

    switch (lexeme.tag) {
      case (Tag::RECORD):
        alias->type = parseRecord(ident.name, vTable, tTable);
      break;
      // case (Tag::LEFT_PARENTHESIS): alias->type = parseEnum(); break;
      default:
        alias->type = parseType(vTable, tTable);
        lexeme = scanner.lex();
        if (lexeme.tag != Tag::SEMICOLON)
          err("';'");
      break;
    }
    lexeme = scanner.lex();

    if (lexeme.tag != Tag::SEMICOLON)
      err("';'");

    tTable[alias->name] = alias;

    scanner.next();
    lexeme = scanner.lex();
  }
}
