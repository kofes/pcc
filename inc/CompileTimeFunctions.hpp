#pragma once

#include <functional>
#include "Sym.hpp"

namespace compiler{

typedef std::function<pSymVar(const pSym&)> CompileTimeFunction;

pSymVar _ord(const pSym& src) {
};
pSymVar _chr(const pSym& src) {
};
pSymVar _sizeof(const pSym& src) {
};
pSymVar _int(const pSym& src) {
};
pSymVar _frac(const pSym& src) {
};
pSymVar _trunc(const pSym& src) {
};
pSymVar _round(const pSym& src) {
};
pSymVar _odd(const pSym& src) {
};
};
