#pragma once

#include <string>
#include <fstream>
#include <map>
#include <cctype>
#include <algorithm>

class Scanner {
public:
  enum class Token : unsigned short {
    UNDEFINED = 0,
    END_OF_FILE = 1,
    IDENTIFICATOR = 2,
    LITERAL = 3,
    PUNCTUATION = 4,
    OPERATOR = 5,
    SPACE = 6,
    DIRECTIVE = 7
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
    INTEGER,
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
    INTEGER_BIN,     // -- '%01'
    INTEGER_HEX,     // -- '$0123456789ABCDEF'
    INTEGER_DEC,     // -- '1234567890'

    LEFT_BRACE,      // -- '['
    RIGHT_BRACE,     // -- ']'
    LEFT_PARENTHESE, // -- '{'
    RIGHT_PARENTHESE,// -- '}'
    LEFT_BRACKET,    // -- '('
    RIGHT_BRACKET,   // -- ')'
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

    DIRECTIVE        // -- '{$keyword}'
  };
  Scanner () : token(Token::UNDEFINED), tag(Tag::UNDEFINED), row(0), column(0), countID(0) {initMap();};
  Scanner ( const std::string& filename );
  void open ( const std::string& filename );
  void nextLex ();
  std::string getNameLex () const;
  Token getTokenLex () const;
  std::string getTokenName () const;
  unsigned long int getTagLex () const;
  unsigned long int getRowLex () const;
  unsigned long int getColumnLex () const;
private:
  void initMap ( void );
  void readID ( void );
  void readDigit ( void );
  void readString ( void );
  void readPunct ( void );
  std::map<std::string, std::pair<Token, Tag> > book;
  std::ifstream input;
  std::string lexeme;
  Token token;
  Tag tag;
  unsigned long int row, column, countID;
  int sym, countAph = 0;
  bool accDot = false;
};
