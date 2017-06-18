#include "../inc/Sym.hpp"

std::string compiler::SymVar::print ( unsigned int deep ) {
  std::ostringstream sstream;

  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << this->name << ':'
          << static_cast<char>(this->glob) << ':'
          << this->type->print(deep)
          << std::endl;

  return sstream.str();
};

compiler::TypeScalar::TypeScalar ( SCALAR_TYPE tp ) : SymType(Lexeme()) {
  this->name = tagBook.at(static_cast<compiler::Tag>(tp));
  this->tag = static_cast<compiler::Tag>(tp);
  this->column = this->row = 0;
  this->token = compiler::Token::IDENTIFIER;
  this->symType = SymEnum::Scalar;
};
