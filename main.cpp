#include <iostream>
#include "inc/Scanner.hpp"

using namespace std;

int main (int argc, char* argv[]) {
  Scanner scanner;
  if (argc > 1)
    scanner.open(argv[1]);
  else
    scanner.open("test.pas");
  do {
    scanner.nextLex();
    cout << scanner.getRowLex() << ':'
         << scanner.getColumnLex() << '\t'
         << scanner.getTokenName() << '\t'
         << (unsigned long int)scanner.getTagLex() << '\t'
         << scanner.getNameLex()
    << std::endl;
  } while (scanner.getTokenLex() != Scanner::Token::END_OF_FILE && scanner.getTokenLex() != Scanner::Token::UNDEFINED);
  return 0;
}
