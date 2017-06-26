#include "../inc/Asm.hpp"

compiler::Mem::Mem(compiler::pOperand src) : size_(MAIN_WORD_STR) {
  pMem mem = std::dynamic_pointer_cast<Mem>(src);
  operand_ = mem->operand_;
  offset_ = mem->offset_;
};

std::string compiler::Mem::name() {
  return (size_.length() ? (size_ + " ") : "") + "[" + operand_->name() + (!offset_ ? "" : std::to_string(offset_)) + "]";
};

void compiler::Generator::addCmd(pCmd cmd) {
  if (cmd->oper_ == OperationEnum::push_) {++countPushes;}
  else if (cmd->oper_ == OperationEnum::pop_) {--countPushes;}
  else if (cmd->cmdType() == CmdEnum::cmd2_ &&
           cmd->operand1_->operandType() == OperandEnum::reg_ &&
           std::dynamic_pointer_cast<Reg>(cmd->operand1_)->reg() == RegisterEnum::rsp_ &&
           cmd->operand2_->operandType() == OperandEnum::imm_int_) {
    auto int_imm_size = std::dynamic_pointer_cast<ImmInt>(cmd->operand2_)->val() / MAIN_WORD_SIZE;
    if (cmd->oper_ == OperationEnum::add_)
      countPushes += int_imm_size;
    else if (cmd->oper_ == OperationEnum::sub_)
      countPushes -= int_imm_size;
  }
  cmds.push_back(cmd);
};

void compiler::Generator::write(const std::string &format) {
  bool sub = false;
  addCmd(to_cmd(OperationEnum::mov_, RegisterEnum::rdi_, format));
  if (!(countPushes % 2)) {
    addCmd(to_cmd(OperationEnum::sub_, RegisterEnum::rsp_, MAIN_WORD_SIZE));
    sub = true;
  }
  addCmd(to_cmd(OperationEnum::call_, "printf"));
  if (sub) addCmd(to_cmd(OperationEnum::add_, RegisterEnum::rsp_, MAIN_WORD_SIZE));
};

void compiler::Generator::writeInt() {
  addCmd(to_cmd(OperationEnum::pop_, RegisterEnum::rax_));
  addCmd(to_cmd(OperationEnum::mov_, RegisterEnum::rsi_, RegisterEnum::rax_));
  addCmd(to_cmd(OperationEnum::xor_, RegisterEnum::rax_, RegisterEnum::rax_));
  write(FORMAT_INT);
};

void compiler::Generator::writeFloat() {
  addCmd(to_cmd(OperationEnum::pop_, RegisterEnum::rax_));
  addCmd(to_cmd(OperationEnum::movq_, RegisterEnum::xmm0_, RegisterEnum::rax_));
  addCmd(to_cmd(OperationEnum::mov_, RegisterEnum::rax_, 1));
  write(FORMAT_INT);
};

void compiler::Generator::writeString(const std::string& val) {
  std::string name = genVar();
  addData(name, "\"" + val + "\"");
  addCmd(to_cmd(OperationEnum::xor_, RegisterEnum::rax_, RegisterEnum::rax_));
  write(name);
};
