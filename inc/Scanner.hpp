#pragma once

#include <string>
#include <fstream>
#include <map>
#include <cctype>
#include <algorithm>

namespace compiler {

enum class Token : unsigned short {
  UNDEFINED = 0,
  END_OF_FILE = 1,
  IDENTIFIER = 2,
  LITERAL = 3,
  PUNCTUATION = 4,
  OPERATOR = 5,
  SPACE = 6,
};

enum class Tag : unsigned long int {
  UNDEFINED,

  ABSOLUTE,
  ABSTRACT,
  ARRAY,
  AS,
  ASM,
  BEGIN,
  BREAK,
  B_FALSE,
  B_TRUE,
  CASE,
  CLASS,
  CONST,
  CONSTRUCTOR,
  CONTAINS,
  CONTINUE,
  DEFAULT,
  DESTRUCTOR,
  DO,
  DOWNTO,
  ELSE,
  END,
  EXCEPT,
  EXPORT,
  EXPORTS,
  EXTERNAL,
  EXIT,
  FILE,
  FINALIZATION,
  FINALLY,
  FOR,
  FORWARD,
  FUNCTION,
  GENERIC,
  GOTO,
  IF,
  IMPLEMENTATION,
  INDEX,
  INHERITED,
  INITIALIZATION,
  INLINE,
  INTERFACE,
  IS,
  LABEL,
  LIBRARY,
  NAME,
  OBJECT,
  OF,
  ON,
  OPERATOR,
  OVERRIDE,
  PACKED,
  PRIVATE,
  PROCEDURE,
  PROGRAM,
  PROPERTY,
  PROTECTED,
  PUBLIC,
  PUBLISHED,
  RAISE,
  READ,
  READLN,
  RECORD,
  REPEAT,
  REINTRODUCE,
  SET,
  SPECIALIZE,
  THEN,
  THREADVAR,
  TO,
  TRY,
  TYPE,
  UNIT,
  UNTIL,
  USES,
  VAR,
  VIEW,
  VIRTUAL,
  WHILE,
  WITH,
  WRITE,
  WRITELN,

  BYTE,
  SHORTINT,
  SMALLINT,
  WORD,
  CARDINAL,
  LONGINT,
  LONGWORD,
  INT64,
  QWORD,

  REAL,
  SINGLE,
  DOUBLE,
  EXTENDED,
  COMP,
  CURRENCY,

  BOOLEAN,
  BYTEBOOL,
  WORDBOOL,
  LONGBOOL,

  CHAR,
  WIDECHAR,
  PCHAR,

  NIL,             // -- 'nil'
  FLOAT,           // -- '12345.67890', '1e-5'
  STRING,          // -- 'abcde'
  CHARACTER,       // -- 'a', 'b', ...
  INTEGER,     // -- '1234567890'

  LEFT_BRACKET,      // -- '['
  RIGHT_BRACKET,     // -- ']'
  LEFT_PARENTHESIS,    // -- '('
  RIGHT_PARENTHESIS,   // -- ')'
  COMMA,           // -- ','
  COLON,           // -- ':'
  DOT,             // -- '.'
  DOUBLE_DOT,      // -- '..'
  SEMICOLON,       // -- ';'

  ADD,             // -- '+'
  AND,             // -- 'and'
  COLON_EQUALS,    // -- ':='
  DIV_INT,         // -- 'div'
  DIV_FLOAT,       // -- '/'
  EQUALS,          // -- '='
  GREATER,         // -- '>'
  GREATER_EQ,      // -- '>='
  IN,              // -- 'in'
  LESS,            // -- '<'
  LESS_EQ,         // -- '<='
  MOD,             // -- 'mod'
  MIS,             // -- '<>'
  MUL,             // -- '*'
  NOT,             // -- 'not'
  OR,              // -- 'or'
  SHL,             // -- 'shl'
  SHR,             // -- 'shr'
  SUB,             // -- '-'
  XOR,             // -- 'xor'
  POINTER,         // -- '^'
  ADDRESS,         // -- '@'

  COMMENT_OLD,     // -- '(*some words*)'
  COMMENT_TURBO,   // -- '{some words}'
  COMMENT_DELPHI,  // -- '//some words'
};

struct Lexeme {
  Lexeme () {};
  Lexeme ( const compiler::Lexeme& lex ) : name(lex.name), token(lex.token), tag(lex.tag), row(lex.row), column(lex.column) {};
  std::string tokenName () const;
  std::string name;
  compiler::Token token;
  compiler::Tag tag;
  unsigned long int row, column;
};

class Scanner {
public:
  Scanner ( void ) : token(Token::UNDEFINED), tag(Tag::UNDEFINED), row(0), column(0) {initMap();};
  Scanner ( const std::string& filename );
  void open ( const std::string& filename );
  void nextLex ( void );
  compiler::Lexeme lex ( void ) const;
private:
  void initMap ( void );
  void readId ( void );
  void readDec ( void );
  void readHex ( void );
  void readBin ( void );
  void readString ( void );
  void readPunct ( void );
  std::map<std::string, std::pair<Token, Tag> > book;
  std::ifstream input;
  std::string lexeme;
  Token token;
  Tag tag;
  unsigned long int row, column;
  int sym, countAph = 0;
  bool accDot = false;
};
};
