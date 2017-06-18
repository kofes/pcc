#pragma once

#include <functional>
#include "Sym.hpp"

namespace compiler{

typedef std::function<pSymVar(pSym)> RunTimeFunction;

};
