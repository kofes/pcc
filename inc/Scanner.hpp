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

  OUT,
  IDENTIFIER,
};

static std::map<Tag, std::string> const tagBook = {
  {Tag::ABSOLUTE, "ABSOLUTE"},
  {Tag::ABSTRACT, "ABSTRACT"},
  {Tag::ARRAY, "ARRAY"},
  {Tag::AS, "AS"},
  {Tag::ASM, "ASM"},
  {Tag::BEGIN, "BEGIN"},
  {Tag::BREAK, "BREAK"},
  {Tag::B_FALSE, "FALSE"},
  {Tag::B_TRUE, "TRUE"},
  {Tag::CASE, "CASE"},
  {Tag::CLASS, "CLASS"},
  {Tag::CONST, "CONST"},
  {Tag::CONSTRUCTOR, "CONSTRUCTOR"},
  {Tag::CONTAINS, "CONTAINS"},
  {Tag::CONTINUE, "CONTINUE"},
  {Tag::DEFAULT, "DEFAULT"},
  {Tag::DESTRUCTOR, "DESTRUCTOR"},
  {Tag::DO, "DO"},
  {Tag::DOWNTO, "DOWNTO"},
  {Tag::ELSE, "ELSE"},
  {Tag::END, "END"},
  {Tag::EXCEPT, "EXCEPT"},
  {Tag::EXPORT, "EXPORT"},
  {Tag::EXPORTS, "EXPORTS"},
  {Tag::EXTERNAL, "EXTERNAL"},
  {Tag::EXIT, "EXIT"},
  {Tag::FILE, "FILE"},
  {Tag::FINALIZATION, "FINALIZATION"},
  {Tag::FINALLY, "FINALLY"},
  {Tag::FOR, "FOR"},
  {Tag::FORWARD, "FORWARD"},
  {Tag::FUNCTION, "FUNCTION"},
  {Tag::GENERIC, "GENERIC"},
  {Tag::GOTO, "GOTO"},
  {Tag::IF, "IF"},
  {Tag::IMPLEMENTATION, "IMPLEMENTATION"},
  {Tag::INDEX, "INDEX"},
  {Tag::INHERITED, "INHERITED"},
  {Tag::INITIALIZATION, "INITIALIZATION"},
  {Tag::INLINE, "INLINE"},
  {Tag::INTERFACE, "INTERFACE"},
  {Tag::IS, "IS"},
  {Tag::LABEL, "LABEL"},
  {Tag::LIBRARY, "LIBRARY"},
  {Tag::NAME, "NAME"},
  {Tag::OBJECT, "OBJECT"},
  {Tag::OF, "OF"},
  {Tag::ON, "ON"},
  {Tag::OPERATOR, "OPERATOR"},
  {Tag::OUT, "OUT"},
  {Tag::ELSE, "ELSE"},
  {Tag::OVERRIDE, "OVERRIDE"},
  {Tag::PACKED, "PACKED"},
  {Tag::PRIVATE, "PRIVATE"},
  {Tag::PROCEDURE, "PROCEDURE"},
  {Tag::PROGRAM, "PROGRAM"},
  {Tag::PROPERTY, "PROPERTY"},
  {Tag::PROTECTED, "PROTECTED"},
  {Tag::PUBLIC, "PUBLIC"},
  {Tag::PUBLISHED, "PUBLISHED"},
  {Tag::RAISE, "RAISE"},
  {Tag::READ, "READ"},
  {Tag::READLN, "READLN"},
  {Tag::RECORD, "RECORD"},
  {Tag::REPEAT, "REPEAT"},
  {Tag::REINTRODUCE, "REINTRODUCE"},
  {Tag::SET, "SET"},
  {Tag::SPECIALIZE, "SPECIALIZE"},
  {Tag::THEN, "THEN"},
  {Tag::THREADVAR, "THREADVAR"},
  {Tag::TO, "TO"},
  {Tag::TRY, "TRY"},
  {Tag::TYPE, "TYPE"},
  {Tag::UNIT, "UNIT"},
  {Tag::UNTIL, "UNTIL"},
  {Tag::USES, "USES"},
  {Tag::VAR, "VAR"},
  {Tag::VIEW, "VIEW"},
  {Tag::VIRTUAL, "VIRTUAL"},
  {Tag::WHILE, "WHILE"},
  {Tag::WITH, "WITH"},
  {Tag::WRITE, "WRITE"},
  {Tag::WRITELN, "WRITELN"},

  {Tag::BYTE, "BYTE"},
  {Tag::SHORTINT, "SHORTINT"},
  {Tag::SMALLINT, "SMALLINT"},
  {Tag::WORD, "WORD"},
  {Tag::CARDINAL, "CARDINAL"},
  {Tag::LONGINT, "LONGINT"},
  {Tag::LONGWORD, "LONGWORD"},
  {Tag::INT64, "INT64"},
  {Tag::QWORD, "QWORD"},

  {Tag::REAL, "REAL"},
  {Tag::SINGLE, "SINGLE"},
  {Tag::DOUBLE, "DOUBLE"},
  {Tag::EXTENDED, "EXTENDED"},
  {Tag::COMP, "COMP"},
  {Tag::CURRENCY, "CURRENCY"},

  {Tag::BOOLEAN, "BOOLEAN"},
  {Tag::BYTEBOOL, "BYTEBOOL"},
  {Tag::WORDBOOL, "WORDBOOL"},
  {Tag::LONGBOOL, "LONGBOOL"},

  {Tag::CHAR, "CHAR"},
  {Tag::WIDECHAR, "WIDECHAR"},
  {Tag::PCHAR, "PCHAR"},

  {Tag::NIL, "NIL"},
  {Tag::FLOAT, "FLOAT"},
  {Tag::STRING, "STRING"},
  {Tag::CHARACTER, "CHARACTER"},
  {Tag::INTEGER, "INTEGER"},

  {Tag::LEFT_BRACKET, "["},
  {Tag::RIGHT_BRACKET, "]"},
  {Tag::LEFT_PARENTHESIS, "("},
  {Tag::RIGHT_PARENTHESIS, ")"},
  {Tag::COMMA, ","},
  {Tag::COLON, ":"},
  {Tag::DOT, "."},
  {Tag::DOUBLE_DOT, ".."},
  {Tag::SEMICOLON, ";"},

  {Tag::ADD, "+"},
  {Tag::AND, "and"},
  {Tag::COLON_EQUALS, ":="},
  {Tag::DIV_INT, "div"},
  {Tag::DIV_FLOAT, "/"},
  {Tag::EQUALS, "="},
  {Tag::GREATER, ">"},
  {Tag::GREATER_EQ, ">="},
  {Tag::IN, "in"},
  {Tag::LESS, "<"},
  {Tag::LESS_EQ, "<="},
  {Tag::MOD, "mod"},
  {Tag::MIS, "<>"},
  {Tag::MUL, "*"},
  {Tag::NOT, "not"},
  {Tag::OR, "or"},
  {Tag::SHL, "shl"},
  {Tag::SHR, "shr"},
  {Tag::SUB, "-"},
  {Tag::XOR, "xor"},
  {Tag::POINTER, "^"},
  {Tag::ADDRESS, "@"}
};

struct Lexeme {
  Lexeme () {};
  Lexeme ( const Lexeme& lex ) : name(lex.name), token(lex.token), tag(lex.tag), row(lex.row), column(lex.column) {};
  std::string tokenName () const;
  std::string name;
  Token token;
  Tag tag;
  unsigned long int row, column;
};

class Scanner {
public:
  Scanner ( void ) : token(Token::UNDEFINED), tag(Tag::UNDEFINED), row(0), column(0) {initMap();};
  Scanner ( const std::string& filename );
  void open ( const std::string& filename );
  void next ( void );
  Lexeme lex ( void ) const;
private:
  void initMap ( void );
  void readId ( void );
  void readDec ( void );
  void readHex ( void );
  void readOct ( void );
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
}
