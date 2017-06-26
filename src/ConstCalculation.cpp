#include "../inc/Parser.hpp"

//nil, true, false == identifiers!

void doUnOp ( compiler::Tag tag, compiler::pExpr& value ) {
  switch (tag) {
    case (compiler::Tag::ADD): break;
    case (compiler::Tag::SUB):
      switch (value->tag){
        case (compiler::Tag::INTEGER):
          value->name = std::to_string(-std::stoll(value->name));
        break;
        case (compiler::Tag::FLOAT):
          value->name = std::to_string(-std::stod(value->name));
        break;
        case (compiler::Tag::POINTER):
          throw compiler::ExprException("sub with pointer at pos(" + std::to_string(value->row) + ", " + std::to_string(value->column) + ");");
        break;
        case (compiler::Tag::B_FALSE): break;
        case (compiler::Tag::B_TRUE): break;
        default:
          throw compiler::ExprException("sub with `bad value` at pos(" + std::to_string(value->row) + ", " + std::to_string(value->column) + ");");
        break;
      }
    break;
    case (compiler::Tag::NOT):
      switch (value->tag) {
        case (compiler::Tag::INTEGER):
          value->name = std::to_string(~std::stoll(value->name));
        break;
        case (compiler::Tag::FLOAT):
          throw compiler::ExprException("not with float at pos(" + std::to_string(value->row) + ", " + std::to_string(value->column) + ");");
        break;
        case (compiler::Tag::POINTER):
          throw compiler::ExprException("not with pointer at pos(" + std::to_string(value->row) + ", " + std::to_string(value->column) + ");");
        break;
        case (compiler::Tag::B_FALSE):
          value->tag = compiler::Tag::B_TRUE;
          value->name = "true";
        break;
        case (compiler::Tag::B_TRUE):
          value->tag = compiler::Tag::B_FALSE;
          value->name = "false";
        break;
        default:
          throw compiler::ExprException("not with `bad value` at pos(" + std::to_string(value->row) + ", " + std::to_string(value->column) + ");");
        break;
      }
    break;
    default:
      throw compiler::ExprException("unary with `bad value` operation at pos(" + std::to_string(value->row) + ", " + std::to_string(value->column) + ");");
    break;
  }
};

compiler::pExpr doBinOp ( compiler::Tag oper, const compiler::pExpr& left, const compiler::pExpr& right) {
  compiler::pExpr res;
  switch (oper) {
    case (compiler::Tag::ADD):
      if (left->tag == compiler::Tag::FLOAT || right->tag == compiler::Tag::FLOAT) {
        res = compiler::pExpr(new compiler::ExprReal(*left));
        res->tag = compiler::Tag::FLOAT;
        res->name = std::to_string(std::stod(left->name)+std::stod(right->name));
      }
    break;
    case (compiler::Tag::SUB):
    break;
    case (compiler::Tag::MUL):
    break;
    case (compiler::Tag::DIV_FLOAT):
    break;
    //int operations
    case (compiler::Tag::DIV_INT):
    break;
    case (compiler::Tag::MOD):
    break;
    case (compiler::Tag::SHL):
    break;
    case (compiler::Tag::SHR):
    break;
    default:
      throw compiler::ExprException("wrong binary operation");
  }
  return res;
};
// {Tag::EQUALS, compiler::Priority::LOWEST},
// {Tag::MIS, compiler::Priority::LOWEST},
// {Tag::GREATER, compiler::Priority::LOWEST},
// {Tag::LESS, compiler::Priority::LOWEST},
// {Tag::GREATER_EQ, compiler::Priority::LOWEST},
// {Tag::LESS_EQ, compiler::Priority::LOWEST},
// {Tag::IN, compiler::Priority::LOWEST},
//
// {Tag::ADD, compiler::Priority::THIRD},
// {Tag::SUB, compiler::Priority::THIRD},
// {Tag::OR, compiler::Priority::THIRD},
// {Tag::XOR, compiler::Priority::THIRD},
//
// {Tag::MUL, compiler::Priority::SECOND},
// {Tag::DIV_FLOAT, compiler::Priority::SECOND},
// {Tag::DIV_INT, compiler::Priority::SECOND},
// {Tag::MOD, compiler::Priority::SECOND},
// {Tag::AND, compiler::Priority::SECOND},
// {Tag::SHL, compiler::Priority::SECOND},
// {Tag::SHR, compiler::Priority::SECOND}
//
// {Tag::NOT, compiler::Priority::HIGHEST},
// {Tag::SUB, compiler::Priority::HIGHEST},
// {Tag::ADD, compiler::Priority::HIGHEST},
// {Tag::ADDRESS, compiler::Priority::HIGHEST}
#include <iostream>
compiler::pExpr compiler::Parser::evalConstExpr ( pExpr& root, SymTable& vTable, TypeTable& tTable ) {
    pExpr res;
    pSymVar var;
    pSymType curr_type;
    auto iter = vTable.find(root->name);
    switch (root->exprType) {
        case (ExprEnum::UnOp):
          res = evalConstExpr(std::dynamic_pointer_cast<ExprUnOp>(root)->arg, vTable, tTable);
          doUnOp(root->tag, res);
        break;
        case (ExprEnum::BinOp):
          return doBinOp(root->tag,
                        evalConstExpr(std::dynamic_pointer_cast<ExprBinOp>(root)->left, vTable, tTable),
                        evalConstExpr(std::dynamic_pointer_cast<ExprBinOp>(root)->right, vTable, tTable));
        break;
        case (ExprEnum::Real):
        case (ExprEnum::Integer):
        case (ExprEnum::Char):
        case (ExprEnum::String):
          return root;
        case (ExprEnum::Identifier):
          if (root->tag == Tag::NIL || root->tag == Tag::B_FALSE || root->tag == Tag::B_TRUE)
            return root;
          //Now must be only const variable!
          if (iter == vTable.end())
            throw ExprException("Variable \"" + root->print(0) + "\" from pos("+ std::to_string(root->row) + ", " + std::to_string(root->column) + ") wasn't declared;");
          if (iter->second->glob != GLOB::CONST)
            throw ExprException("Variable \"" + root->print(0) + "\" from pos("+ std::to_string(root->row) + ", " + std::to_string(root->column) + ") isn't const;");
          return root;
        break;
        case (ExprEnum::Function):
          if (ctFuncTable.find(root->name) == ctFuncTable.end())
            throw ExprException("Function \"" + root->print(0) + "\" from pos("+ std::to_string(root->row) + ", " + std::to_string(root->column) + ") isn't compile-time function;");
        break;
        default: break;
    }
    throw ExprException("wrong symbol \"" + root->name + "\" from pos("+ std::to_string(root->row) + ", " + std::to_string(root->column) + ");");
};
