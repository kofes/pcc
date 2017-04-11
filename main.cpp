#include <iostream>
#include <string>
#include <cstring>
#include "Scanner.hpp"
#include "Parser.hpp"

const std::string help =
  "Usage\n\n"
  "   <exec_file> [option] <file>\n\n"
  "Options\n\n"
  "   -s, --scanner          - Lexer -> print all lexems from <file> to &1\n"
  "   -p, --parser           - Parser -> print syntax trees of statements from <file> to &1\n"
  "   -e, --parse-expression - Parser -> print syntax tree of expressions from <file> to &1\n"
  "   -h, --help             - Print 'help'\n\n"
  "<file> <- 'test.pas', if not setted.\n";

int main(int argc, char const *argv[]) {
  if (argc < 2 || !std::strcmp(argv[1], "--help") || !std::strcmp(argv[1], "-h")) {
    std::cout << help << std::endl;
    return 0;
  }

  // SCANNER
  if (!std::strcmp(argv[1], "--scanner") || !std::strcmp(argv[1], "-s")) {
    compiler::Scanner scanner;
    compiler::Lexeme lex;
    if (argc > 2)
      scanner.open(argv[2]);
    else
      scanner.open("test.pas");
    do {
      scanner.nextLex();
      lex = scanner.lex();
      std::cout << lex.row << ':'
           << lex.column << '\t'
           << lex.tokenName() << '\t'
           << (unsigned long int)lex.tag << '\t'
           << lex.name
      << std::endl;
    } while (lex.token != compiler::Token::END_OF_FILE && lex.token != compiler::Token::UNDEFINED);
    return 0;
  }

  //PARSER
  if (!std::strcmp(argv[1], "--parser") || !std::strcmp(argv[1], "-p")) {
    compiler::Parser parser;
    if (argc > 2)
      parser.set(argv[2]);
    else
      parser.set("test.pas");
    try {
      parser.parse();
      std::cout << parser.print();
    } catch (std::exception& ex) {
      std::cout << ex.what() << std::endl;
    }
    return 0;
  }

  //PARSER OF EXPRESSIONS
  if (!std::strcmp(argv[1], "--parse-expression") || !std::strcmp(argv[1], "-e")) {
    compiler::Parser parser;
    if (argc > 2)
      parser.set(argv[2]);
    else
      parser.set("test.pas");
    try {
      parser.parseExpr();
      std::cout << parser.print();
    } catch (std::exception& ex) {
      std::cout << ex.what() << std::endl;
    }
    return 0;
  }

  std::cout << "unknown token: " << argv[1] << std::endl;
  std::cout << help << std::endl;

  return 0;
}
