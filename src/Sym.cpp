#include "../inc/Sym.hpp"

std::string compiler::SymVar::print ( unsigned int deep ) {
  std::ostringstream sstream;

  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << this->name << ':'
          << static_cast<char>(this->glob) << ':';
  if (type == nullptr)
      sstream << "<untyped>:";
  else
      sstream << this->type->print(0) << ':';
  sstream << this->value;

  return sstream.str();
};

std::string compiler::SymFunc::print ( unsigned int deep ) {
  std::ostringstream sstream;

  if (retType != nullptr) {
      sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
              << "<function>:" << this->name << "\n";
      sstream << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
              << "<return's type>:\n";
      sstream << retType->print(deep+2) << '\n';
  } else {
      sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
              << "<procedure>:" << this->name << "\n";
  }
  if (params != nullptr) {
      sstream << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
              << "<params>:\n";
      for (const std::pair< std::string, pSymVar >& elem : *(params.get()))
        sstream << elem.second->print(deep+2) << '\n';
  }
  if (varTable.size()) {
      sstream << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
              << "<variables>:\n";
      for (const std::pair< std::string, pSymVar >& elem : varTable)
        sstream << elem.second->print(deep+2) << '\n';
  }
  sstream << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<body>:\n";
  body->print(deep+2);
  sstream << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<end>\n";
  return sstream.str();
};

std::string compiler::TypeArray::print ( unsigned int deep ) {
  std::ostringstream sstream;

  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "ARRAY";
  if (low < high)
    sstream << "[" << low << ":" << high << "]";
  sstream << " OF "
          << elemType->print(0);
  if (values.size()) {
    sstream << '(' << values.front()->value;
    for (unsigned long long i = low+1; i <= high; ++i)
      sstream << ", " << values[i-low]->value;
    sstream << ')';
  }

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
          << "^(" << elemType->print(0) << ')';

  return sstream.str();
};
//TAG: TYPE/ALIAS, NAME: nameNewType, TYPE: what is type was copied?
std::string compiler::TypeAlias::print ( unsigned int deep ) {
  std::ostringstream sstream;

  sstream << this->name << "="
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
