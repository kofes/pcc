#include "../inc/Sym.hpp"

std::string compiler::SymVar::print ( unsigned int deep ) {
  std::ostringstream sstream;

  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << this->name << ':'
          << static_cast<char>(this->glob) << ':'
          << this->type->print(0) << ':'
          << this->value;
          // << std::endl;

  return sstream.str();
};

std::string compiler::SymFunc::print ( unsigned int deep ) {
  std::ostringstream sstream;

  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "Function:" << this->name << "\n"
          << "Params:\n";

  for (const std::pair< std::string, pSymVar >& elem : *(params.get()))
    sstream << elem.second->print(deep);

  sstream << "Variables:\n";

  for (const std::pair< std::string, pSymVar >& elem : varTable)
    sstream << elem.second->print(deep);

  //TODO: print body
  return sstream.str();
};

std::string compiler::TypeArray::print ( unsigned int deep ) {
  std::ostringstream sstream;

  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "ARRAY[" << low << ":" << high << "] OF "
          << elemType->print(0);

  return sstream.str();
};

std::string compiler::TypeRecord::print ( unsigned int deep ) {
  std::ostringstream sstream;

  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "RECORD OF:\n";

  for (const std::pair< std::string, pSymVar >& elem : *(field.get()))
    sstream << elem.second->print(deep) << '\n';

  return sstream.str();
};

std::string compiler::TypePointer::print ( unsigned int deep ) {
  std::ostringstream sstream;

  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "POINTER:" << elemType->print(0);

  return sstream.str();
};
//TAG: TYPE/ALIAS, NAME: nameNewType, TYPE: what is type was copied?
std::string compiler::TypeAlias::print ( unsigned int deep ) {
  std::ostringstream sstream;

  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "ALIAS OF:\n"
          << this->name << "="
          << type->print(0);

  return sstream.str();

};

std::string compiler::TypeScalar::print ( unsigned int deep ) {
  std::ostringstream sstream;

  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << this->name;

  return sstream.str();
};

compiler::TypeScalar::TypeScalar ( SCALAR_TYPE tp ) {
  this->name = tagBook.at(static_cast<compiler::Tag>(tp));
  this->type = tp;
  this->symType = SymEnum::Scalar;
};
