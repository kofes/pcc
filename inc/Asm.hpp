#pragma once

#include <map>
#include <memory>
#include <vector>
#include <list>
#include <string>
#include <sstream>
//namespace compiler::asm
namespace compiler {

const static int MAIN_WORD_SIZE = 8;
const static std::string MAIN_WORD_STR = "qword";
const static std::string DEFINE_NUM = "dq";
const static std::string DEFINE_CHAR = "db";
const static std::string FORMAT_INT = "formatInt";
const static std::string FORMAT_FLOAT = "formatFloat";

enum class RegisterEnum : unsigned char {
  rax_,
  rdx_,
  rcx_,
  rbx_,
  rsp_,
  rbp_,
  rsi_,
  rdi_,
  xmm0_,
  xmm1_
};

enum class OperationEnum : size_t {
  ret_,

  push_,
  pop_,

  call_,
  jmp_,
  jz_,
  jnz_,
  jc_,
  jnc_,
  ja_,
  jna_,
  jg_,
  jge_,
  jl_,
  jle_,

  add_,
  addsd_,
  divsd_,
  idiv_,
  sub_,
  subsd_,
  imul_,
  mulsd_,
  and_,
  or_,
  xor_,
  mov_,
  movq_,
  cvtsi2sd_,
  sar_,
  sal_,
  lea_,
  cmp_,
  test_,
  not_,
  shr_,
  shl_,
  label_
};

enum class CmdEnum : unsigned char {
    line_,
    section_,
    global_,
    format_,
    constant_,
    reg_,
    mark_,
    imn_,
    ind_,
    cmd_,
    cmd0_,
    cmd1_,
    cmd2_,
    label_,
};

enum class OperandEnum : unsigned char {
  operand_,
  reg_,
  mem_,
  imm_string_,
  imm_int_,
};

static std::map<OperationEnum, std::string> operations = {
  {OperationEnum::ret_, "ret"},
  {OperationEnum::push_, "push"},
  {OperationEnum::pop_, "pop"},
  {OperationEnum::call_, "call"},
  {OperationEnum::jmp_, "jmp"},
  {OperationEnum::jz_, "jz"},
  {OperationEnum::jnz_, "jnz"},
  {OperationEnum::jc_, "jc"},
  {OperationEnum::jnc_, "jnc"},
  {OperationEnum::ja_, "ja"},
  {OperationEnum::jna_, "jna"},
  {OperationEnum::jg_, "jg"},
  {OperationEnum::jge_, "jge"},
  {OperationEnum::jl_, "jl"},
  {OperationEnum::jle_, "jle"},
  {OperationEnum::add_, "add"},
  {OperationEnum::addsd_, "addsd"},
  {OperationEnum::divsd_, "divsd"},
  {OperationEnum::idiv_, "idiv"},
  {OperationEnum::sub_, "sub"},
  {OperationEnum::subsd_, "subsd"},
  {OperationEnum::imul_, "imul"},
  {OperationEnum::mulsd_, "mulsd"},
  {OperationEnum::and_, "and"},
  {OperationEnum::or_, "or"},
  {OperationEnum::xor_, "xor"},
  {OperationEnum::mov_, "mov"},
  {OperationEnum::movq_, "movq"},
  {OperationEnum::cvtsi2sd_, "cvtsi2sd"},
  {OperationEnum::sar_, "sar"},
  {OperationEnum::sal_, "sal"},
  {OperationEnum::lea_, "lea"},
  {OperationEnum::cmp_, "cmp"},
  {OperationEnum::test_, "test"},
  {OperationEnum::not_, "not"},
  {OperationEnum::shr_, "shr"},
  {OperationEnum::shl_, "shl"},
  {OperationEnum::label_, "label"},
};

static std::map<RegisterEnum, std::string> registers = {
  {RegisterEnum::rax_, "rax"},
  {RegisterEnum::rbx_, "rbx"},
  {RegisterEnum::rcx_, "rcx"},
  {RegisterEnum::rdx_, "rdx"},
  {RegisterEnum::rbp_, "rbp"},
  {RegisterEnum::rsp_, "rsp"},
  {RegisterEnum::rsi_, "rsi"},
  {RegisterEnum::rdi_, "rdi"},
  {RegisterEnum::xmm0_, "xmm0"},
  {RegisterEnum::xmm1_, "xmm1"},
};

struct Operand;
struct Cmd;
struct Mem;
struct Data;
typedef std::shared_ptr<Operand> pOperand;
typedef std::shared_ptr<Cmd> pCmd;
typedef std::shared_ptr<Mem> pMem;
typedef std::shared_ptr<Data> pData;

struct Operand {
  virtual std::string name() = 0;
  virtual OperandEnum operandType() { return OperandEnum::operand_; };
  virtual bool isImmediate() { return false; };
};

struct Cmd {
  Cmd (OperationEnum oper, pOperand operand1 = pOperand(), pOperand operand2 = pOperand()) : oper_(oper), operand1_(operand1), operand2_(operand2) {};
  virtual std::string name() { return ""; };
  virtual CmdEnum cmdType() { return CmdEnum::cmd_; };
  OperationEnum oper_;
  pOperand operand1_, operand2_;
};

struct Cmd0 : public Cmd {
  Cmd0(OperationEnum oper) : Cmd(oper) {};
  std::string name() override { return operations[oper_]; };
  CmdEnum cmdType() override { return CmdEnum::cmd0_; };
};

struct Cmd1 : public Cmd {
  Cmd1(OperationEnum oper, pOperand operand) : Cmd(oper, operand) {};
  std::string name() override { return operations[oper_] + " " + operand1_->name(); };
  CmdEnum cmdType() override { return CmdEnum::cmd1_; };
};

struct Cmd2 : public Cmd {
  Cmd2(OperationEnum oper, pOperand operand1, pOperand operand2) : Cmd(oper, operand1, operand2) {};
  std::string name() override { return operations[oper_] + " " + operand1_->name() + " " + operand2_->name(); };
  CmdEnum cmdType() override { return CmdEnum::cmd2_; };
};

struct Label : public Cmd {
  Label(const std::string& name) : Cmd(OperationEnum::label_) {};
  CmdEnum cmdType() override { return CmdEnum::label_; };
  std::string name() override { return name_ + ':'; };
private:
  std::string name_;
};

struct Reg : public Operand {
  Reg (RegisterEnum reg) : reg_(reg) {};
  std::string name() override { return registers[reg_]; };
  OperandEnum operandType() override { return OperandEnum::reg_; };
  RegisterEnum reg() { return reg_; };
private:
  RegisterEnum reg_;
};

struct Immediate : public Operand {
  bool isImmediate() override { return true; };
};

struct ImmInt : public Immediate {
  ImmInt (int val) : val_(val) {};
  std::string name() override { return std::to_string(val_); };
  OperandEnum operandType() override { return OperandEnum::imm_int_; };
  int val() { return val_; };
private:
  int val_;
};

struct ImmString : public Immediate {
  ImmString (const std::string& val) : val_(val) {};
  std::string name() override { return val_; };
  OperandEnum operandType() override { return OperandEnum::imm_string_; };
private:
  std::string val_;
};

struct Mem : public Operand {
  Mem (const std::string& name, int offset = 0) : operand_(new ImmString(name)), offset_(offset) {};
  Mem (RegisterEnum reg, int offset = 0) : operand_(new Reg(reg)), offset_(offset) {};
  Mem (const Mem& src) : operand_(src.operand_), offset_(src.offset_), size_(MAIN_WORD_STR) {};
  Mem (pOperand src);
  std::string name() override;
  OperandEnum operandType() override { return OperandEnum::mem_; };
  pOperand operand() { return operand_; };
  void clear() { size_.clear(); };
  void addOffset (int addition) { offset_+=addition; };
  int offset() { return offset_; };
private:
    pOperand operand_;
    int offset_;
    std::string size_;
};

struct Data {
  Data (const std::string& name) : name_(name) {};
  virtual std::string name() = 0;
protected:
  std::string name_;
};

struct Int : public Data {
  Int (const std::string& name, int val) : Data(name), val_(val) {};
  std::string name() override { return name_ + ": " + DEFINE_NUM + " " + std::to_string(val_); };
private:
  int val_;
};

struct Float : public Data {
  Float (const std::string& name, double val) : Data(name), val_(val) {};
  std::string name() override { return name_ + ": " + DEFINE_NUM + " " + std::to_string(val_); };
private:
  double val_;
};

struct Array : public Data {
  Array (const std::string& name, int size) : Data(name), size_(size) {};
  std::string name() override { return name_+": times "+std::to_string(size_) + " " + DEFINE_CHAR; };
private:
  int size_;
};

struct String : public Data {
  String (const std::string& name, std::string val) : Data(name), val_(val) {};
  std::string name() override { return name_ + ": " + DEFINE_CHAR + " " + val_ + ", 0"; };
private:
    std::string val_;
};

pCmd to_cmd(OperationEnum oper);
pCmd to_cmd(OperationEnum oper, RegisterEnum reg);
pCmd to_cmd(OperationEnum oper, int val);
pCmd to_cmd(OperationEnum oper, const std::string& val);
pCmd to_cmd(OperationEnum oper, pOperand operand);
pCmd to_cmd(OperationEnum oper, RegisterEnum to, RegisterEnum src);
pCmd to_cmd(OperationEnum oper, RegisterEnum reg, const std::string& val);
pCmd to_cmd(OperationEnum oper, RegisterEnum reg, int val);
pCmd to_cmd(OperationEnum oper, pOperand operand, RegisterEnum reg);
pCmd to_cmd(OperationEnum oper, RegisterEnum reg, pOperand operand);

class Generator {
public:
  Generator();
  std::string print();
  void addCmd(pCmd cmd);
  void writeInt();
  void writeNewLine();
  void writeFloat();
  void writeString(const std::string& src);
  void addData(const std::string& name, const std::string& val);
  void addData(const std::string& name, int val);
  void addData(const std::string& name, double val);
  void addArray(const std::string& name, int size);
  void addLabel(const std::string& name) { addCmd(pCmd(new Label(name))); };
  std::string varName(const std::string& name) { return "var" + name; };
  std::string genVar() { return "var" + std::to_string(countNames++); };
  std::string genLabel() { return "L" + std::to_string(countLabels++); };
  pOperand operandAdress(const std::string& name, int offset = 0);
  pOperand operandAdress(RegisterEnum reg, int offset = 0);
protected:
  void write(const std::string& format);
  std::vector<pCmd> cmds;
  std::vector<pData> data;
  size_t countNames, countPushes, countLabels;
};

};
