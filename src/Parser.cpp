#include "../inc/Parser.hpp"

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
  checkParenthesis (scanner.lex());
  root = parseExpr(compiler::Priority::LOWEST);
};

compiler::pExpr compiler::Parser::parseExpr ( const compiler::Priority& priority ) {
  if (priority == Priority::HIGHEST)
    return parseFactor();
  compiler::pExpr left = parseExpr(upPriority(priority));
  compiler::Lexeme lexeme = scanner.lex();
  while (checkPriority(priority, lexeme.tag)) {
    scanner.nextLex();
    checkParenthesis (scanner.lex());
    compiler::pExpr right = parseExpr(upPriority(priority));
    left = compiler::pExpr(new compiler::ExprBinOp(lexeme, left, right));
    lexeme = scanner.lex();
  }
  return left;
};

compiler::pExpr compiler::Parser::parseFactor ( void ) {
  compiler::pExpr tmpExpr;
  compiler::Lexeme lexeme = scanner.lex();

  if (lexeme.token == compiler::Token::IDENTIFICATOR &&
    lexeme.tag == compiler::Tag::UNDEFINED)
    return parseTerm();

  scanner.nextLex();
  checkParenthesis (scanner.lex());

  if (scanner.lex().tag == compiler::Tag::LEFT_BRACE && (lexeme.token != compiler::Token::IDENTIFICATOR))
    throw ExprException("Unexpected expression: \'[\' in pos (" + std::to_string(scanner.lex().row) + ", " + std::to_string(scanner.lex().column) + ");");

  if (isUnary(lexeme.tag)) return compiler::pExpr(new ExprUnOp(lexeme, parseExpr(Priority::HIGHEST)));
  if (lexeme.tag == compiler::Tag::INTEGER) return compiler::pExpr(new ExprInteger(lexeme));
  if (lexeme.tag == compiler::Tag::FLOAT) return compiler::pExpr(new ExprReal(lexeme));

  if (lexeme.tag == compiler::Tag::LEFT_BRACKET) {
    tmpExpr = parseExpr(compiler::Priority::LOWEST);
    if (scanner.lex().tag != compiler::Tag::RIGHT_BRACKET) {
      lexeme = scanner.lex();
      if (lexeme.token == compiler::Token::END_OF_FILE)
        throw ExprException("Unexpected end of file: in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + ");");
      throw ExprException("Unexpected expression: \'" + lexeme.name + "\' in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + "); expected \')\';");
    }
    scanner.nextLex();
    checkParenthesis (scanner.lex());
    return tmpExpr;
  }
  if (lexeme.token == compiler::Token::END_OF_FILE)
      throw ExprException("Unexpected end of file: in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + ");");

  throw ExprException("Illegal expression: \'" + lexeme.name + "\' in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + ");");
};

compiler::pExpr compiler::Parser::parseTerm ( void ) {
  compiler::Lexeme lexeme = scanner.lex();
  compiler::pExpr left = compiler::pExpr(new ExprIdentifier(lexeme));
  while (true) {
    compiler::Lexeme tmpLex;
    scanner.nextLex();
    checkParenthesis (scanner.lex());
    lexeme = scanner.lex();
    if (lexeme.tag == compiler::Tag::DOT) {
      tmpLex = lexeme;
      scanner.nextLex();
      checkParenthesis (scanner.lex());
      lexeme = scanner.lex();
      if (lexeme.token == compiler::Token::END_OF_FILE)
          throw ExprException("Unexpected end of file: in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + ");");
      if (lexeme.token != compiler::Token::IDENTIFICATOR || lexeme.tag != compiler::Tag::UNDEFINED)
        throw ExprException("Illegal expression: \'" + lexeme.name + "\' in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + "); expected \'IDENTIFICATOR\';");
      compiler::pExpr right = compiler::pExpr(new ExprIdentifier(lexeme));
      left = compiler::pExpr(new ExprRecordAccess(tmpLex, left, right));
    } else if (lexeme.tag == compiler::Tag::LEFT_BRACE) {
      tmpLex = lexeme;
      std::vector< compiler::pExpr > args = parseArrayIndex();
      left = compiler::pExpr(new ExprArrayIndex(tmpLex, left, args));
      lexeme = scanner.lex();
      if (lexeme.token == compiler::Token::END_OF_FILE)
          throw ExprException("Unexpected end of file: in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + ");");
      if (lexeme.tag != compiler::Tag::RIGHT_BRACE)
        throw ExprException("Illegal expresssion: \'" + lexeme.name + "\' in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + "); expected \']\';");
    } else break;
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
  checkParenthesis (scanner.lex());
  expr.push_back(parseExpr(compiler::Priority::LOWEST));
  lexeme = scanner.lex();
  while (lexeme.tag == compiler::Tag::COMMA) {
    scanner.nextLex();
    checkParenthesis (scanner.lex());
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

void compiler::Parser::checkParenthesis ( const compiler::Lexeme& lex ) {
  if (lex.tag == compiler::Tag::LEFT_BRACE) ++diffBrace;
  if (lex.tag == compiler::Tag::RIGHT_BRACE) --diffBrace;
  if (lex.tag == compiler::Tag::LEFT_BRACKET) ++diffBracket;
  if (lex.tag == compiler::Tag::RIGHT_BRACKET) --diffBracket;

  if (diffBracket < 0 || diffBrace < 0)
    throw ExprException("Unexpected parenth': \'" + lex.name + "\' in pos (" + std::to_string(lex.row) + ", " + std::to_string(lex.column) + ");");
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
