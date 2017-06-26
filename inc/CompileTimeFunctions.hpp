#pragma once

#include <functional>
#include "Sym.hpp"

namespace compiler{

typedef std::function<pSymVar(const pSym&)> CompileTimeFunction;

pSymVar _ord(const pSym& src);
pSymVar _chr(const pSym& src);
pSymVar _sizeof(const pSym& src);
pSymVar _int(const pSym& src);
pSymVar _frac(const pSym& src);
pSymVar _trunc(const pSym& src);
pSymVar _round(const pSym& src);
pSymVar _odd(const pSym& src);
pSymVar _writeln(const pSym& src);
pSymVar _write(const pSym& src);
pSymVar _readln(const pSym& src);
pSymVar _read(const pSym& src);
};
