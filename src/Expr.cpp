#include "../inc/Expr.hpp"

std::string compiler::Expr::print ( unsigned int deep ) {
  std::ostringstream sstream;
  sstream << std::string(deep*3, ' ') << this->name << std::endl;
  return sstream.str();
};

std::string compiler::ExprBinOp::print ( unsigned int deep ) {
  std::ostringstream sstream;
  sstream << right->print(deep+1);
  sstream << std::string(deep*3, ' ') << this->name << std::endl;
  sstream << left->print(deep+1);
  return sstream.str();
};

std::string compiler::ExprUnOp::print ( unsigned int deep ) {
  std::ostringstream sstream;
  sstream << this->arg->print(deep+1);
  sstream << std::string(deep*3, ' ') << this->name << std::endl;
  return sstream.str();
};

std::string compiler::ExprArrayIndex::print ( unsigned int deep ) {
  std::ostringstream sstream;
  for (pExpr expr : args)
    sstream << expr->print(deep+1);
  sstream << std::string(deep*3, ' ') << this->name << std::endl;
  sstream << this->left->print(deep+1);
  return sstream.str();
};
