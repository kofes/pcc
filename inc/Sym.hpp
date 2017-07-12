#pragma once

#include <memory>
#include <vector>
#include <map>
#include <sstream>
#include "Node.hpp"

namespace compiler {

enum class SymEnum {
  //Params
  ValParam,
  ConstParam,
  VarParam,
  OutParam,
  //Variables
  VarConst,
  VarGlobal,
  VarLocal,
  //Consts
  ConstBool,
  ConstInt,
  ConstFloat,
  ConstChar,
  ConstPointer,
  //Basic types
  Bool,
  Int,
  Float,
  Char,
  Pointer,
  //Complex types
  Array,
  OpenArray,
  Record,
  Alias,
  String,
  //Function
  Function,
};

struct Sym;
struct SymVar;
struct SymType;
class SymTable;

typedef std::shared_ptr<Sym> pSym;
typedef std::shared_ptr<SymVar> pSymVar;
typedef std::shared_ptr<SymType> pSymType;
typedef std::shared_ptr<SymTable> pSymTable;

class SymTable {
public:
  void pushSymbol ( pSym symbol );
  bool isSymbolExist ( const std::string& name );
  pSym find ( const std::string& name );
  void checkIdentifier ( const std::string& name);
private:
  std::vector <pSym> symbols;
  std::map< std::string, pSymVar > symBook;
};
class SymStack {
public:
  void pushTable ( pSymTable table );
  pSymTable top();
  void pop();
  pSymTable find ( const std::string& symbol );
  bool isSymbolExist ( const std::string& symbol );
private:
  std::list<SymTable> tables;
};

struct Sym : public Node {
  Sym ( const std::string& src, SymEnum sym ) : name(src), sym(sym) {};
  std::string name;
  SymEnum sym;
};

bool operator!= (pSymType type1, pSymType type2);
bool operator== (pSymType type1, pSymType type2);

struct SymVar : public Sym {
  SymVar ( const std::string& name, SymEnum sym ) : Sym(name, sym) {};
  std::string print ( unsigned int deep ) override;
  pSymType type;
  std::string value;
};
struct SymVarConst : public SymVar {
  SymVarConst ( const std::string& name ) : SymVar(name, SymEnum::VarConst) {};
};
struct SymVarGlobal : public SymVar {
  SymVarGlobal ( const std::string& name ) : SymVar(name, SymEnum::VarGlobal) {};
};
struct SymVarLocal : public SymVar {
  SymVarLocal ( const std::string& name ) : SymVar(name, SymEnum::VarLocal) {};
};

struct SymParam : public SymVar {
  SymParam ( const std::string& name, SymEnum sym ) : SymVar(name, sym) {};
  std::string print ( unsigned int deep ) override;
  pSym elem;
};
struct SymValParam : public SymParam {
  SymValParam ( const std::string ) : SymParam(name, SymEnum::ValParam) {};
};
struct SymConstParam : public SymParam {
  SymConstParam ( const std::string ) : SymParam(name, SymEnum::ConstParam) {};
};
struct SymVarParam : public SymParam {
  SymVarParam ( const std::string ) : SymParam(name, SymEnum::VarParam) {};
};
struct SymOutParam : public SymParam {
  SymOutParam ( const std::string ) : SymParam(name, SymEnum::OutParam) {};
};

struct SymConst : public Sym {
  SymConst ( const std::string& name, SymEnum sym ) : Sym(name, sym) {};
};
struct SymConstBool : public SymConst {
  SymConstBool ( const std::string& name) : SymConst(name, SymEnum::ConstBool) {};
  bool value;
};
struct SymConstInteger : public SymConst {
  SymConstInteger ( const std::string& name) : SymConst(name, SymEnum::ConstInt) {};
  int value;
};
struct SymConstFloat : public SymConst {
  SymConstFloat ( const std::string& name) : SymConst(name, SymEnum::ConstFloat) {};
  double value;
};
struct SymConstChar : public SymConst {
  SymConstChar ( const std::string& name) : SymConst(name, SymEnum::ConstChar) {};
  float value;
};
struct SymConstPointer : public SymConst {
  SymConstPointer ( const std::string& name) : SymConst(name, SymEnum::ConstPointer) {};
  pSymVar value;
};

struct SymType : public Sym {
  SymType ( const std::string& name, SymEnum sym ) : Sym(name, sym) {};
  std::string print ( unsigned int deep ) override;
};
struct SymTypeBool : public SymType {
  SymTypeBool ( const std::string& name ) : SymType(name, SymEnum::Bool) {};
};
struct SymTypeInt : public SymType {
  SymTypeInt ( const std::string& name ) : SymType(name, SymEnum::Int) {};
};
struct SymTypeFloat : public SymType {
  SymTypeFloat ( const std::string& name ) : SymType(name, SymEnum::Float) {};
};
struct SymTypeChar : public SymType {
  SymTypeChar ( const std::string& name ) : SymType(name, SymEnum::Char) {};
};
struct SymTypePointer : public SymType {
  SymTypePointer ( const std::string& name ) : SymType(name, SymEnum::Pointer) {};
  std::string print ( unsigned int deep ) override;
  pSymType type;
};

struct SymTypeArray : public SymType {
  SymTypeArray ( const std::string& name = "" ) : SymType(name, SymEnum::Array) {};
  std::string print ( unsigned int deep ) override;
  pSymType elem;
  std::vector<pSymVar> values;
  size_t low, high;
};
struct SymTypeOpenArray : public SymType {
  SymTypeOpenArray ( const std::string& name = "" ) : SymType(name, SymEnum::OpenArray) {};
  pSymType elem;
};
struct SymTypeRecord : public SymType {
  SymTypeRecord ( const std::string& name ) : SymType(name, SymEnum::Record) {};
  std::string print ( unsigned int deep ) override;
  pSymTable fields;
  void checkIdent ( const compiler::Lexeme& lexeme );
};
struct SymTypeAlias : public SymType {
  SymTypeAlias ( const std::string& name ) : SymType(name, SymEnum::Alias) {};
  std::string print ( unsigned int deep ) override;
  pSymType type;
};
struct SymTypeString : public SymType {
  SymTypeString ( const std::string& name ) : SymType(name, SymEnum::String) {};
  std::string value;
};

struct SymFunction : public Sym {
  SymFunction ( const std::string& name ) : Sym(name, SymEnum::Function) {};
  std::string print ( unsigned int deep ) override;
  pSymType ret;
  pSymTable args, locals;
  pNode body;
};
}
