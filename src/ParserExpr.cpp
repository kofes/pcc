#include "../inc/Parser.hpp"

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
}

compiler::pExpr compiler::Parser::parseFactor ( void ) {
  compiler::pExpr tmpExpr;
  compiler::Lexeme lexeme = scanner.lex();
  scanner.next();

  if (isUnary(lexeme.tag))
    return compiler::pExpr(new ExprUnOp(lexeme, parseExpr(Priority::HIGHEST)));

  switch (lexeme.tag) {
    case (Tag::IDENTIFIER) : return parseIdentifier(lexeme);
    case (Tag::INTEGER) : return pExpr(new ExprInteger(lexeme));
    case (Tag::FLOAT) : return pExpr(new ExprReal(lexeme));
    case (Tag::CHARACTER) : return pExpr(new ExprChar(lexeme));
    case (Tag::STRING) : return pExpr(new ExprString(lexeme));
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
    case (Tag::NIL):
      return pExpr(new ExprIdentifier(lexeme));
    default: break;
  }

  if (lexeme.token == Token::END_OF_FILE) err();
  err(lexeme);
  return compiler::pExpr(new Expr(lexeme));
}

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
        if (lexeme.tag != Tag::IDENTIFIER) err("`identifier`");
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
}
