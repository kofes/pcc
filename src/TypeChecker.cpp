#include "../inc/Parser.hpp"

compiler::Tag compiler::Parser::checkType ( Tag operation, Tag left_operand, Tag right_operand ) {
  switch (operation) {
    case (Tag::EQUALS):
    case (Tag::MIS):
    case (Tag::GREATER):
    case (Tag::LESS):
    case (Tag::GREATER_EQ):
    case (Tag::LESS_EQ):
      if (left_operand == right_operand &&
          (left_operand == Tag::INTEGER || left_operand == Tag::REAL ||
          left_operand == Tag::POINTER || left_operand == Tag::CHAR ||
          left_operand == Tag::ARRAY))
        return Tag::BOOLEAN;
    break;
    case (Tag::ADD):
    case (Tag::SUB):
      if ((left_operand == Tag::INTEGER || left_operand == Tag::FLOAT) &&
          right_operand == Tag::UNDEFINED)
        return left_operand;
    case (Tag::MUL):
      if (left_operand == Tag::INTEGER && right_operand == Tag::INTEGER)
        return Tag::INTEGER;
    case (Tag::DIV_FLOAT):
      if (left_operand != Tag::FLOAT && left_operand != Tag::INTEGER &&
         right_operand != Tag::FLOAT && right_operand != Tag::INTEGER)
        break;
      if (left_operand == Tag::FLOAT || right_operand == Tag::FLOAT)
        return Tag::FLOAT;
    break;
    case (Tag::DIV_INT):
    case (Tag::MOD):
    case (Tag::SHL):
    case (Tag::SHR):
      if (left_operand == Tag::INTEGER && right_operand == Tag::INTEGER)
        return Tag::INTEGER;
    break;
    case (Tag::NOT):
      if (left_operand == Tag::INTEGER)
        return Tag::INTEGER;
      if (left_operand == Tag::BOOLEAN)
        return Tag::BOOLEAN;
    break;
    case (Tag::OR):
    case (Tag::XOR):
    case (Tag::AND):
      if (left_operand == Tag::INTEGER && right_operand == Tag::INTEGER)
        return Tag::INTEGER;
      if (left_operand == Tag::BOOLEAN && right_operand == Tag::BOOLEAN)
        return Tag::BOOLEAN;
    break;
    case (Tag::ADDRESS):
      if (left_operand == Tag::IDENTIFIER)
        return Tag::POINTER;
    break;
    case (Tag::POINTER):
      if (left_operand == Tag::IDENTIFIER)
        return Tag::TYPE;
    break;
    case (Tag::IN):
      if (left_operand == Tag::INTEGER && right_operand == Tag::SET)
        return Tag::BOOLEAN;
    break;
    default: break;
  };
  return Tag::UNDEFINED;
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
// {Tag::SHL, compiler::Priority::SECOND},
// {Tag::SHR, compiler::Priority::SECOND}
// {Tag::AND, compiler::Priority::SECOND},
//
// {Tag::NOT, compiler::Priority::HIGHEST},
// {Tag::SUB, compiler::Priority::HIGHEST},
// {Tag::ADD, compiler::Priority::HIGHEST},
// {Tag::ADDRESS, compiler::Priority::HIGHEST}
