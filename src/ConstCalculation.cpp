#include "../inc/Parser.hpp"

//nil, true, false == identifiers!
#include <iostream>

compiler::pExpr doUnOp ( std::shared_ptr<compiler::ExprUnOp> oper, compiler::pExpr value ) {
  switch (oper->tag) {
    case (compiler::Tag::ADD):
      if (value->exprType != compiler::ExprEnum::Real && value->exprType != compiler::ExprEnum::Integer)
        throw compiler::ExprException("Incompatible unary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
    break;
    case (compiler::Tag::SUB):
      switch (value->exprType){
        case (compiler::ExprEnum::Real):
          value->name = std::to_string(-std::stoll(value->name));
        break;
        case (compiler::ExprEnum::Integer):
          value->name = std::to_string(-std::stod(value->name));
        break;
        default:
          throw compiler::ExprException("Incompatible unary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
      }
    break;
    case (compiler::Tag::NOT):
      switch (value->tag) {
        case (compiler::Tag::INTEGER):
          value->name = std::to_string(~std::stoll(value->name));
        break;
        case (compiler::Tag::B_FALSE):
          value->tag = compiler::Tag::B_TRUE;
          value->name = "TRUE";
        break;
        case (compiler::Tag::B_TRUE):
          value->tag = compiler::Tag::B_FALSE;
          value->name = "FALSE";
        break;
        default:
          throw compiler::ExprException("Incompatible unary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
        break;
      }
    break;
    default:
      throw compiler::ExprException("Unary with `bad value` operation at pos (" + std::to_string(value->row) + ", " + std::to_string(value->column) + ");");
    break;
  }
  return value;
};

compiler::pExpr doBinOp ( std::shared_ptr<compiler::ExprBinOp> oper,
  const compiler::pExpr& left, const compiler::pExpr& right) {
  compiler::pExpr res;
  switch (oper->tag) {
    case (compiler::Tag::ADD):
      if ((left->exprType != compiler::ExprEnum::Real && left->exprType != compiler::ExprEnum::Integer) ||
          (right->exprType != compiler::ExprEnum::Real && right->exprType != compiler::ExprEnum::Integer))
        throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
      if (left->exprType == compiler::ExprEnum::Real || right->exprType == compiler::ExprEnum::Real) {
        res = compiler::pExpr(new compiler::ExprReal(*left));
        res->tag = compiler::Tag::FLOAT;
        res->name = std::to_string(std::stod(left->name)+std::stod(right->name));
        return res;
      } else {
        res = compiler::pExpr(new compiler::ExprInteger(*left));
        res->tag = compiler::Tag::INTEGER;
        res->name = std::to_string(std::stoll(left->name)+std::stoll(right->name));
        return res;
      }
    break;
    case (compiler::Tag::SUB):
      if ((left->exprType != compiler::ExprEnum::Real && left->exprType != compiler::ExprEnum::Integer) ||
          (right->exprType != compiler::ExprEnum::Real && right->exprType != compiler::ExprEnum::Integer))
        throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
      if (left->exprType == compiler::ExprEnum::Real || right->exprType == compiler::ExprEnum::Real) {
        res = compiler::pExpr(new compiler::ExprReal(*left));
        res->tag = compiler::Tag::FLOAT;
        res->name = std::to_string(std::stod(left->name)-std::stod(right->name));
      } else {
        res = compiler::pExpr(new compiler::ExprInteger(*left));
        res->tag = compiler::Tag::INTEGER;
        res->name = std::to_string(std::stoll(left->name)-std::stoll(right->name));
      }
    break;
    case (compiler::Tag::MUL):
      if ((left->exprType != compiler::ExprEnum::Real && left->exprType != compiler::ExprEnum::Integer) ||
          (right->exprType != compiler::ExprEnum::Real && right->exprType != compiler::ExprEnum::Integer))
        throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
      if (left->exprType == compiler::ExprEnum::Real || right->exprType == compiler::ExprEnum::Real) {
        res = compiler::pExpr(new compiler::ExprReal(*left));
        res->tag = compiler::Tag::FLOAT;
        res->exprType = compiler::ExprEnum::Real;
        res->name = std::to_string(std::stod(left->name)*std::stod(right->name));
      } else {
        res = compiler::pExpr(new compiler::ExprInteger(*left));
        res->tag = compiler::Tag::INTEGER;
        res->exprType = compiler::ExprEnum::Integer;
        res->name = std::to_string(std::stoll(left->name)*std::stoll(right->name));
      }
    break;
    case (compiler::Tag::DIV_FLOAT):
      if ((left->exprType != compiler::ExprEnum::Real && left->exprType != compiler::ExprEnum::Integer) ||
          (right->exprType != compiler::ExprEnum::Real && right->exprType != compiler::ExprEnum::Integer))
        throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
      res = compiler::pExpr(new compiler::ExprReal(*left));
      res->tag = compiler::Tag::FLOAT;
      res->exprType = compiler::ExprEnum::Real;
      res->name = std::to_string(std::stod(left->name)/std::stod(right->name));
    break;
    case (compiler::Tag::DIV_INT):
      if (left->exprType != compiler::ExprEnum::Integer || right->exprType != compiler::ExprEnum::Integer)
        throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
      res = compiler::pExpr(new compiler::ExprInteger(*left));
      res->tag = compiler::Tag::INTEGER;
      if (right->name == "0")
        throw compiler::ExprException("DIV0 at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
      res->name = std::to_string(std::stoll(left->name)/std::stoll(right->name));
    break;
    case (compiler::Tag::MOD):
      if (left->exprType != compiler::ExprEnum::Integer || right->exprType != compiler::ExprEnum::Integer)
        throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
      res = compiler::pExpr(new compiler::ExprInteger(*left));
      res->tag = compiler::Tag::INTEGER;
      if (right->name == "0")
        throw compiler::ExprException("MOD0 at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
      res->name = std::to_string(std::stoll(left->name)%std::stoll(right->name));
    break;
    case (compiler::Tag::SHL):
      if (left->exprType != compiler::ExprEnum::Integer || right->exprType != compiler::ExprEnum::Integer)
        throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
      res = compiler::pExpr(new compiler::ExprInteger(*left));
      res->tag = compiler::Tag::INTEGER;
      res->name = std::to_string(std::stoll(left->name)<<std::stoll(right->name));
    break;
    case (compiler::Tag::SHR):
      if (left->exprType != compiler::ExprEnum::Integer || right->exprType != compiler::ExprEnum::Integer)
        throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
      res = compiler::pExpr(new compiler::ExprInteger(*left));
      res->tag = compiler::Tag::INTEGER;
      res->name = std::to_string(std::stoll(left->name)>>std::stoll(right->name));
    break;
    case (compiler::Tag::EQUALS):
      switch (left->tag) {
        case (compiler::Tag::INTEGER):
          if (right->exprType != compiler::ExprEnum::Integer && right->exprType != compiler::ExprEnum::Real)
            throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
          res = compiler::pExpr(new compiler::ExprIdentifier(*left));
          res->exprType = compiler::ExprEnum::Identifier;
          if (right->exprType == compiler::ExprEnum::Integer) {
            if (std::stoll(left->name) == std::stoll(right->name)) {
              res->tag = compiler::Tag::B_TRUE;
              res->name = "TRUE";
            } else {
              res->tag = compiler::Tag::B_FALSE;
              res->name = "FALSE";
            }
          } else {
            if (std::stod(left->name) == std::stod(right->name)) {
              res->tag = compiler::Tag::B_TRUE;
              res->name = "TRUE";
            } else {
              res->tag = compiler::Tag::B_FALSE;
              res->name = "FALSE";
            }
          }
        break;
        case (compiler::Tag::FLOAT):
          if (right->exprType != compiler::ExprEnum::Integer && right->exprType != compiler::ExprEnum::Real)
            throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
          res = compiler::pExpr(new compiler::ExprIdentifier(*left));
          res->exprType = compiler::ExprEnum::Identifier;
          if (std::stod(left->name) == std::stod(right->name)) {
            res->tag = compiler::Tag::B_TRUE;
            res->name = "TRUE";
          } else {
            res->tag = compiler::Tag::B_FALSE;
            res->name = "FALSE";
          }
        break;
        case (compiler::Tag::B_FALSE):
        case (compiler::Tag::B_TRUE):
          if (right->tag != compiler::Tag::B_FALSE && right->tag != compiler::Tag::B_TRUE)
            throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
          res = compiler::pExpr(new compiler::ExprIdentifier(*left));
          res->exprType = compiler::ExprEnum::Identifier;
          if (left->tag == right->tag) {
            res->tag = compiler::Tag::B_TRUE;
            res->name = "TRUE";
          } else {
            res->tag = compiler::Tag::B_FALSE;
            res->name = "FALSE";
          }
        break;
        case (compiler::Tag::NIL):
          if (right->tag != compiler::Tag::NIL)
            throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
          res = compiler::pExpr(new compiler::ExprIdentifier(*left));
          res->exprType = compiler::ExprEnum::Identifier;
          res->tag = compiler::Tag::B_TRUE;
          res->name = "TRUE";
        break;
        case (compiler::Tag::CHARACTER):
          if (right->exprType != compiler::ExprEnum::Char)
            throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
          res = compiler::pExpr(new compiler::ExprIdentifier(*left));
          res->exprType = compiler::ExprEnum::Identifier;
          if (left->name == right->name) {
            res->tag = compiler::Tag::B_TRUE;
            res->name = "TRUE";
          } else {
            res->tag = compiler::Tag::B_FALSE;
            res->name = "FALSE";
          }
        break;
        default:
          throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
        break;
      }
    break;
    case (compiler::Tag::MIS):
      switch (left->tag) {
        case (compiler::Tag::INTEGER):
          if (right->exprType != compiler::ExprEnum::Integer && right->exprType != compiler::ExprEnum::Real)
            throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
          res = compiler::pExpr(new compiler::ExprIdentifier(*left));
          res->exprType = compiler::ExprEnum::Identifier;
          if (right->exprType != compiler::ExprEnum::Integer) {
            if (std::stoll(left->name) == std::stoll(right->name)) {
              res->tag = compiler::Tag::B_TRUE;
              res->name = "TRUE";
            } else {
              res->tag = compiler::Tag::B_FALSE;
              res->name = "FALSE";
            }
          } else {
            if (std::stod(left->name) != std::stod(right->name)) {
              res->tag = compiler::Tag::B_TRUE;
              res->name = "TRUE";
            } else {
              res->tag = compiler::Tag::B_FALSE;
              res->name = "FALSE";
            }
          }
        break;
        case (compiler::Tag::FLOAT):
          if (right->exprType != compiler::ExprEnum::Integer && right->exprType != compiler::ExprEnum::Real)
            throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
          res = compiler::pExpr(new compiler::ExprIdentifier(*left));
          res->exprType = compiler::ExprEnum::Identifier;
          if (std::stod(left->name) != std::stod(right->name)) {
            res->tag = compiler::Tag::B_TRUE;
            res->name = "TRUE";
          } else {
            res->tag = compiler::Tag::B_FALSE;
            res->name = "FALSE";
          }
        break;
        case (compiler::Tag::B_FALSE):
        case (compiler::Tag::B_TRUE):
          if (right->tag != compiler::Tag::B_FALSE && right->tag != compiler::Tag::B_TRUE)
            throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
          res = compiler::pExpr(new compiler::ExprIdentifier(*left));
          res->exprType = compiler::ExprEnum::Identifier;
          if (left->tag != right->tag) {
            res->tag = compiler::Tag::B_TRUE;
            res->name = "TRUE";
          } else {
            res->tag = compiler::Tag::B_FALSE;
            res->name = "FALSE";
          }
        break;
        case (compiler::Tag::NIL):
          if (right->tag != compiler::Tag::NIL)
            throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
          res = compiler::pExpr(new compiler::ExprIdentifier(*left));
          res->exprType = compiler::ExprEnum::Identifier;
          res->tag = compiler::Tag::B_FALSE;
          res->name = "FALSE";
        break;
        default:
          throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
        break;
      }
    break;
    case (compiler::Tag::GREATER):
      switch (left->tag) {
        case (compiler::Tag::INTEGER):
          if (right->exprType != compiler::ExprEnum::Integer && right->exprType != compiler::ExprEnum::Real)
            throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
          res = compiler::pExpr(new compiler::ExprIdentifier(*left));
          res->exprType = compiler::ExprEnum::Identifier;
          if (right->exprType != compiler::ExprEnum::Integer) {
            if (std::stoll(left->name) > std::stoll(right->name)) {
              res->tag = compiler::Tag::B_TRUE;
              res->name = "TRUE";
            } else {
              res->tag = compiler::Tag::B_FALSE;
              res->name = "FALSE";
            }
          } else {
            if (std::stod(left->name) > std::stod(right->name)) {
              res->tag = compiler::Tag::B_TRUE;
              res->name = "TRUE";
            } else {
              res->tag = compiler::Tag::B_FALSE;
              res->name = "FALSE";
            }
          }
        break;
        case (compiler::Tag::FLOAT):
          if (right->exprType != compiler::ExprEnum::Integer && right->exprType != compiler::ExprEnum::Real)
            throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
          res = compiler::pExpr(new compiler::ExprIdentifier(*left));
          res->exprType = compiler::ExprEnum::Identifier;
          if (std::stod(left->name) > std::stod(right->name)) {
            res->tag = compiler::Tag::B_TRUE;
            res->name = "TRUE";
          } else {
            res->tag = compiler::Tag::B_FALSE;
            res->name = "FALSE";
          }
        break;
        default:
          throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
        break;
      }
    break;
    case (compiler::Tag::LESS):
      switch (left->tag) {
        case (compiler::Tag::INTEGER):
          if (right->exprType != compiler::ExprEnum::Integer && right->exprType != compiler::ExprEnum::Real)
            throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
          res = compiler::pExpr(new compiler::ExprIdentifier(*left));
          res->exprType = compiler::ExprEnum::Identifier;
          if (right->exprType != compiler::ExprEnum::Integer) {
            if (std::stoll(left->name) < std::stoll(right->name)) {
              res->tag = compiler::Tag::B_TRUE;
              res->name = "TRUE";
            } else {
              res->tag = compiler::Tag::B_FALSE;
              res->name = "FALSE";
            }
          } else {
            if (std::stod(left->name) < std::stod(right->name)) {
              res->tag = compiler::Tag::B_TRUE;
              res->name = "TRUE";
            } else {
              res->tag = compiler::Tag::B_FALSE;
              res->name = "FALSE";
            }
          }
        break;
        case (compiler::Tag::FLOAT):
          if (right->exprType != compiler::ExprEnum::Integer && right->exprType != compiler::ExprEnum::Real)
            throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
          res = compiler::pExpr(new compiler::ExprIdentifier(*left));
          res->exprType = compiler::ExprEnum::Identifier;
          if (std::stod(left->name) < std::stod(right->name)) {
            res->tag = compiler::Tag::B_TRUE;
            res->name = "TRUE";
          } else {
            res->tag = compiler::Tag::B_FALSE;
            res->name = "FALSE";
          }
        break;
        default:
          throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
        break;
      }
    break;
    case (compiler::Tag::GREATER_EQ):
      switch (left->tag) {
        case (compiler::Tag::INTEGER):
          if (right->exprType != compiler::ExprEnum::Integer && right->exprType != compiler::ExprEnum::Real)
            throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
          res = compiler::pExpr(new compiler::ExprIdentifier(*left));
          res->exprType = compiler::ExprEnum::Identifier;
          if (right->exprType != compiler::ExprEnum::Integer) {
            if (std::stoll(left->name) >= std::stoll(right->name)) {
              res->tag = compiler::Tag::B_TRUE;
              res->name = "TRUE";
            } else {
              res->tag = compiler::Tag::B_FALSE;
              res->name = "FALSE";
            }
          } else {
            if (std::stod(left->name) >= std::stod(right->name)) {
              res->tag = compiler::Tag::B_TRUE;
              res->name = "TRUE";
            } else {
              res->tag = compiler::Tag::B_FALSE;
              res->name = "FALSE";
            }
          }
        break;
        case (compiler::Tag::FLOAT):
          if (right->exprType != compiler::ExprEnum::Integer && right->exprType != compiler::ExprEnum::Real)
            throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
          res = compiler::pExpr(new compiler::ExprIdentifier(*left));
          res->exprType = compiler::ExprEnum::Identifier;
          if (std::stod(left->name) >= std::stod(right->name)) {
            res->tag = compiler::Tag::B_TRUE;
            res->name = "TRUE";
          } else {
            res->tag = compiler::Tag::B_FALSE;
            res->name = "FALSE";
          }
        break;
        default:
          throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
        break;
      }
    break;
    case (compiler::Tag::LESS_EQ):
      switch (left->tag) {
        case (compiler::Tag::INTEGER):
          if (right->exprType != compiler::ExprEnum::Integer && right->exprType != compiler::ExprEnum::Real)
            throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
          res = compiler::pExpr(new compiler::ExprIdentifier(*left));
          res->exprType = compiler::ExprEnum::Identifier;
          if (right->exprType != compiler::ExprEnum::Integer) {
            if (std::stoll(left->name) <= std::stoll(right->name)) {
              res->tag = compiler::Tag::B_TRUE;
              res->name = "TRUE";
            } else {
              res->tag = compiler::Tag::B_FALSE;
              res->name = "FALSE";
            }
          } else {
            if (std::stod(left->name) <= std::stod(right->name)) {
              res->tag = compiler::Tag::B_TRUE;
              res->name = "TRUE";
            } else {
              res->tag = compiler::Tag::B_FALSE;
              res->name = "FALSE";
            }
          }
        break;
        case (compiler::Tag::FLOAT):
          if (right->exprType != compiler::ExprEnum::Integer && right->exprType != compiler::ExprEnum::Real)
            throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
          res = compiler::pExpr(new compiler::ExprIdentifier(*left));
          res->exprType = compiler::ExprEnum::Identifier;
          if (std::stod(left->name) <= std::stod(right->name)) {
            res->tag = compiler::Tag::B_TRUE;
            res->name = "TRUE";
          } else {
            res->tag = compiler::Tag::B_FALSE;
            res->name = "FALSE";
          }
        break;
        default:
          throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
        break;
      }
    break;
    case (compiler::Tag::OR):
      switch (left->tag) {
        case (compiler::Tag::INTEGER):
          if (right->exprType != compiler::ExprEnum::Integer)
            throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
          res = compiler::pExpr(new compiler::ExprInteger(*left));
          res->name = std::stoll(left->name) | std::stoll(right->name);
        break;
        case (compiler::Tag::B_FALSE):
        case (compiler::Tag::B_TRUE):
          if (right->tag != compiler::Tag::B_FALSE && right->tag != compiler::Tag::B_TRUE)
            throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
          res = compiler::pExpr(new compiler::ExprIdentifier(*left));
          res->exprType = compiler::ExprEnum::Identifier;
          if (left->tag == compiler::Tag::B_TRUE || right->tag == compiler::Tag::B_TRUE) {
            res->tag = compiler::Tag::B_TRUE;
            res->name = "TRUE";
          } else {
            res->tag = compiler::Tag::B_FALSE;
            res->name = "FALSE";
          }
        break;
        default:
          throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
        break;
      }
    break;
    case (compiler::Tag::XOR):
      switch (left->tag) {
        case (compiler::Tag::INTEGER):
          if (right->exprType != compiler::ExprEnum::Integer)
            throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
          res = compiler::pExpr(new compiler::ExprInteger(*left));
          res->name = std::stoll(left->name) ^ std::stoll(right->name);
        break;
        case (compiler::Tag::B_FALSE):
        case (compiler::Tag::B_TRUE):
          if (right->tag != compiler::Tag::B_FALSE && right->tag != compiler::Tag::B_TRUE)
            throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
          res = compiler::pExpr(new compiler::ExprIdentifier(*left));
          res->exprType = compiler::ExprEnum::Identifier;
          if (left->tag != right->tag) {
            res->tag = compiler::Tag::B_TRUE;
            res->name = "TRUE";
          } else {
            res->tag = compiler::Tag::B_FALSE;
            res->name = "FALSE";
          }
        break;
        default:
          throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
        break;
      }
    break;
    case (compiler::Tag::AND):
      switch (left->tag) {
        case (compiler::Tag::INTEGER):
          if (right->exprType != compiler::ExprEnum::Integer)
            throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
          res = compiler::pExpr(new compiler::ExprInteger(*left));
          res->name = std::stoll(left->name) & std::stoll(right->name);
        break;
        case (compiler::Tag::B_FALSE):
        case (compiler::Tag::B_TRUE):
          if (right->tag != compiler::Tag::B_FALSE && right->tag != compiler::Tag::B_TRUE)
            throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
          res = compiler::pExpr(new compiler::ExprIdentifier(*left));
          res->exprType = compiler::ExprEnum::Identifier;
          if (left->tag == right->tag && right->tag == compiler::Tag::B_TRUE) {
            res->tag = compiler::Tag::B_TRUE;
            res->name = "TRUE";
          } else {
            res->tag = compiler::Tag::B_FALSE;
            res->name = "FALSE";
          }
        break;
        default:
          throw compiler::ExprException("Incompatible binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
        break;
      }
    break;
    default:
      throw compiler::ExprException("Wrong binary operation at pos (" + std::to_string(oper->row) + ", " + std::to_string(oper->column) + ");");
  }
  return res;
};

compiler::pExpr compiler::Parser::evalConstExpr ( pExpr& root, SymTable& vTable, TypeTable& tTable ) {
  pExpr res;
  pSymVar var;
  pSymType curr_type;
  auto iter = vTable.find(root->name);
  switch (root->exprType) {
      case (ExprEnum::UnOp):
        return doUnOp(std::dynamic_pointer_cast<ExprUnOp>(root),
                      evalConstExpr(std::dynamic_pointer_cast<ExprUnOp>(root)->arg, vTable, tTable));
      case (ExprEnum::BinOp):
        return doBinOp(std::dynamic_pointer_cast<ExprBinOp>(root),
                      evalConstExpr(std::dynamic_pointer_cast<ExprBinOp>(root)->left, vTable, tTable),
                      evalConstExpr(std::dynamic_pointer_cast<ExprBinOp>(root)->right, vTable, tTable));
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
          throw ExprException("Variable \"" + root->print(0) + "\" from pos ("+ std::to_string(root->row) + ", " + std::to_string(root->column) + ") wasn't declared;");
        if (iter->second->glob != GLOB::CONST)
          throw ExprException("Variable \"" + root->print(0) + "\" from pos ("+ std::to_string(root->row) + ", " + std::to_string(root->column) + ") isn't const;");
        // iter->second->type == nullptr;
        if (iter->second->type->symType == SymEnum::Pointer) {
          root->tag = Tag::NIL;
          root->name = "NIL";
          return root;
        }
        switch (std::dynamic_pointer_cast<TypeScalar>(iter->second->type)->type) {
          case (SCALAR_TYPE::BOOLEAN):
            root->exprType = ExprEnum::Identifier;
            if (iter->second->value == "TRUE") {
              root->tag = Tag::B_TRUE;
              root->name = "TRUE";
            } else {
              root->tag = Tag::B_FALSE;
              root->name = "FALSE";
            }
          break;
          case (SCALAR_TYPE::CHAR):
            root->exprType = ExprEnum::Char;
            root->tag = Tag::CHARACTER;
            root->name = iter->second->value;
          break;
          case (SCALAR_TYPE::INTEGER):
            root->exprType = ExprEnum::Integer;
            root->tag = Tag::INTEGER;
            root->name = iter->second->value;
          break;
          case (SCALAR_TYPE::REAL):
            root->exprType = ExprEnum::Real;
            root->tag = Tag::FLOAT;
            root->name = iter->second->value;
          break;
        }
        return root;
      break;
      case (ExprEnum::Function):
        if (ctFuncTable.find(root->name) == ctFuncTable.end())
          throw ExprException("Function \"" + root->print(0) + "\" from pos ("+ std::to_string(root->row) + ", " + std::to_string(root->column) + ") isn't compile-time function;");
      break;
      default: break;
  }
  throw ExprException("wrong symbol \"" + root->name + "\" from pos ("+ std::to_string(root->row) + ", " + std::to_string(root->column) + ");");
};
