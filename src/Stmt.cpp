#include "../inc/Stmt.hpp"

std::string compiler::StmtIf::print ( unsigned int deep ) {
  std::ostringstream sstream;

  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<if statement>:\n"
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<condition>:\n"
          << condition->print(deep+2) << "\n"
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<if body>:\n"
          << ifBody->print(deep+2);

  if (elseBody != nullptr)
    sstream << '\n' << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
            << "<else body>:\n"
            << elseBody->print(deep+2);

  return sstream.str();
};

std::string compiler::StmtWhile::print ( unsigned int deep ) {
  std::ostringstream sstream;

  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<while statement>:\n"
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<condition>:\n"
          << condition->print(deep+2) << "\n"
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<body>:\n"
          << body->print(deep+2);

  return sstream.str();
};

std::string compiler::StmtRepeat::print ( unsigned int deep ) {
  std::ostringstream sstream;

  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<repeat statement>:\n"
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<body>:\n";
  for (pNode& elem : body)
    sstream << elem->print(deep+2) << "\n"
            << std::string((deep+2)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
            << "------------------\n";
  sstream << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<until>:\n"
          << condition->print(deep+2);

  return sstream.str();
};

void compiler::StmtRepeat::add ( const pNode& nd ) {
  body.push_back(nd);
};

std::string compiler::StmtFor::print ( unsigned int deep ) {
  std::ostringstream sstream;

  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<for statement>:\n"
          //
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<control variable>:\n"
          << std::string((deep+2)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << variableName.name << '\n'
          //
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<type>:\n"
          << std::string((deep+2)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << compiler::tagBook.at(type) << '\n'
          //
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<initial value>:\n"
          << initVal->print(deep+2) << '\n'
          //
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<final value>:\n"
          << finalVal->print(deep+2) << '\n'
          //
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<body>:\n"
          << body->print(deep+2);

  return sstream.str();
};

std::string compiler::StmtAssignment::print ( unsigned int deep) {
  std::ostringstream sstream;

  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<assignment statement>:\n"
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<variable>:\n"
          << variable->print(deep+2) << "\n"
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<value>:\n"
          << value->print(deep+2);

  return sstream.str();
};

std::string compiler::StmtProcedure::print ( unsigned int deep ) {
  std::ostringstream sstream;

  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<procedure calling>:\n"
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<value>:\n"
          << value->print(deep+2);

  return sstream.str();
};

std::string compiler::StmtBlock::print ( unsigned int deep ) {
  std::ostringstream sstream;

  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<block statement>:\n";
  for (pNode& elem : node)
    sstream << elem->print(deep+1) << "\n"
            << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
            << "------------------\n";
  sstream << "------------------\n";
  return sstream.str();
};

void compiler::StmtBlock::add ( const pNode& nd ) {
  node.push_back(nd);
};
