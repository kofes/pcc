#include <iostream>
#include "inc/Scanner.hpp"

using namespace std;

int main (int argc, char* argv[]) {
  compiler::Scanner scanner;
  compiler::Lexeme lex;
  if (argc > 1)
    scanner.open(argv[1]);
  else
    scanner.open("test.pas");
  do {
    scanner.nextLex();
    lex = scanner.lex();
    cout << lex.row << ':'
         << lex.column << '\t'
         << lex.tokenName() << '\t'
         << (unsigned long int)lex.tag << '\t'
         << lex.name
    << std::endl;
  } while (lex.token != compiler::Token::END_OF_FILE && lex.token != compiler::Token::UNDEFINED);
  return 0;
}
