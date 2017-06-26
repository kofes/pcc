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
          << "<while statement>\n"
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<condition>\n"
          << condition->print(deep+2) << "\n"
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "</condition>\n"
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<body>\n"
          << body->print(deep+2)
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "</body>\n"
          << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "</while statement>";


  return sstream.str();
};

std::string compiler::StmtRepeat::print ( unsigned int deep ) {
  std::ostringstream sstream;

  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<repeat statement>\n";
  for (pNode& elem : body)
    sstream << elem->print(deep+2) << "\n";
  sstream << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<until>\n"
          << condition->print(deep+2)
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "</until>\n"
          << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "</repeat statement>";

  return sstream.str();
};

void compiler::StmtRepeat::add ( const pNode& nd ) {
  body.push_back(nd);
};

std::string compiler::StmtFor::print ( unsigned int deep ) {
  std::ostringstream sstream;

  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<for statement>\n"
          //
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<control variable>:\n"
          << std::string((deep+2)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << variableName.name << '\n'
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "</control variable>:\n"
          //
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<type>\n"
          << std::string((deep+2)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << compiler::tagBook.at(type) << '\n'
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "</type>\n"
          //
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<initial value>:\n"
          << initVal->print(deep+2) << '\n'
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "</initial value>:\n"
          //
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<final value>\n"
          << finalVal->print(deep+2) << '\n'
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "</final value>\n"
          //
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<body>\n"
          << body->print(deep+2)
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "</body>\n"
          //
          << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "</for statement>";

  return sstream.str();
};

std::string compiler::StmtAssignment::print ( unsigned int deep) {
  std::ostringstream sstream;

  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<assignment statement>\n"
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<variable>\n"
          << variable->print(deep+2) << "\n"
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "</variable>\n"
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<value>\n"
          << value->print(deep+2) << '\n'
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "</value>\n"
          << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "</assignment statement>";

  return sstream.str();
};

std::string compiler::StmtProcedure::print ( unsigned int deep ) {
  std::ostringstream sstream;

  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<procedure calling>\n"
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<value>\n"
          << value->print(deep+2) << '\n'
          << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "</value>\n"
          << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "</procedure calling>";

  return sstream.str();
};

std::string compiler::StmtBreak::print ( unsigned int deep ) {
  std::ostringstream sstream;
  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<break stmt/>";
  return sstream.str();
};

std::string compiler::StmtContinue::print ( unsigned int deep ) {
  std::ostringstream sstream;
  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<continue stmt/>";
  return sstream.str();
};

std::string compiler::StmtBlock::print ( unsigned int deep ) {
  std::ostringstream sstream;

  sstream << std::string(deep*compiler::DEEP_STEP, compiler::DEEP_CHAR)
          << "<block statement>\n";
  for (pNode& elem : node)
    sstream << elem->print(deep+1) << "\n";
            // << std::string((deep+1)*compiler::DEEP_STEP, compiler::DEEP_CHAR)
            // << "------------------\n";
  sstream << "</block statement>\n";
  return sstream.str();
};

void compiler::StmtBlock::add ( const pNode& nd ) {
  node.push_back(nd);
};
