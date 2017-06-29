#include "../inc/Parser.hpp"

std::string compiler::SymVar::print ( unsigned int deep ) {
  std::ostringstream sstream;

  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << name << ':'
          << static_cast<char>(this->glob) << ':';
  if (type == nullptr)
      sstream << "<untyped>";
  else {
    if (type->symType == SymEnum::Record)
      sstream << '\n' << type->print(deep+1);
    else
      sstream << type->print(0);
  }
  if (value.length())
    sstream << ':' << value;

  return sstream.str();
}

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
}

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
}

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
}

void compiler::TypeRecord::checkIdent ( const Lexeme& lexeme ) {
  if (field == nullptr)
    throw ExprException("Record " + name + " hasn't been initialized;");
  if (field->find(lexeme.name) != field->end())
    throw ExprException("Duplicate identifier \"" + lexeme.name + "\" in pos (" + std::to_string(lexeme.row) + ", " + std::to_string(lexeme.column) + ");");
}

std::string compiler::TypePointer::print ( unsigned int deep ) {
  std::ostringstream sstream;

  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR);
  if (elemType != nullptr)
    sstream << "^(" << elemType->print(0) << ')';
  else
    sstream << "^()";

  return sstream.str();
}
//TAG: TYPE/ALIAS, NAME: nameNewType, TYPE: what is type was copied?
std::string compiler::TypeAlias::print ( unsigned int deep ) {
  std::ostringstream sstream;

  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << name << "=";
  if (type->symType == SymEnum::Record)
    sstream << '\n' << type->print(deep+1);
  else
    sstream << type->print(0);

  return sstream.str();

}

std::string compiler::TypeScalar::print ( unsigned int deep ) {
  std::ostringstream sstream;

  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << this->name;

  return sstream.str();
}

compiler::TypeScalar::TypeScalar ( SCALAR_TYPE tp ) {
  this->name = tagBook.at(static_cast<compiler::Tag>(tp));
  this->type = tp;
  this->symType = SymEnum::Scalar;
}

bool compiler::operator== (pSymType type1, pSymType type2) {
  if (type1 == nullptr && type2 == nullptr)
    return true;
  if (type1 == nullptr || type2 == nullptr)
    return false;
  type1 = evalAlias(type1);
  type2 = evalAlias(type2);
  if (type1->symType != type2->symType)
    return false;
  pSymTable left, right;
  SCALAR_TYPE tp1, tp2;
  switch (type1->symType) {
    case (SymEnum::Array):
    return std::dynamic_pointer_cast<TypeArray>(type1)->elemType == std::dynamic_pointer_cast<TypeArray>(type2)->elemType;
    case (SymEnum::Pointer):
    return std::dynamic_pointer_cast<TypePointer>(type1)->elemType == std::dynamic_pointer_cast<TypePointer>(type2)->elemType;
    case (SymEnum::Record):
    return std::dynamic_pointer_cast<TypeRecord>(type1)->name == std::dynamic_pointer_cast<TypeRecord>(type2)->name;
    case (SymEnum::Scalar):
      tp1 = std::dynamic_pointer_cast<TypeScalar>(type1)->type;
      tp2 = std::dynamic_pointer_cast<TypeScalar>(type2)->type;
      if (((tp1 == SCALAR_TYPE::INTEGER || tp1 == SCALAR_TYPE::REAL) &&
          (tp2 == SCALAR_TYPE::INTEGER || tp2 == SCALAR_TYPE::REAL)) ||
          tp1 == tp2)
        return true;
    return false;
    default: return false;
  }
}

bool compiler::operator!= (pSymType type1, pSymType type2) {
  return !(type1 == type2);
}
// void compiler::SymFunc::generate(Generator& asmGenerator) {
//   for (auto elem : varTable)
//     elem.second->generate(asmGenerator);
//   asmGenerator.addLabel(name + std::to_string(level));
//   asmGenerator.addCmd(to_cmd(OperationEnum::push_, RegisterEnum::rbp_));
//   asmGenerator.addCmd(to_cmd(OperationEnum::mov_, RegisterEnum::rbp_, RegisterEnum::rsp_));
//   asmGenerator.addCmd(to_cmd(OperationEnum::sub_, RegisterEnum::rsp_, size_table));
//   body->generate(asmGenerator);
//   asmGenerator.addCmd(to_cmd(OperationEnum::mov_, RegisterEnum::rsp_, RegisterEnum::rbp_));
//   asmGenerator.addCmd(to_cmd(OperationEnum::pop_, RegisterEnum::rbp_));
//   asmGenerator.addCmd(to_cmd(OperationEnum::ret_));
// }
