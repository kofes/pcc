#include <iostream>
#include "../inc/Parser.hpp"

using namespace std;

int main (int argc, char* argv[]) {
  compiler::Parser parser;
  compiler::Lexeme lex;
  if (argc > 1)
    parser.set(argv[1]);
  else
    parser.set("test.pas");
  try {
    parser.parse();
    cout << parser.print() << endl;
  } catch (std::exception& ex) {
    cout << ex.what() << endl;
  }
  return 0;
}
