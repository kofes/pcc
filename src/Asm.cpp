#include "../inc/Asm.hpp"

compiler::Mem::Mem(compiler::pOperand src) : size_("qword") {
  pMem mem = std::dynamic_pointer_cast<Mem>(src);
  operand_ = mem->operand_;
  offset_ = mem->offset_;
};

std::string compiler::Mem::name() {
  return (size_.length() ? (size_ + " ") : "") + "[" + operand_->name() + (!offset_ ? "" : std::to_string(offset_)) + "]";
};
