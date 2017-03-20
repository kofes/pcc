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

void compiler::Scanner::nextLex () {
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
        tag = Tag::LEFT_BRACKET;
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
    token = Token::IDENTIFICATOR;
    tag = Tag::UNDEFINED;
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
  if (countDots == 2)
    if (lexeme[lexeme.length() - 1] == '.') {
      accDot = true;
      --column;
      lexeme.pop_back();
    } else {
      lexeme += sym;
      countAph = -1;
      token = Token::UNDEFINED;
      tag = Tag::UNDEFINED;
      return;
    }
  if (!std::isspace(sym) && !std::ispunct(sym) || sym == '\'' || sym == '_') {
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
  unsigned int count = 0;
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
  if (!std::isspace(sym) && !std::ispunct(sym) || sym == '\'' || sym == '_' || std::isalnum(sym) || sym == '.') {
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

void compiler::Scanner::readHex ( void ) {
  sym = input.get();
  ++column;
  unsigned long long int sum = 0;
  unsigned int count = 0;
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
    // countAph = -1;
    token = Token::UNDEFINED;
    tag = Tag::UNDEFINED;
    return;
  }
  if (!std::isspace(sym) && !std::ispunct(sym) || sym == '\'' || sym == '_' || std::isalpha(sym) || sym == '.') {
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
  tag = Tag::STRING;
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

std::string compiler::Lexeme::tokenName () const {
  switch (token) {
    case (compiler::Token::END_OF_FILE) : return "END_OF_FILE";
    case (compiler::Token::IDENTIFICATOR) : return "IDENTIFICATOR";
    case (compiler::Token::LITERAL) : return "LITERAL";
    case (compiler::Token::PUNCTUATION) : return "PUNCTUATION";
    case (compiler::Token::OPERATOR) : return "OPERATOR";
    case (compiler::Token::SPACE) : return "SPACE";
    // case (compiler::Token::DIRECTIVE) : return "DIRECTIVE";
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
  book["ABSOLUTE"] = std::make_pair(Token::IDENTIFICATOR, Tag::ABSOLUTE);
  book["ABSTRACT"] = std::make_pair(Token::IDENTIFICATOR, Tag::ABSTRACT);
  book["ARRAY"] = std::make_pair(Token::IDENTIFICATOR, Tag::ARRAY);
  book["AS"] = std::make_pair(Token::IDENTIFICATOR, Tag::AS);
  book["ASM"] = std::make_pair(Token::IDENTIFICATOR, Tag::ASM);
  book["BEGIN"] = std::make_pair(Token::IDENTIFICATOR, Tag::BEGIN);
  book["BREAK"] = std::make_pair(Token::IDENTIFICATOR, Tag::BREAK);
  book["FALSE"] = std::make_pair(Token::IDENTIFICATOR, Tag::B_FALSE);
  book["TRUE"] = std::make_pair(Token::IDENTIFICATOR, Tag::B_TRUE);
  book["CASE"] = std::make_pair(Token::IDENTIFICATOR, Tag::CASE);
  book["CLASS"] = std::make_pair(Token::IDENTIFICATOR, Tag::CLASS);
  book["CONST"] = std::make_pair(Token::IDENTIFICATOR, Tag::CONST);
  book["CONSTRUCTOR"] = std::make_pair(Token::IDENTIFICATOR, Tag::CONSTRUCTOR);
  book["CONTAINS"] = std::make_pair(Token::IDENTIFICATOR, Tag::CONTAINS);
  book["CONTINUE"] = std::make_pair(Token::IDENTIFICATOR, Tag::CONTINUE);
  book["DEFAULT"] = std::make_pair(Token::IDENTIFICATOR, Tag::DEFAULT);
  book["DESTRUCTOR"] = std::make_pair(Token::IDENTIFICATOR, Tag::DESTRUCTOR);
  book["DO"] = std::make_pair(Token::IDENTIFICATOR, Tag::DO);
  book["DOWNTO"] = std::make_pair(Token::IDENTIFICATOR, Tag::DOWNTO);
  book["ELSE"] = std::make_pair(Token::IDENTIFICATOR, Tag::ELSE);
  book["END"] = std::make_pair(Token::IDENTIFICATOR, Tag::END);
  book["EXCEPT"] = std::make_pair(Token::IDENTIFICATOR, Tag::EXCEPT);
  book["EXPORT"] = std::make_pair(Token::IDENTIFICATOR, Tag::EXPORT);
  book["EXPORTS"] = std::make_pair(Token::IDENTIFICATOR, Tag::EXPORTS);
  book["EXTERNAL"] = std::make_pair(Token::IDENTIFICATOR, Tag::EXTERNAL);
  book["EXIT"] = std::make_pair(Token::IDENTIFICATOR, Tag::EXIT);
  book["FILE"] = std::make_pair(Token::IDENTIFICATOR, Tag::FILE);
  book["FINALIZATION"] = std::make_pair(Token::IDENTIFICATOR, Tag::FINALIZATION);
  book["FINALLY"] = std::make_pair(Token::IDENTIFICATOR, Tag::FINALLY);
  book["FOR"] = std::make_pair(Token::IDENTIFICATOR, Tag::FOR);
  book["FORWARD"] = std::make_pair(Token::IDENTIFICATOR, Tag::FORWARD);
  book["FUNCTION"] = std::make_pair(Token::IDENTIFICATOR, Tag::FUNCTION);
  book["GENERIC"] = std::make_pair(Token::IDENTIFICATOR, Tag::GENERIC);
  book["GOTO"] = std::make_pair(Token::IDENTIFICATOR, Tag::GOTO);
  book["IF"] = std::make_pair(Token::IDENTIFICATOR, Tag::IF);
  book["IMPLEMENTATION"] = std::make_pair(Token::IDENTIFICATOR, Tag::IMPLEMENTATION);
  book["INDEX"] = std::make_pair(Token::IDENTIFICATOR, Tag::INDEX);
  book["INHERITED"] = std::make_pair(Token::IDENTIFICATOR, Tag::INHERITED);
  book["INITIALIZATION"] = std::make_pair(Token::IDENTIFICATOR, Tag::INITIALIZATION);
  book["INLINE"] = std::make_pair(Token::IDENTIFICATOR, Tag::INLINE);
  book["INTERFACE"] = std::make_pair(Token::IDENTIFICATOR, Tag::INTERFACE);
  book["IS"] = std::make_pair(Token::IDENTIFICATOR, Tag::IS);
  book["LABEL"] = std::make_pair(Token::IDENTIFICATOR, Tag::LABEL);
  book["LIBRARY"] = std::make_pair(Token::IDENTIFICATOR, Tag::LIBRARY);
  book["NAME"] = std::make_pair(Token::IDENTIFICATOR, Tag::NAME);
  book["OBJECT"] = std::make_pair(Token::IDENTIFICATOR, Tag::OBJECT);
  book["OF"] = std::make_pair(Token::IDENTIFICATOR, Tag::OF);
  book["ON"] = std::make_pair(Token::IDENTIFICATOR, Tag::ON);
  book["OPERATOR"] = std::make_pair(Token::IDENTIFICATOR, Tag::OPERATOR);
  book["ELSE"] = std::make_pair(Token::IDENTIFICATOR, Tag::ELSE);
  book["OVERRIDE"] = std::make_pair(Token::IDENTIFICATOR, Tag::OVERRIDE);
  book["PACKED"] = std::make_pair(Token::IDENTIFICATOR, Tag::PACKED);
  book["PRIVATE"] = std::make_pair(Token::IDENTIFICATOR, Tag::PRIVATE);
  book["PROCEDURE"] = std::make_pair(Token::IDENTIFICATOR, Tag::PROCEDURE);
  book["PROGRAM"] = std::make_pair(Token::IDENTIFICATOR, Tag::PROGRAM);
  book["PROPERTY"] = std::make_pair(Token::IDENTIFICATOR, Tag::PROPERTY);
  book["PROTECTED"] = std::make_pair(Token::IDENTIFICATOR, Tag::PROTECTED);
  book["PUBLIC"] = std::make_pair(Token::IDENTIFICATOR, Tag::PUBLIC);
  book["PUBLISHED"] = std::make_pair(Token::IDENTIFICATOR, Tag::PUBLISHED);
  book["RAISE"] = std::make_pair(Token::IDENTIFICATOR, Tag::RAISE);
  book["READ"] = std::make_pair(Token::IDENTIFICATOR, Tag::READ);
  book["READLN"] = std::make_pair(Token::IDENTIFICATOR, Tag::READLN);
  book["RECORD"] = std::make_pair(Token::IDENTIFICATOR, Tag::RECORD);
  book["REPEAT"] = std::make_pair(Token::IDENTIFICATOR, Tag::REPEAT);
  book["REINTRODUCE"] = std::make_pair(Token::IDENTIFICATOR, Tag::REINTRODUCE);
  book["SET"] = std::make_pair(Token::IDENTIFICATOR, Tag::SET);
  book["SPECIALIZE"] = std::make_pair(Token::IDENTIFICATOR, Tag::SPECIALIZE);
  book["THEN"] = std::make_pair(Token::IDENTIFICATOR, Tag::THEN);
  book["THREADVAR"] = std::make_pair(Token::IDENTIFICATOR, Tag::THREADVAR);
  book["TO"] = std::make_pair(Token::IDENTIFICATOR, Tag::TO);
  book["TRY"] = std::make_pair(Token::IDENTIFICATOR, Tag::TRY);
  book["TYPE"] = std::make_pair(Token::IDENTIFICATOR, Tag::TYPE);
  book["UNIT"] = std::make_pair(Token::IDENTIFICATOR, Tag::UNIT);
  book["UNTIL"] = std::make_pair(Token::IDENTIFICATOR, Tag::UNTIL);
  book["USES"] = std::make_pair(Token::IDENTIFICATOR, Tag::USES);
  book["VAR"] = std::make_pair(Token::IDENTIFICATOR, Tag::VAR);
  book["VIEW"] = std::make_pair(Token::IDENTIFICATOR, Tag::VIEW);
  book["VIRTUAL"] = std::make_pair(Token::IDENTIFICATOR, Tag::VIRTUAL);
  book["WHILE"] = std::make_pair(Token::IDENTIFICATOR, Tag::WHILE);
  book["WITH"] = std::make_pair(Token::IDENTIFICATOR, Tag::WITH);
  book["WRITE"] = std::make_pair(Token::IDENTIFICATOR, Tag::WRITE);
  book["WRITELN"] = std::make_pair(Token::IDENTIFICATOR, Tag::WRITELN);

  book["BYTE"] = std::make_pair(Token::IDENTIFICATOR, Tag::BYTE );
  book["SHORTINT"] = std::make_pair(Token::IDENTIFICATOR, Tag::SHORTINT );
  book["SMALLINT"] = std::make_pair(Token::IDENTIFICATOR, Tag::SMALLINT );
  book["WORD"] = std::make_pair(Token::IDENTIFICATOR, Tag::WORD );
  book["INTEGER"] = std::make_pair(Token::IDENTIFICATOR, Tag::INTEGER );
  book["CARDINAL"] = std::make_pair(Token::IDENTIFICATOR, Tag::CARDINAL );
  book["LONGINT"] = std::make_pair(Token::IDENTIFICATOR, Tag::LONGINT );
  book["LONGWORD"] = std::make_pair(Token::IDENTIFICATOR, Tag::LONGWORD );
  book["INT64"] = std::make_pair(Token::IDENTIFICATOR, Tag::INT64 );
  book["QWORD"] = std::make_pair(Token::IDENTIFICATOR, Tag::QWORD );

  book["REAL"] = std::make_pair(Token::IDENTIFICATOR, Tag::REAL);
  book["SINGLE"] = std::make_pair(Token::IDENTIFICATOR, Tag::SINGLE);
  book["DOUBLE"] = std::make_pair(Token::IDENTIFICATOR, Tag::DOUBLE);
  book["EXTENDED"] = std::make_pair(Token::IDENTIFICATOR, Tag::EXTENDED);
  book["COMP"] = std::make_pair(Token::IDENTIFICATOR, Tag::COMP);
  book["CURRENCY"] = std::make_pair(Token::IDENTIFICATOR, Tag::CURRENCY);

  book["BOOLEAN"] = std::make_pair(Token::IDENTIFICATOR, Tag::BOOLEAN);
  book["BYTEBOOL"] = std::make_pair(Token::IDENTIFICATOR, Tag::BYTEBOOL);
  book["WORDBOOL"] = std::make_pair(Token::IDENTIFICATOR, Tag::WORDBOOL);
  book["LONGBOOL"] = std::make_pair(Token::IDENTIFICATOR, Tag::LONGBOOL);

  book["CHAR"] = std::make_pair(Token::IDENTIFICATOR, Tag::CHAR);
  book["WIDECHAR"] = std::make_pair(Token::IDENTIFICATOR, Tag::WIDECHAR);
  book["PCHAR"] = std::make_pair(Token::IDENTIFICATOR, Tag::PCHAR);

  book["NIL"] = std::make_pair(Token::LITERAL, Tag::NIL);

  book["["] = std::make_pair(Token::PUNCTUATION, Tag::LEFT_BRACE);       // -- '['
  book["]"] = std::make_pair(Token::PUNCTUATION, Tag::RIGHT_BRACE);      // -- ']'
  // book["{"] = std::make_pair(Token::PUNCTUATION, Tag::LEFT_PARENTHESE);  // -- '{'
  // book["}"] = std::make_pair(Token::PUNCTUATION, Tag::RIGHT_PARENTHESE); // -- '}'
  book["("] = std::make_pair(Token::PUNCTUATION, Tag::LEFT_BRACKET);     // -- '('
  book[")"] = std::make_pair(Token::PUNCTUATION, Tag::RIGHT_BRACKET);    // -- ')'
  book[","] = std::make_pair(Token::PUNCTUATION, Tag::COMMA);            // -- ','
  book[":"] = std::make_pair(Token::PUNCTUATION, Tag::COLON);            // -- ':'
  book["."] = std::make_pair(Token::PUNCTUATION, Tag::DOT);              // -- '.'
  book[".."] = std::make_pair(Token::PUNCTUATION, Tag::DOUBLE_DOT);       // -- '..'
  book[";"] = std::make_pair(Token::PUNCTUATION, Tag::SEMICOLON);        // -- ';'

  book["+"] = std::make_pair(Token::OPERATOR, Tag::ADD);             // -- '+'
  book["AND"] = std::make_pair(Token::OPERATOR, Tag::AND);             // -- 'and'
  book[":="] = std::make_pair(Token::OPERATOR, Tag::COLON_EQUALS);    // -- ':='
  book["DIV"] = std::make_pair(Token::OPERATOR, Tag::DIV_INT);         // -- 'div'
  book["/"] = std::make_pair(Token::OPERATOR, Tag::DIV_FLOAT);       // -- '/'
  book["="] = std::make_pair(Token::OPERATOR, Tag::EQUALS);          // -- '='
  book[">"] = std::make_pair(Token::OPERATOR, Tag::GREATER);         // -- '>'
  book[">="] = std::make_pair(Token::OPERATOR, Tag::GREATER_EQ);      // -- '>='
  book["IN"] = std::make_pair(Token::OPERATOR, Tag::IN);              // -- 'in'
  book["<"] = std::make_pair(Token::OPERATOR, Tag::LESS);            // -- '<'
  book["<="] = std::make_pair(Token::OPERATOR, Tag::LESS_EQ);         // -- '<='
  book["MOD"] = std::make_pair(Token::OPERATOR, Tag::MOD);             // -- 'mod'
  book["<>"] = std::make_pair(Token::OPERATOR, Tag::MIS);             // -- '<>'
  book["*"] = std::make_pair(Token::OPERATOR, Tag::MUL);             // -- '*'
  book["NOT"] = std::make_pair(Token::OPERATOR, Tag::NOT);             // -- 'not'
  book["OR"] = std::make_pair(Token::OPERATOR, Tag::OR);              // -- 'or'
  book["SHL"] = std::make_pair(Token::OPERATOR, Tag::SHL);             // -- 'shl'
  book["SHR"] = std::make_pair(Token::OPERATOR, Tag::SHR);             // -- 'shr'
  book["-"] = std::make_pair(Token::OPERATOR, Tag::SUB);             // -- '-'
  book["XOR"] = std::make_pair(Token::OPERATOR, Tag::XOR);             // -- 'xor'
  book["^"] = std::make_pair(Token::OPERATOR, Tag::POINTER);         // -- '^'
  book["@"] = std::make_pair(Token::OPERATOR, Tag::ADDRESS);         // -- '@'
};
