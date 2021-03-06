#pragma once

#include <memory>
#include <vector>
#include <map>
#include <sstream>
#include "Node.hpp"

namespace compiler {

enum class GLOB : char {
  VAR_PARAM = 'v',
  CONST_PARAM = 'c',
  OUT_PARAM = 'o',
  VAL_PARAM = 'p',
  CONST = 'C',
  VAR = 'V'
};

enum class SCALAR_TYPE : unsigned long int {
  BOOLEAN = static_cast<unsigned long int>(Tag::BOOLEAN),
  CHAR = static_cast<unsigned long int>(Tag::CHAR),
  INTEGER = static_cast<unsigned long int>(Tag::INTEGER),
  REAL = static_cast<unsigned long int>(Tag::REAL)
};

struct Sym;
struct SymVar;
struct SymType;

typedef std::shared_ptr<Sym> pSym;
typedef std::shared_ptr<SymVar> pSymVar;
typedef std::shared_ptr<SymType> pSymType;
//first: varName; second: descriptor of var
typedef std::map< std::string, pSymVar > SymTable;
//first: typeName; second: descriptor of type
typedef std::map< std::string, pSymType > TypeTable;
typedef std::shared_ptr<SymTable> pSymTable;
typedef std::shared_ptr<TypeTable> pTypeTable;

struct Sym : public Node {
  Sym ( const std::string& src ) : name(src) { nodeType = NodeEnum::Sym; };
  // virtual void generate(Generator& asmGenerator);
  // virtual void generateDecl(Generator& asmGenerator);
  // virtual void generateLValue(Generator& asmGenerator);
  // virtual void generateValue(Generator& asmGenerator);
  SymEnum symType;
  std::string name;
  int offset;
  int size;
};

bool operator!= (pSymType type1, pSymType type2);

bool operator== (pSymType type1, pSymType type2);

struct SymType : public Sym {
  SymType ( const std::string& name = "" ) : Sym(name) {};
};
//TOKEN: IDENTIFIER, TAG: UNDEF, NAME: varName
struct SymVar : public Sym {
  SymVar ( const std::string& name = "" ) : Sym(name) { symType = SymEnum::Var; };
  std::string print ( unsigned int deep ) override;
  // void generateDecl(Generator& asmGenerator) override;
  // void generateLValue(Generator& asmGenerator) override;
  // void generateValue(Generator& asmGenerator) override;
  pSymType type;
  GLOB glob;
  std::string value;
};
//TOKEN: IDENTIFIER, TAG: FUNC, NAME: funcName
struct SymFunc : public Sym {
  SymFunc ( const std::string& name ) : Sym(name) { symType = SymEnum::Function; };
  std::string print ( unsigned int deep ) override;
  // void generate(Generator& asmGenerator) override;
  pSymType retType;
  pSymTable params;
  SymTable varTable;
  TypeTable typeTable;
  pNode body;
  int level;
  size_t size_table;
};
//NIL, INTEGER, REAL, CHAR, BOOLEAN
struct TypeScalar : public SymType {
  TypeScalar ( const std::string& name ) : SymType(name) { symType = SymEnum::Scalar; };
  TypeScalar ( SCALAR_TYPE tp );
  SCALAR_TYPE type;
  std::string print ( unsigned int deep ) override;
};
//ARRAY: <low>...<high> of <elemType>
struct TypeArray : public SymType {
  TypeArray ( const std::string& name = "" ) : SymType(name) { symType = SymEnum::Array; };
  std::string print ( unsigned int deep ) override;
  pSymType elemType;
  std::vector<pSymVar> values;
  size_t low, high;
};
//RECORD: <v11>,...<v1N> : <type1>;...; <vM1>,...,<vMK> : <typeM>; end;
struct TypeRecord : public SymType {
  TypeRecord ( const std::string& name ) : SymType(name) { symType = SymEnum::Record; };
  std::string print ( unsigned int deep ) override;
  pSymTable field;
  void checkIdent ( const compiler::Lexeme& lexeme );
};
//TAG: POINTER, NAME: nameType
struct TypePointer : public SymType {
  TypePointer ( const std::string& name ) : SymType(name) { symType = SymEnum::Pointer; };
  std::string print ( unsigned int deep ) override;
  pSymType elemType;
};
//TAG: TYPE/ALIAS, NAME: nameNewType, TYPE: what is type was copied?
struct TypeAlias : public SymType {
  TypeAlias ( const std::string& name ) : SymType(name) { symType = SymEnum::Alias; };
  std::string print ( unsigned int deep ) override;
  pSymType type;
};
}
