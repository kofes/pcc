#pragma once

#include <memory>
#include <vector>
#include <map>
#include <sstream>
#include "Node.hpp"

namespace compiler {

enum GLOB {
  PARAMS,
  CONST,
  LOCAL,
  GLOBAL
};

struct Sym;
typedef std::shared_ptr<Sym> pSym;
//first: varName; second: type descriptor
typedef std::map< std::string, pSym > SymTable;
//first: typeName; second: descriptor of type
typedef std::map< std::string, pSym > TypeTable;
typedef std::shared_ptr<SymTable> pSymTable;
typedef std::shared_ptr<TypeTable> pTypeTable;

struct Sym : public Node {
  Sym ( const Lexeme& lex ) : Node(lex) {};
  // virtual std::string print ( unsigned int deep );
};

struct SymType : public Sym {
  SymType ( const Lexeme& lex ) : Sym(lex) {};
  // std::string print ( unsigned int deep ) override;
};
//TOKEN: IDENTIFIER, TAG: UNDEF
struct SymVar : public Sym {
  SymVar ( const Lexeme& lex, const pSym& st ) : Sym(lex), type(st) {};
  std::string print ( unsigned int deep ) override;
  pSym type;
  GLOB glob;
};
//TAG: FUNC, NAME: funcName,
struct SymFunc : public Sym {
  SymFunc ( const Lexeme& lex, const pSym& rt, const pSymTable& tbl, const pNode& bd ) : Sym(lex), retType(rt), symTable(tbl), body(bd) {};
  std::string print ( unsigned int deep ) override;
  pSym retType;
  pSymTable symTable;
  pTypeTable typeTable;
  pNode body;//begin->end;,... (like in GLOBAL!)
};
//NIL, INTEGER, REAL, CHAR, BOOLEAN
struct TypeScalar : public SymType {
  TypeScalar ( const Lexeme& lex ) : SymType(lex) {};
};
//ARRAY: <low>...<high> of <elemType>
struct TypeArray : public SymType {
  TypeArray ( const Lexeme& lex, const pSym& eltype, unsigned long long lw, unsigned long long hgh ) : SymType(lex), elemType(eltype), low(lw), high(hgh) {};
  std::string print ( unsigned int deep ) override;
  pSym elemType;
  unsigned long long low, high;
};
//RECORD: <v11>,...<v1N> : <type1>;...; <vM1>,...,<vMK> : <typeM>; end;
struct TypeRecord : public SymType {
  TypeRecord ( const Lexeme& lex, const pSymTable& tbl ) : SymType(lex), field(tbl) {};
  std::string print ( unsigned int deep ) override;
  pSymTable field;
};
//TAG: POINTER, NAME: nameType
struct TypePointer : public SymType {
  TypePointer ( const Lexeme& lex ) : SymType(lex) {};
};
//TAG: TYPE/ALIAS, NAME: nameNewType, TYPE: what is type was copied?
struct TypeAlias : public SymType {
  TypeAlias ( const Lexeme& lex, const pSym& tp ) : SymType(lex), type(tp) {};
  std::string print ( unsigned int deep ) override;
  pSym type;
};
};
