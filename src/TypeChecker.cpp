#include "../inc/Parser.hpp"

#include <iostream>

compiler::pSymType compiler::Parser::evalVarType ( pExpr val, SymTable& vTable ) {
  pSymType type = nullptr;
  SymTable::iterator iter = vTable.find(val->name);
  switch (val->exprType) {
    case (ExprEnum::Array):
      type = evalVarType(std::dynamic_pointer_cast<ExprArrayIndex>(val)->left, vTable);
      for (size_t i = 0; i < std::dynamic_pointer_cast<ExprArrayIndex>(val)->args.size(); ++i) {
        if (type->symType != SymEnum::Array)
          errDecl(*val);
        type = std::dynamic_pointer_cast<TypeArray>(type)->elemType;
      }
    break;
    case (ExprEnum::Record):
      type = evalVarType(std::dynamic_pointer_cast<ExprRecordAccess>(val)->left, vTable);
      if (type->symType != SymEnum::Record)
        errDecl(*val);
      iter = std::dynamic_pointer_cast<TypeRecord>(type)->field->find(std::dynamic_pointer_cast<ExprRecordAccess>(val)->right->name);
      if (iter == std::dynamic_pointer_cast<TypeRecord>(type)->field->end())
        errDecl(*(std::dynamic_pointer_cast<ExprRecordAccess>(val)->right));
      type = iter->second->type;
    break;
    case (ExprEnum::Identifier):
      if (iter == vTable.end()) {
        iter = varTable.find(val->name);
        if (iter == varTable.end())
          errDecl(*val);
        type = iter->second->type;
        if (iter->second->glob == GLOB::CONST)
          errConst(*val);
      } else {
        type = iter->second->type;
        if (iter->second->glob == GLOB::CONST)
          errConst(*val);
      }
    return type;
    case (ExprEnum::UnOp):
      if (val->tag != Tag::POINTER)
        errDecl(*val);
      type = evalVarType(std::dynamic_pointer_cast<ExprUnOp>(val)->arg, vTable);
      if (type->symType != SymEnum::Pointer)
        errDecl(*val);
      type = std::dynamic_pointer_cast<TypePointer>(type)->elemType;
    break;
    default : err(*val);
  }

  return type;
};

compiler::pSymType compiler::Parser::evalExprType( pExpr val, SymTable& vTable ) {
  pSymType type = nullptr, rType = nullptr;
  SymTable::iterator iterVar = vTable.find(val->name);
  auto iterFunc = funcTable.find(val->name);
  auto iterCTFunc = ctFuncTable.find(val->name);
  std::ostringstream sstream;
  int countArgs = 0;
  SCALAR_TYPE sc1, sc2;
  switch (val->exprType) {
    case (ExprEnum::Array):
      type = evalExprType(std::dynamic_pointer_cast<ExprArrayIndex>(val)->left, vTable);
      for (size_t i = 0; i < std::dynamic_pointer_cast<ExprArrayIndex>(val)->args.size(); ++i) {
        if (type->symType != SymEnum::Array)
          errDecl(*val);
        type = std::dynamic_pointer_cast<TypeArray>(type)->elemType;
      }
    return evalAlias(type);
    case (ExprEnum::Record):
      type = evalExprType(std::dynamic_pointer_cast<ExprRecordAccess>(val)->left, vTable);
      if (type->symType != SymEnum::Record)
        errDecl(*val);
      iterVar = std::dynamic_pointer_cast<TypeRecord>(type)->field->find(std::dynamic_pointer_cast<ExprRecordAccess>(val)->right->name);
      if (iterVar == std::dynamic_pointer_cast<TypeRecord>(type)->field->end())
        errDecl(*(std::dynamic_pointer_cast<ExprRecordAccess>(val)->right));
    return evalAlias(iterVar->second->type);
    case (ExprEnum::Identifier):
      if (val->tag == Tag::NIL) return pSymType(new TypePointer("NIL"));
      if (val->tag == Tag::B_FALSE) {
        type = pSymType(new TypeScalar(SCALAR_TYPE::BOOLEAN));
        type->name = "FALSE";
        return type;
      }
      if (val->tag == Tag::B_TRUE) {
        type = pSymType(new TypeScalar(SCALAR_TYPE::BOOLEAN));
        type->name = "TRUE";
        return type;
      }
      if (iterVar == vTable.end())
        errDecl(*val);
    return evalAlias(iterVar->second->type);
    break;
    case (ExprEnum::UnOp):
      type = evalAlias(evalExprType(std::dynamic_pointer_cast<ExprUnOp>(val)->arg, vTable));
      if (val->tag == Tag::POINTER) {
        if (type->symType != SymEnum::Pointer || type->name == "NIL")
          errDecl(*val);
        return evalAlias(std::dynamic_pointer_cast<TypePointer>(type)->elemType);
      }
      if (val->tag == Tag::ADD || val->tag == Tag::SUB) {
        if (type->symType != SymEnum::Scalar &&
          std::dynamic_pointer_cast<TypeScalar>(type)->type != SCALAR_TYPE::INTEGER &&
          std::dynamic_pointer_cast<TypeScalar>(type)->type != SCALAR_TYPE::REAL)
          throw ExprException("Incompatible unary operation at pos (" + std::to_string(val->row) + ", " + std::to_string(val->column) + ");");
        return evalAlias(type);
      }
      if (val->tag == Tag::NOT) {
        if (type->symType != SymEnum::Scalar &&
          std::dynamic_pointer_cast<TypeScalar>(type)->type != SCALAR_TYPE::INTEGER &&
          std::dynamic_pointer_cast<TypeScalar>(type)->type != SCALAR_TYPE::BOOLEAN)
          throw ExprException("Incompatible unary operation at pos (" + std::to_string(val->row) + ", " + std::to_string(val->column) + ");");
        return evalAlias(type);
      }
    throw ExprException("Incompatible unary operation at pos (" + std::to_string(val->row) + ", " + std::to_string(val->column) + ");");
    case (ExprEnum::BinOp):
      type = evalAlias(evalExprType(std::dynamic_pointer_cast<ExprBinOp>(val)->left, vTable));
      rType = evalAlias(evalExprType(std::dynamic_pointer_cast<ExprBinOp>(val)->right, vTable));
      switch (val->tag) {
        case (Tag::GREATER):
        case (Tag::LESS):
        case (Tag::GREATER_EQ):
        case (Tag::LESS_EQ):
          if (type->symType != SymEnum::Scalar || type->symType != SymEnum::Scalar)
            throw ExprException("Incompatible binary operation at pos (" + std::to_string(val->row) + ", " + std::to_string(val->column) + ");");
          sc1 = std::dynamic_pointer_cast<TypeScalar>(type)->type;
          sc2 = std::dynamic_pointer_cast<TypeScalar>(rType)->type;
          if ((sc1 != SCALAR_TYPE::CHAR || sc2 != SCALAR_TYPE::CHAR) &&
              ((sc1 != SCALAR_TYPE::INTEGER && sc1 != SCALAR_TYPE::REAL) ||
              (sc2 != SCALAR_TYPE::INTEGER && sc2 != SCALAR_TYPE::REAL)))
            throw ExprException("Incompatible binary operation at pos (" + std::to_string(val->row) + ", " + std::to_string(val->column) + ");");
          return pSymType(new TypeScalar(SCALAR_TYPE::BOOLEAN));
        case (Tag::ADD):
        case (Tag::SUB):
        case (Tag::MUL):
          if (type->symType != SymEnum::Scalar || type->symType != SymEnum::Scalar)
            throw ExprException("Incompatible binary operation at pos (" + std::to_string(val->row) + ", " + std::to_string(val->column) + ");");
          sc1 = std::dynamic_pointer_cast<TypeScalar>(type)->type;
          sc2 = std::dynamic_pointer_cast<TypeScalar>(rType)->type;
          if ((sc1 != SCALAR_TYPE::REAL && sc1 != SCALAR_TYPE::INTEGER) ||
              (sc2 != SCALAR_TYPE::REAL && sc2 != SCALAR_TYPE::INTEGER))
            throw ExprException("Incompatible binary operation at pos (" + std::to_string(val->row) + ", " + std::to_string(val->column) + ");");
          if (sc1 == SCALAR_TYPE::REAL || sc2 == SCALAR_TYPE::REAL)
            return pSymType(new TypeScalar(SCALAR_TYPE::REAL));
          return pSymType(new TypeScalar(SCALAR_TYPE::INTEGER));
        case (Tag::DIV_FLOAT):
          if (type->symType != SymEnum::Scalar || type->symType != SymEnum::Scalar)
            throw ExprException("Incompatible binary operation at pos (" + std::to_string(val->row) + ", " + std::to_string(val->column) + ");");
          sc1 = std::dynamic_pointer_cast<TypeScalar>(type)->type;
          sc2 = std::dynamic_pointer_cast<TypeScalar>(rType)->type;
          if ((sc1 != SCALAR_TYPE::REAL && sc1 != SCALAR_TYPE::INTEGER) ||
              (sc2 != SCALAR_TYPE::REAL && sc2 != SCALAR_TYPE::INTEGER))
            throw ExprException("Incompatible binary operation at pos (" + std::to_string(val->row) + ", " + std::to_string(val->column) + ");");
          return type;
        case (Tag::DIV_INT):
        case (Tag::MOD):
        case (Tag::SHL):
        case (Tag::SHR):
          if (type->symType != SymEnum::Scalar || type->symType != SymEnum::Scalar)
            throw ExprException("Incompatible binary operation at pos (" + std::to_string(val->row) + ", " + std::to_string(val->column) + ");");
          sc1 = std::dynamic_pointer_cast<TypeScalar>(type)->type;
          sc2 = std::dynamic_pointer_cast<TypeScalar>(rType)->type;
          if (sc1 != SCALAR_TYPE::INTEGER || sc2 != SCALAR_TYPE::INTEGER)
            throw ExprException("Incompatible binary operation at pos (" + std::to_string(val->row) + ", " + std::to_string(val->column) + ");");
          return type;
        case (Tag::OR):
        case (Tag::XOR):
        case (Tag::AND):
          if (type->symType != SymEnum::Scalar || type->symType != SymEnum::Scalar)
            throw ExprException("Incompatible binary operation at pos (" + std::to_string(val->row) + ", " + std::to_string(val->column) + ");");
          sc1 = std::dynamic_pointer_cast<TypeScalar>(type)->type;
          sc2 = std::dynamic_pointer_cast<TypeScalar>(rType)->type;
          if ((sc1 != SCALAR_TYPE::INTEGER || sc2 != SCALAR_TYPE::INTEGER) &&
              (sc1 != SCALAR_TYPE::BOOLEAN || sc2 != SCALAR_TYPE::BOOLEAN))
            throw ExprException("Incompatible binary operation at pos (" + std::to_string(val->row) + ", " + std::to_string(val->column) + ");");
          return type;
        case (Tag::EQUALS):
        case (Tag::MIS):
          if (type->symType != SymEnum::Scalar || type->symType != SymEnum::Scalar)
            throw ExprException("Incompatible binary operation at pos (" + std::to_string(val->row) + ", " + std::to_string(val->column) + ");");
          sc1 = std::dynamic_pointer_cast<TypeScalar>(type)->type;
          sc2 = std::dynamic_pointer_cast<TypeScalar>(rType)->type;
          if (((sc1 != SCALAR_TYPE::REAL && sc1 != SCALAR_TYPE::INTEGER) ||
              (sc2 != SCALAR_TYPE::REAL && sc2 != SCALAR_TYPE::INTEGER)) &&
              (sc1 != SCALAR_TYPE::BOOLEAN || sc2 != SCALAR_TYPE::BOOLEAN) &&
              (sc1 != SCALAR_TYPE::CHAR || sc2 != SCALAR_TYPE::CHAR))
            throw ExprException("Incompatible binary operation at pos (" + std::to_string(val->row) + ", " + std::to_string(val->column) + ");");
          return pSymType(new TypeScalar(SCALAR_TYPE::BOOLEAN));
        break;
        default: break;
      }
    throw ExprException("Incompatible binary operation at pos (" + std::to_string(val->row) + ", " + std::to_string(val->column) + ");");
    case (ExprEnum::Function):
      if (iterFunc == funcTable.end()) {
        if (iterCTFunc == ctFuncTable.end())
          errDecl(*val);
      }//TODO: iterCTFunc->run.
      for (pExpr elem : std::dynamic_pointer_cast<ExprFunc>(val)->args) {
        pSymType tmp_type = evalAlias(evalExprType(val, vTable));
        if (!countArgs || type == tmp_type) {
          ++countArgs;
        } else {
          sstream << countArgs;
          if (type != nullptr) sstream << ':' << type->print(0);
          sstream << ';';
          countArgs = 1;
        }
        type = tmp_type;
      }
      if (countArgs) {
        sstream << countArgs;
        if (type != nullptr) sstream << ':' << type->print(0);
        sstream << ';';
        countArgs = 0;
      }
      type = nullptr;
      for (const std::pair< std::string, pSym>& it : iterFunc->second) {
        if (unification(sstream.str(), it.first)) {
          type = std::dynamic_pointer_cast<SymFunc>(it.second)->retType;
          break;
        }
      }
      if (type == nullptr) err(*val);
    return evalAlias(type);
    case (ExprEnum::Char): return pSymType(new TypeScalar(SCALAR_TYPE::REAL));
    case (ExprEnum::Integer): return pSymType(new TypeScalar(SCALAR_TYPE::INTEGER));
    case (ExprEnum::Real): return pSymType(new TypeScalar(SCALAR_TYPE::REAL));
    case (ExprEnum::String): err(*val);
  }

  return type;
};

//TODO
bool compiler::Parser::unification ( const std::string& first, const std::string& second) {

  return false;
};

compiler::pSymType compiler::evalAlias ( pSymType type ) {
  while (type->symType == SymEnum::Alias)
    type = std::dynamic_pointer_cast<TypeAlias>(type)->type;
  return type;
};
