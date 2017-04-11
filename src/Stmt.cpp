#include "../inc/Stmt.hpp"

std::string compiler::StmtIf::print ( unsigned int deep ) {
  std::ostringstream sstream;

    sstream << condition->print(deep+1);
  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR) << this->name << std::endl;
    sstream << body->print(deep+1);

  return sstream.str();
};

std::string compiler::StmtWhile::print ( unsigned int deep ) {
  std::ostringstream sstream;

    sstream << condition->print(deep+1);
  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR) << this->name << std::endl;
    sstream << body->print(deep+1);

  return sstream.str();
};

std::string compiler::StmtRepeat::print ( unsigned int deep ) {
  std::ostringstream sstream;

    sstream << condition->print(deep+1);
  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR) << this->name << std::endl;
    sstream << body->print(deep+1);

  return sstream.str();
};

std::string compiler::StmtFor::print ( unsigned int deep ) {
  std::ostringstream sstream;

    sstream << var->print(deep+1);
    sstream << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR) << initVal.name;
    sstream << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR) << finalVal.name;
    sstream << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR) << compiler::tagBook.at(type);
  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR) << this->name << std::endl;
    sstream << body->print(deep+1);

  return sstream.str();
};
