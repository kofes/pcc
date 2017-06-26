#include "../inc/Sym.hpp"

std::string compiler::SymVar::print ( unsigned int deep ) {
  std::ostringstream sstream;

  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << this->name << ':'
          << static_cast<char>(this->glob) << ':';
  if (type == nullptr)
      sstream << "<untyped>";
  else
      sstream << this->type->print(0);
  if (this->value.length())
    sstream << ':' << this->value;

  return sstream.str();
};

std::string compiler::SymFunc::print ( unsigned int deep ) {
  std::ostringstream sstream;

  if (retType != nullptr) {
      sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
              << "<function><name>" << this->name << "</name>\n";
      sstream << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
              << "<return type>" << retType->print(0) << "</return type>\n";
  } else {
      sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
              << "<procedure><name>" << this->name << "</name>\n";
  }
  if (params != nullptr) {
      sstream << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
              << "<params>\n";
      for (const std::pair< std::string, pSymVar >& elem : *(params.get()))
        sstream << std::string((deep+2)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
                << "<param>" << elem.second->print(0) << "</param>\n";
      sstream << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
              << "</params>\n";
  }
  if (varTable.size()) {
      sstream << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
              << "<variables>\n";
      for (const std::pair< std::string, pSymVar >& elem : varTable)
        sstream << elem.second->print(deep+2) << '\n';
      sstream << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
              << "</variables>\n";
  }
  sstream << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<body>\n";
  body->print(deep+2);
  sstream << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "</body>\n";
  if (retType != nullptr) {
      sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
              << "</function>";
  } else {
      sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
              << "</procedure>";
  }
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
    sstream << " = (" << values.front()->value;
    for (unsigned long long i = low+1; i <= high; ++i)
      sstream << ", " << values[i-low]->value;
    sstream << ')';
  }

  return sstream.str();
};

std::string compiler::TypeRecord::print ( unsigned int deep ) {
  std::ostringstream sstream;

  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<record>\n";

  for (const std::pair< std::string, pSymVar >& elem : *field)
    sstream << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
            << "<field>" << elem.second->print(0) << "</field>\n";

  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "</record>";

  return sstream.str();
};

void compiler::TypeRecord::checkIdent ( const Lexeme& lexeme ) {
  if (field == nullptr)
    throw ExprException("Record " + name + " hasn't been initialized;");
  if (field->find(lexeme.name) != field->end())
    throw ExprException("Duplicate identifier \"" + lexeme.name + "\" in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + ");");
};

std::string compiler::TypePointer::print ( unsigned int deep ) {
  std::ostringstream sstream;

  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR);
  if (elemType != nullptr)
    sstream << "^(" << elemType->print(0) << ')';
  else
    sstream << "^()";
  
  return sstream.str();
};
//TAG: TYPE/ALIAS, NAME: nameNewType, TYPE: what is type was copied?
std::string compiler::TypeAlias::print ( unsigned int deep ) {
  std::ostringstream sstream;

  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << this->name << "=";
  if (this->type->symType == SymEnum::Record)
    sstream << '\n' << type->print(deep+1);
  else
    sstream << type->print(0);

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
