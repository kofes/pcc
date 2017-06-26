#include "../inc/Scanner.hpp"

compiler::Scanner::Scanner ( const std::string& filename ) {
  input.open(filename);
  token = Token::UNDEFINED;
  tag = Tag::UNDEFINED;
  row = 1;
  column = 0;
  initMap();
};

void compiler::Scanner::open ( const std::string& filename ) {
  input.close();
  input.open(filename);
  token = Token::UNDEFINED;
  tag = Tag::UNDEFINED;
  row = 1;
  column = 0;
  lexeme.clear();
};

void compiler::Scanner::next ( void ) {
  if (!input.is_open())
    return;

  if (token == Token::END_OF_FILE)
    return;

  lexeme.clear();
  countAph = 0;
  if (!column) { //if first entering
    sym = input.get();
    ++column;
  }

  while (!input.eof()) {
    if (accDot) {
      ++column;
      lexeme += '.';
      readPunct();
      return;
    }
    if (sym == '\t')
      ++column;
    else if (sym == '\n') {
      ++row;
      column = 0;
    } else if (std::isalpha(sym) || sym == '_') {
      lexeme += sym;
      readId();
      return;
    }
    if (std::isdigit(sym)) {
      lexeme += sym;
      readDec();
      return;
    }
    if (sym == '%') {
      readBin();
      return;
    }
    if (sym == '$') {
      readHex();
      return;
    }
    if (sym == '&') {
      readOct();
      return;
    }
    if (std::ispunct(sym)) {
      if (sym == '{') {
        while (!input.eof() && sym != '}') {
          ++column;
          sym = input.get();
          if (sym == '\n') {
            column = 0;
            ++row;
          }
        }
        ++column;
        sym = input.get();
        continue;
      }
      if (sym == '(') {
        ++column;
        sym = input.get();
        if (sym == '*') {
          char buff[2] = {0, 0};
          while (!input.eof()) {
            ++column;
            sym = input.get();
            if (sym == '\n') {
              column = 0;
              ++row;
            }
            buff[0] = buff[1];
            buff[1] = sym;
            if (buff[0] == '*' && buff[1] == ')')
              break;
          }
          ++column;
          sym = input.get();
          continue;
        }
        lexeme += '(';
        token = Token::PUNCTUATION;
        tag = Tag::LEFT_PARENTHESIS;
        return;
      }
      if (sym == '/') {
        ++column;//
        sym = input.get();
        if (sym == '/') {
          while (!input.eof() && sym != '\n')
            sym = input.get();
          column = 1;
          ++row;
          sym = input.get();
          continue;
        }
        lexeme += '/';
        token = Token::OPERATOR;
        tag = Tag::DIV_FLOAT;
        return;
      }
      if (sym == '\'' || sym == '#')
        readString();
      else
        readPunct();
      return;
    }
    ++column;
    sym = input.get();
  }
  lexeme.clear();
  token = Token::END_OF_FILE;
  tag = Tag::UNDEFINED;
};

void compiler::Scanner::readId ( void ) {
  sym = input.get();

  while (!input.eof() && (std::isalnum(sym) || sym == '_')) {
    lexeme += sym;
    ++column;
    sym = input.get();
  }
  if (sym == '\"' || sym == '\'') {
    token = Token::UNDEFINED;
    tag = Tag::UNDEFINED;
    return;
  }
  ++column;
  std::transform(lexeme.begin(), lexeme.end(), lexeme.begin(), ::toupper);

  auto iter = book.find(lexeme);
  if (iter == book.end()) {
    token = Token::IDENTIFIER;
    tag = Tag::IDENTIFIER;
  } else {
    token = iter->second.first;
    tag = iter->second.second;
  }
};

void compiler::Scanner::readDec ( void ) {
  sym = input.get();
  ++column;
  unsigned char countDots = 0;

  while (!input.eof() && (std::isdigit(sym) || sym == '.')) {
    if (sym == '.') ++countDots;
    if (countDots > 1) break;
    lexeme += sym;
    ++column;
    sym = input.get();
  }
  if (countDots == 2) {
    if (lexeme[lexeme.length() - 1] == '.') {
      accDot = true;
      --column;
      lexeme.pop_back();
      countDots = 0;
    } else {
      lexeme += sym;
      countAph = -1;
      token = Token::UNDEFINED;
      tag = Tag::UNDEFINED;
      return;
    }
  }
  if (sym == '\'' || sym == '_') {
    lexeme += sym;
    token = Token::UNDEFINED;
    tag = Tag::UNDEFINED;
    return;
  }

  token = Token::LITERAL;
  if (countDots) tag = Tag::FLOAT;
  else tag = Tag::INTEGER;
};

void compiler::Scanner::readBin ( void ) {
  sym = input.get();
  ++column;
  unsigned long long int sum = 0;
  std::string buff = "%";

  while (!input.eof() && (sym == '0' || sym == '1')) {
    buff += sym;
    sum <<= 1;
    sum += sym - '0';
    ++column;
    sym = input.get();
  }
  if (buff.length() == 1) {
    lexeme = buff;
    token = Token::UNDEFINED;
    tag = Tag::UNDEFINED;
    return;
  }
  if (sym == '\'' || sym == '_' || std::isalnum(sym) || sym == '.') {
    lexeme = buff;
    lexeme += sym;
    countAph = -1;
    token = Token::UNDEFINED;
    tag = Tag::UNDEFINED;
    return;
  }
  lexeme = std::to_string(sum);
  countAph = buff.length() - lexeme.length();

  token = Token::LITERAL;
  tag = Tag::INTEGER;
}

void compiler::Scanner::readOct ( void ) {
  sym = input.get();
  ++column;
  unsigned long long int sum = 0;
  std::string buff = "&";

  while (!input.eof() && std::isdigit(sym) && sym >= '0' && sym <= '7') {
    buff += sym;
    sum <<= 3;
    sum += sym - '0';
    ++column;
    sym = input.get();
  }
  if (buff.length() == 1) {
    lexeme = buff;
    token = Token::UNDEFINED;
    tag = Tag::UNDEFINED;
    return;
  }
  if (sym == '\'' || sym == '_' || std::isalpha(sym) || sym == '.') {
    lexeme = buff;
    lexeme += sym;
    countAph = -1;
    token = Token::UNDEFINED;
    tag = Tag::UNDEFINED;
    return;
  }
  lexeme = std::to_string(sum);
  countAph = buff.length() - lexeme.length();

  token = Token::LITERAL;
  tag = Tag::INTEGER;
};

void compiler::Scanner::readHex ( void ) {
  sym = input.get();
  ++column;
  unsigned long long int sum = 0;
  std::string buff = "$";

  while (!input.eof() && (std::isdigit(sym) || (std::toupper(sym) >= 'A' && std::toupper(sym) <= 'F'))) {
    buff += sym;
    sum <<= 4;
    if (std::isdigit(sym))
      sum += sym - '0';
    else
      sum += std::toupper(sym) - 'A' + 10;
    ++column;
    sym = input.get();
  }
  if (buff.length() == 1) {
    lexeme = buff;
    token = Token::UNDEFINED;
    tag = Tag::UNDEFINED;
    return;
  }
  if (sym == '\'' || sym == '_' || std::isalpha(sym) || sym == '.') {
    lexeme = buff;
    lexeme += sym;
    countAph = -1;
    token = Token::UNDEFINED;
    tag = Tag::UNDEFINED;
    return;
  }
  lexeme = std::to_string(sum);
  countAph = buff.length() - lexeme.length();

  token = Token::LITERAL;
  tag = Tag::INTEGER;
}

void compiler::Scanner::readString( void ) {
  int buff_char = 0;
  while (sym == '#' || sym == '\'') {
    if (sym == '#') { //ASCII
      std::string buff;
      sym = input.get();
      ++column;
      if (!std::isdigit(sym)) {
        lexeme += '#';
        token = Token::UNDEFINED;
        tag = Tag::UNDEFINED;
        return;
      }
      while (!input.eof() && std::isdigit(sym)) {
        buff += sym;
        ++column;
        sym = input.get();
      }
      int c = std::stoi(buff);
      if (c >= 256 || c <  0) {
        token = Token::UNDEFINED;
        tag = Tag::UNDEFINED;
        return;
      }
      countAph += buff.length();
      lexeme += c;
    } else {
      buff_char = sym;
      sym = input.get();
      ++column;
      while (!input.eof()) {
        if (sym == '\'') {
          buff_char = sym;
          sym = input.get();
          ++column;
          if (sym != '\'') {
            countAph+=2;
            break;
          }
          ++countAph;
        }
        lexeme += sym;
        buff_char = sym;
        ++column;
        sym = input.get();
      }
      if (input.eof() && buff_char != '\'') {
        countAph += 1;
        token = Token::UNDEFINED;
        tag = Tag::UNDEFINED;
        return;
      }
    }
  }

  token = Token::LITERAL;
  if (lexeme.length() == 1) tag = Tag::CHARACTER;
  else tag = Tag::STRING;
}

void compiler::Scanner::readPunct ( void ) {
  lexeme += sym;
  if ( accDot && sym == '.') {
    accDot = false;
    token = Token::PUNCTUATION;
    tag = Tag::DOUBLE_DOT;
    sym = input.get();
    ++column;
    return;
  }
  auto iter = book.find(lexeme);
  if (iter == book.end()) {
    ++column;
    sym = input.get();
    token = Token::UNDEFINED;
    tag = Tag::UNDEFINED;
    return;
  }
  if (sym == '.') {
    ++column;
    sym = input.get();
    if (sym == '.') {
      lexeme += sym;
      token = Token::PUNCTUATION;
      tag = Tag::DOUBLE_DOT;
      sym = input.get();
      ++column;
      return;
    }
    token = iter->second.first;
    tag = iter->second.second;
    return;
  }
  if (sym == ':') {
    ++column;
    sym = input.get();
    if (sym == '=') {
      lexeme += sym;
      token = Token::OPERATOR;
      tag = Tag::COLON_EQUALS;
      sym = input.get();
      ++column;
      return;
    }
    token = iter->second.first;
    tag = iter->second.second;
    return;
  }
  if (sym == '>') {
    ++column;
    sym = input.get();
    if (sym == '=') {
      lexeme += sym;
      token = Token::OPERATOR;
      tag = Tag::GREATER_EQ;
      sym = input.get();
      ++column;
      return;
    }
    token = iter->second.first;
    tag = iter->second.second;
    return;
  }
  if (sym == '<') {
    ++column;
    sym = input.get();
    if (sym == '=') {
      lexeme += sym;
      token = Token::OPERATOR;
      tag = Tag::LESS_EQ;
      sym = input.get();
      ++column;
      return;
    }
    if (sym == '>') {
      lexeme += sym;
      token = Token::OPERATOR;
      tag = Tag::MIS;
      sym = input.get();
      ++column;
      return;
    }
    token = iter->second.first;
    tag = iter->second.second;
    return;
  }
  token = iter->second.first;
  tag = iter->second.second;
  ++column;
  sym = input.get();
};

std::string compiler::Lexeme::tokenName ( void ) const {
  switch (token) {
    case (compiler::Token::END_OF_FILE) : return "END_OF_FILE";
    case (compiler::Token::IDENTIFIER) : return "IDENTIFIER";
    case (compiler::Token::LITERAL) : return "LITERAL";
    case (compiler::Token::PUNCTUATION) : return "PUNCTUATION";
    case (compiler::Token::OPERATOR) : return "OPERATOR";
    case (compiler::Token::SPACE) : return "SPACE";
    // case (compiler::Token::DIRECTIVE) : return "DIRECTIVE";
    default: break;
  };
  return "UNDEFINED";
};

compiler::Lexeme compiler::Scanner::lex ( void ) const {
  compiler::Lexeme res;
  res.column = column - lexeme.length() - countAph;
  res.row = row;
  res.tag = tag;
  res.token = token;
  res.name = lexeme;
  return res;
};

void compiler::Scanner::initMap ( void ) {
  book = {
    {"ABSOLUTE", std::make_pair(Token::IDENTIFIER, Tag::ABSOLUTE)},
    {"ABSTRACT", std::make_pair(Token::IDENTIFIER, Tag::ABSTRACT)},
    {"ARRAY", std::make_pair(Token::IDENTIFIER, Tag::ARRAY)},
    {"AS", std::make_pair(Token::IDENTIFIER, Tag::AS)},
    {"ASM", std::make_pair(Token::IDENTIFIER, Tag::ASM)},
    {"BEGIN", std::make_pair(Token::IDENTIFIER, Tag::BEGIN)},
    {"BREAK", std::make_pair(Token::IDENTIFIER, Tag::BREAK)},
    {"FALSE", std::make_pair(Token::IDENTIFIER, Tag::B_FALSE)},
    {"TRUE", std::make_pair(Token::IDENTIFIER, Tag::B_TRUE)},
    {"CASE", std::make_pair(Token::IDENTIFIER, Tag::CASE)},
    {"CLASS", std::make_pair(Token::IDENTIFIER, Tag::CLASS)},
    {"CONST", std::make_pair(Token::IDENTIFIER, Tag::CONST)},
    {"CONSTRUCTOR", std::make_pair(Token::IDENTIFIER, Tag::CONSTRUCTOR)},
    {"CONTAINS", std::make_pair(Token::IDENTIFIER, Tag::CONTAINS)},
    {"CONTINUE", std::make_pair(Token::IDENTIFIER, Tag::CONTINUE)},
    {"DEFAULT", std::make_pair(Token::IDENTIFIER, Tag::DEFAULT)},
    {"DESTRUCTOR", std::make_pair(Token::IDENTIFIER, Tag::DESTRUCTOR)},
    {"DO", std::make_pair(Token::IDENTIFIER, Tag::DO)},
    {"DOWNTO", std::make_pair(Token::IDENTIFIER, Tag::DOWNTO)},
    {"ELSE", std::make_pair(Token::IDENTIFIER, Tag::ELSE)},
    {"END", std::make_pair(Token::IDENTIFIER, Tag::END)},
    {"EXCEPT", std::make_pair(Token::IDENTIFIER, Tag::EXCEPT)},
    {"EXPORT", std::make_pair(Token::IDENTIFIER, Tag::EXPORT)},
    {"EXPORTS", std::make_pair(Token::IDENTIFIER, Tag::EXPORTS)},
    {"EXTERNAL", std::make_pair(Token::IDENTIFIER, Tag::EXTERNAL)},
    {"EXIT", std::make_pair(Token::IDENTIFIER, Tag::EXIT)},
    {"FILE", std::make_pair(Token::IDENTIFIER, Tag::FILE)},
    {"FINALIZATION", std::make_pair(Token::IDENTIFIER, Tag::FINALIZATION)},
    {"FINALLY", std::make_pair(Token::IDENTIFIER, Tag::FINALLY)},
    {"FOR", std::make_pair(Token::IDENTIFIER, Tag::FOR)},
    {"FORWARD", std::make_pair(Token::IDENTIFIER, Tag::FORWARD)},
    {"FUNCTION", std::make_pair(Token::IDENTIFIER, Tag::FUNCTION)},
    {"GENERIC", std::make_pair(Token::IDENTIFIER, Tag::GENERIC)},
    {"GOTO", std::make_pair(Token::IDENTIFIER, Tag::GOTO)},
    {"IF", std::make_pair(Token::IDENTIFIER, Tag::IF)},
    {"IMPLEMENTATION", std::make_pair(Token::IDENTIFIER, Tag::IMPLEMENTATION)},
    {"INDEX", std::make_pair(Token::IDENTIFIER, Tag::INDEX)},
    {"INHERITED", std::make_pair(Token::IDENTIFIER, Tag::INHERITED)},
    {"INITIALIZATION", std::make_pair(Token::IDENTIFIER, Tag::INITIALIZATION)},
    {"INLINE", std::make_pair(Token::IDENTIFIER, Tag::INLINE)},
    {"INTERFACE", std::make_pair(Token::IDENTIFIER, Tag::INTERFACE)},
    {"IS", std::make_pair(Token::IDENTIFIER, Tag::IS)},
    {"LABEL", std::make_pair(Token::IDENTIFIER, Tag::LABEL)},
    {"LIBRARY", std::make_pair(Token::IDENTIFIER, Tag::LIBRARY)},
    {"NAME", std::make_pair(Token::IDENTIFIER, Tag::NAME)},
    {"OBJECT", std::make_pair(Token::IDENTIFIER, Tag::OBJECT)},
    {"OF", std::make_pair(Token::IDENTIFIER, Tag::OF)},
    {"ON", std::make_pair(Token::IDENTIFIER, Tag::ON)},
    {"OPERATOR", std::make_pair(Token::IDENTIFIER, Tag::OPERATOR)},
    {"OUT", std::make_pair(Token::IDENTIFIER, Tag::OUT)},
    {"ELSE", std::make_pair(Token::IDENTIFIER, Tag::ELSE)},
    {"OVERRIDE", std::make_pair(Token::IDENTIFIER, Tag::OVERRIDE)},
    {"PACKED", std::make_pair(Token::IDENTIFIER, Tag::PACKED)},
    {"PRIVATE", std::make_pair(Token::IDENTIFIER, Tag::PRIVATE)},
    {"PROCEDURE", std::make_pair(Token::IDENTIFIER, Tag::PROCEDURE)},
    {"PROGRAM", std::make_pair(Token::IDENTIFIER, Tag::PROGRAM)},
    {"PROPERTY", std::make_pair(Token::IDENTIFIER, Tag::PROPERTY)},
    {"PROTECTED", std::make_pair(Token::IDENTIFIER, Tag::PROTECTED)},
    {"PUBLIC", std::make_pair(Token::IDENTIFIER, Tag::PUBLIC)},
    {"PUBLISHED", std::make_pair(Token::IDENTIFIER, Tag::PUBLISHED)},
    {"RAISE", std::make_pair(Token::IDENTIFIER, Tag::RAISE)},
    {"READ", std::make_pair(Token::IDENTIFIER, Tag::READ)},
    {"READLN", std::make_pair(Token::IDENTIFIER, Tag::READLN)},
    {"RECORD", std::make_pair(Token::IDENTIFIER, Tag::RECORD)},
    {"REPEAT", std::make_pair(Token::IDENTIFIER, Tag::REPEAT)},
    {"REINTRODUCE", std::make_pair(Token::IDENTIFIER, Tag::REINTRODUCE)},
    {"SET", std::make_pair(Token::IDENTIFIER, Tag::SET)},
    {"SPECIALIZE", std::make_pair(Token::IDENTIFIER, Tag::SPECIALIZE)},
    {"THEN", std::make_pair(Token::IDENTIFIER, Tag::THEN)},
    {"THREADVAR", std::make_pair(Token::IDENTIFIER, Tag::THREADVAR)},
    {"TO", std::make_pair(Token::IDENTIFIER, Tag::TO)},
    {"TRY", std::make_pair(Token::IDENTIFIER, Tag::TRY)},
    {"TYPE", std::make_pair(Token::IDENTIFIER, Tag::TYPE)},
    {"UNIT", std::make_pair(Token::IDENTIFIER, Tag::UNIT)},
    {"UNTIL", std::make_pair(Token::IDENTIFIER, Tag::UNTIL)},
    {"USES", std::make_pair(Token::IDENTIFIER, Tag::USES)},
    {"VAR", std::make_pair(Token::IDENTIFIER, Tag::VAR)},
    {"VIEW", std::make_pair(Token::IDENTIFIER, Tag::VIEW)},
    {"VIRTUAL", std::make_pair(Token::IDENTIFIER, Tag::VIRTUAL)},
    {"WHILE", std::make_pair(Token::IDENTIFIER, Tag::WHILE)},
    {"WITH", std::make_pair(Token::IDENTIFIER, Tag::WITH)},
    {"WRITE", std::make_pair(Token::IDENTIFIER, Tag::WRITE)},
    {"WRITELN", std::make_pair(Token::IDENTIFIER, Tag::WRITELN)},

    {"BYTE", std::make_pair(Token::IDENTIFIER, Tag::BYTE )},
    {"SHORTINT", std::make_pair(Token::IDENTIFIER, Tag::SHORTINT )},
    {"SMALLINT", std::make_pair(Token::IDENTIFIER, Tag::SMALLINT )},
    {"WORD", std::make_pair(Token::IDENTIFIER, Tag::WORD )},
    {"INTEGER", std::make_pair(Token::IDENTIFIER, Tag::INTEGER )},
    {"CARDINAL", std::make_pair(Token::IDENTIFIER, Tag::CARDINAL )},
    {"LONGINT", std::make_pair(Token::IDENTIFIER, Tag::LONGINT )},
    {"LONGWORD", std::make_pair(Token::IDENTIFIER, Tag::LONGWORD )},
    {"INT64", std::make_pair(Token::IDENTIFIER, Tag::INT64 )},
    {"QWORD", std::make_pair(Token::IDENTIFIER, Tag::QWORD )},

    {"REAL", std::make_pair(Token::IDENTIFIER, Tag::REAL)},
    {"SINGLE", std::make_pair(Token::IDENTIFIER, Tag::SINGLE)},
    {"DOUBLE", std::make_pair(Token::IDENTIFIER, Tag::DOUBLE)},
    {"EXTENDED", std::make_pair(Token::IDENTIFIER, Tag::EXTENDED)},
    {"COMP", std::make_pair(Token::IDENTIFIER, Tag::COMP)},
    {"CURRENCY", std::make_pair(Token::IDENTIFIER, Tag::CURRENCY)},

    {"BOOLEAN", std::make_pair(Token::IDENTIFIER, Tag::BOOLEAN)},
    {"BYTEBOOL", std::make_pair(Token::IDENTIFIER, Tag::BYTEBOOL)},
    {"WORDBOOL", std::make_pair(Token::IDENTIFIER, Tag::WORDBOOL)},
    {"LONGBOOL", std::make_pair(Token::IDENTIFIER, Tag::LONGBOOL)},

    {"CHAR", std::make_pair(Token::IDENTIFIER, Tag::CHAR)},
    {"WIDECHAR", std::make_pair(Token::IDENTIFIER, Tag::WIDECHAR)},
    {"PCHAR", std::make_pair(Token::IDENTIFIER, Tag::PCHAR)},

    {"NIL", std::make_pair(Token::LITERAL, Tag::NIL)},

    {"[", std::make_pair(Token::PUNCTUATION, Tag::LEFT_BRACKET)},       // -- '['
    {"]", std::make_pair(Token::PUNCTUATION, Tag::RIGHT_BRACKET)},      // -- ']'
    {"(", std::make_pair(Token::PUNCTUATION, Tag::LEFT_PARENTHESIS)},     // -- '('
    {")", std::make_pair(Token::PUNCTUATION, Tag::RIGHT_PARENTHESIS)},    // -- ')'
    {",", std::make_pair(Token::PUNCTUATION, Tag::COMMA)},            // -- ','
    {":", std::make_pair(Token::PUNCTUATION, Tag::COLON)},            // -- ':'
    {".", std::make_pair(Token::PUNCTUATION, Tag::DOT)},              // -- '.'
    {"..", std::make_pair(Token::PUNCTUATION, Tag::DOUBLE_DOT)},       // -- '..'
    {";", std::make_pair(Token::PUNCTUATION, Tag::SEMICOLON)},        // -- ';'

    {"+", std::make_pair(Token::OPERATOR, Tag::ADD)},             // -- '+'
    {":=", std::make_pair(Token::OPERATOR, Tag::COLON_EQUALS)},    // -- ':='
    {"/", std::make_pair(Token::OPERATOR, Tag::DIV_FLOAT)},       // -- '/'
    {"=", std::make_pair(Token::OPERATOR, Tag::EQUALS)},          // -- '='
    {">", std::make_pair(Token::OPERATOR, Tag::GREATER)},         // -- '>'
    {">=", std::make_pair(Token::OPERATOR, Tag::GREATER_EQ)},      // -- '>='
    {"<", std::make_pair(Token::OPERATOR, Tag::LESS)},            // -- '<'
    {"<=", std::make_pair(Token::OPERATOR, Tag::LESS_EQ)},         // -- '<='
    {"<>", std::make_pair(Token::OPERATOR, Tag::MIS)},             // -- '<>'
    {"*", std::make_pair(Token::OPERATOR, Tag::MUL)},             // -- '*'
    {"-", std::make_pair(Token::OPERATOR, Tag::SUB)},             // -- '-'
    {"^", std::make_pair(Token::OPERATOR, Tag::POINTER)},         // -- '^'
    {"@", std::make_pair(Token::OPERATOR, Tag::ADDRESS)},         // -- '@'
    {"AND", std::make_pair(Token::OPERATOR, Tag::AND)},             // -- 'and'
    {"DIV", std::make_pair(Token::OPERATOR, Tag::DIV_INT)},         // -- 'div'
    {"IN", std::make_pair(Token::OPERATOR, Tag::IN)},              // -- 'in'
    {"MOD", std::make_pair(Token::OPERATOR, Tag::MOD)},             // -- 'mod'
    {"NOT", std::make_pair(Token::OPERATOR, Tag::NOT)},             // -- 'not'
    {"OR", std::make_pair(Token::OPERATOR, Tag::OR)},              // -- 'or'
    {"SHL", std::make_pair(Token::OPERATOR, Tag::SHL)},             // -- 'shl'
    {"SHR", std::make_pair(Token::OPERATOR, Tag::SHR)},             // -- 'shr'
    {"XOR", std::make_pair(Token::OPERATOR, Tag::XOR)},             // -- 'xor'
  };

};
