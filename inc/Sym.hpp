#pragma once

#include <memory>
#include <vector>
#include <map>
#include <sstream>
#include "Node.hpp"

namespace compiler {

enum class GLOB : char {
  PARAM = 'P',
  CONST = 'C',
  LOCAL = 'L',
  GLOBAL = 'G'
};

enum class SCALAR_TYPE : unsigned long int {
  POINTER = static_cast<unsigned long int>(Tag::POINTER),
  BOOLEAN = static_cast<unsigned long int>(Tag::BOOLEAN),
  CHAR = static_cast<unsigned long int>(Tag::CHAR),
  INTEGER = static_cast<unsigned long int>(Tag::INTEGER),
  REAL = static_cast<unsigned long int>(Tag::REAL)
};

struct Sym;
struct SymVar;
typedef std::shared_ptr<Sym> pSym;
typedef std::shared_ptr<SymVar> pSymVar;
//first: varName; second: descriptor of var
typedef std::map< std::string, pSymVar > SymTable;
//first: typeName; second: descriptor of type
typedef std::map< std::string, pSym > TypeTable;
typedef std::shared_ptr<SymTable> pSymTable;
typedef std::shared_ptr<TypeTable> pTypeTable;

struct Sym : public Node {
  Sym ( const Lexeme& lex ) : Node(lex) { nodeType = NodeEnum::Sym; };
  SymEnum symType;
};

struct SymType : public Sym {
  SymType ( const Lexeme& lex ) : Sym(lex) {};
};
//TOKEN: IDENTIFIER, TAG: UNDEF, NAME: varName
struct SymVar : public Sym {
  SymVar ( const Lexeme& lex ) : Sym(lex) { symType = SymEnum::Var; };
  std::string print ( unsigned int deep ) override;
  pSym type;
  GLOB glob;
  std::string value;
};
//TOKEN: IDENTIFIER, TAG: FUNC, NAME: funcName
struct SymFunc : public Sym {
  SymFunc ( const Lexeme& lex ) : Sym(lex) { symType = SymEnum::Function; };
  SymFunc ( const Lexeme& lex, const pSym& rt, const pSymTable& prms, const pNode& bd ) :
    Sym(lex), retType(rt), params(prms), body(bd) { symType = SymEnum::Function; };
  //TODO!
  std::string print ( unsigned int deep ) override;
  pSym retType;
  pSymTable params;
  SymTable varTable;
  TypeTable typeTable;
  pNode body;//begin->end;,... (like in GLOBAL!)
};
//NIL, INTEGER, REAL, CHAR, BOOLEAN
struct TypeScalar : public SymType {
  TypeScalar ( const Lexeme& lex ) : SymType(lex) { symType = SymEnum::Scalar; };
  TypeScalar ( SCALAR_TYPE tp );
  std::string print ( unsigned int deep ) override;
};
//ARRAY: <low>...<high> of <elemType>
struct TypeArray : public SymType {
  TypeArray ( const Lexeme& lex ) : SymType(lex) { symType = SymEnum::Array; };
  TypeArray ( const Lexeme& lex, const pSym& eltype, unsigned long long lw, unsigned long long hgh ) :
    SymType(lex), elemType(eltype), low(lw), high(hgh) { symType = SymEnum::Array; };
  std::string print ( unsigned int deep ) override;
  pSym elemType;
  unsigned long long low, high;
};
//RECORD: <v11>,...<v1N> : <type1>;...; <vM1>,...,<vMK> : <typeM>; end;
struct TypeRecord : public SymType {
  TypeRecord ( const Lexeme& lex, const pSymTable& tbl ) : SymType(lex), field(tbl) { symType = SymEnum::Record; };
  std::string print ( unsigned int deep ) override;
  pSymTable field;
};
//TAG: POINTER, NAME: nameType
struct TypePointer : public SymType {
  TypePointer ( const Lexeme& lex ) : SymType(lex) { symType = SymEnum::Pointer; };
  std::string print ( unsigned int deep ) override;
  pSym elemType;
};
//TAG: TYPE/ALIAS, NAME: nameNewType, TYPE: what is type was copied?
struct TypeAlias : public SymType {
  TypeAlias ( const Lexeme& lex, const pSym& tp ) : SymType(lex), type(tp) { symType = SymEnum::Alias; };
  std::string print ( unsigned int deep ) override;
  pSym type;
};
};
