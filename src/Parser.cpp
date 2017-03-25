#include "Parser.hpp"

compiler::Parser::Parser ( void ) {
  setPriorities();
};

compiler::Parser::Parser ( const std::string& filename ) : scanner(filename) {
  setPriorities();
};

void compiler::Parser::set ( const std::string& filename ) {
  scanner.open(filename);
};

void compiler::Parser::parse ( void ) {
  scanner.nextLex();
  root = parseExpr(compiler::Priority::LOWEST);
  if (scanner.lex().token != compiler::Token::END_OF_FILE) err(scanner.lex());
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
  std::ostringstream out;
  if (root != nullptr)
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
  throw ExprException("Unexpected token '" + lexeme.name + "' in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + "); expected '" + expected_token + "';");
};

void compiler::Parser::err ( const compiler::Lexeme& lexeme ) {
  throw ExprException("Unexpected token '" + lexeme.name + "' in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + ");");
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
