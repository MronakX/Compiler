#pragma once

#include "AST.h"
#include "koopa.h"
#include <map>

#define ALIGN_NUM 16
inline int top_stack_ptr = 0;
inline int stack_size = 0;
std::map<koopa_raw_value_t, int> value_map;

void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_slice_t &slice);
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_value_t &value);

void Visit(const koopa_raw_return_t &ret);
void Visit(const koopa_raw_integer_t &integer, const koopa_raw_value_t &value);
void Visit(const koopa_raw_binary_t &binary, const koopa_raw_value_t &value);
void Visit(const koopa_raw_load_t &load_val, const koopa_raw_value_t &value);
void Visit(const koopa_raw_store_t &store_val, const koopa_raw_value_t &value);
void Visit(const koopa_raw_branch_t &branch, const koopa_raw_value_t &value);
void Visit(const koopa_raw_jump_t &jump, const koopa_raw_value_t &value);

void load(const koopa_raw_value_t &value, std::string type);
void store(const koopa_raw_value_t &value, std::string type);

void Visit(const koopa_raw_program_t &program)
{
	// 访问所有全局变量
	std::cout << ".data" << std::endl;
	Visit(program.values);
	// 访问所有函数
	std::cout << ".text" << std::endl;
	Visit(program.funcs);
}

// 访问 raw slice
void Visit(const koopa_raw_slice_t &slice)
{
	for (size_t i = 0; i < slice.len; ++i)
	{
		auto ptr = slice.buffer[i];
		// 根据 slice 的 kind 决定将 ptr 视作何种元素
		switch (slice.kind)
		{
		case KOOPA_RSIK_FUNCTION:
			// 访问函数
			Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
			break;
		case KOOPA_RSIK_BASIC_BLOCK:
			// 访问基本块
			Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
			break;
		case KOOPA_RSIK_VALUE:
			// 访问指令
			Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
			break;
		default:
			// 我们暂时不会遇到其他内容, 于是不对其做任何处理
			assert(false);
		}
	}
}

// 访问函数
void Visit(const koopa_raw_function_t &func)
{
	// skip declaration
	if (func->bbs.len == 0)
    	return;
	// .globl main
	std::cout << ".globl ";
	printf("%s\n", func->name + 1); // func->name is "@main", should ignore the @
	// main:
	printf("%s:\n", func->name + 1);

	// search over all command in IR, calculate required stack size
	auto &blocks = func->bbs;
	auto size = blocks.len;
	for (int i = 0; i < int(size); i++) {
		auto ptr = reinterpret_cast<koopa_raw_basic_block_t>(blocks.buffer[i]);
		auto &insts = ptr->insts;
		int insts_size = insts.len;
		for (int j = 0; j < int(insts_size); j++) {
			auto inst_ptr = reinterpret_cast<koopa_raw_value_t>(insts.buffer[j]);
			value_map[inst_ptr] = stack_size;
			stack_size += 4;
		}
	}

	// align to 16 (top)
	stack_size = (stack_size + ALIGN_NUM - 1) / ALIGN_NUM * ALIGN_NUM;
	if (stack_size <= 2048)
		std::cout << TAB << "addi sp, sp, " << std::to_string(-stack_size) << std::endl;
	// if stack size over 2048, 1. li t0, stacksize 2. add sp, sp, t0 (sp = sp + t0)
	else {
		std::cout << TAB << "li t0, " << std::to_string(-stack_size) << std::endl;
		std::cout << TAB << "add sp, sp, t0" << std::endl;
	}

	Visit(func->bbs);
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb)
{
	std::cout << (bb->name + 1) << ":" << std::endl;
	Visit(bb->insts);
}

// 访问指令
void Visit(const koopa_raw_value_t &value)
{
	// 根据指令类型判断后续需要如何访问
	const auto &kind = value->kind;
	switch (kind.tag)
	{
	case KOOPA_RVT_RETURN:
		// 访问 return 指令
		Visit(kind.data.ret);
		break;
	case KOOPA_RVT_INTEGER:
		// 访问 integer 指令
		Visit(kind.data.integer, value);
		break;
	case KOOPA_RVT_BINARY:
		Visit(kind.data.binary, value);
		break;
	// @a_1 = alloc i32 is just a decl, nothing needs to be done
	case KOOPA_RVT_ALLOC:
		break;
	case KOOPA_RVT_GLOBAL_ALLOC:
		break;
	case KOOPA_RVT_LOAD:
		Visit(kind.data.load, value);
		break;
	case KOOPA_RVT_STORE:
		Visit(kind.data.store, value);
		break;
	case KOOPA_RVT_BRANCH:
		Visit(kind.data.branch, value);
		break;
	case KOOPA_RVT_JUMP:
		Visit(kind.data.jump, value);
		break;
	default:
		assert(false);
	}
}

// visit return
void Visit(const koopa_raw_return_t &ret)
{
	// nullptr if return nothing;
	koopa_raw_value_t ret_val = ret.value;
	if (ret_val) {
		load(ret_val, "ret");	
	}
	if (stack_size <= 2048)
		std::cout << TAB << "addi sp, sp, " << std::to_string(stack_size) << std::endl;
	else {
		std::cout << TAB << "li t0, " << std::to_string(stack_size) << std::endl;
		std::cout << TAB << "addi sp, sp, t0" << std::endl;
	}
	std::cout << TAB << "ret" << std::endl;
}

// visit integer
void Visit(const koopa_raw_integer_t &integer, const koopa_raw_value_t &value)
{
	int int_val = integer.value;
	std::cout << int_val << std::endl;
}

// visit binary expression
void Visit(const koopa_raw_binary_t &binary, const koopa_raw_value_t &value)
{
	auto lhs = binary.lhs;
	auto op = binary.op;
	auto rhs = binary.rhs;
	
	load(lhs, "lhs");
	load(rhs, "rhs");

	// format for add, sub, mul, div, mod, and, or
	auto print_basic_binary = [&] (std::string op) {
		std::cout << TAB << op << " t0, t0, t1" << std::endl;
	};

	switch (op) {
		case KOOPA_RBO_ADD:
			print_basic_binary("add");
			break;
		case KOOPA_RBO_SUB:
			print_basic_binary("sub");
			break;
  		case KOOPA_RBO_MUL:
			print_basic_binary("mul");
			break;
  		case KOOPA_RBO_DIV:
			print_basic_binary("div");
			break;
  		case KOOPA_RBO_MOD:
			print_basic_binary("rem");
			break;
		case KOOPA_RBO_AND:
			print_basic_binary("and");
			break;
		case KOOPA_RBO_OR:
			print_basic_binary("or");
			break;
		case KOOPA_RBO_GT:
			print_basic_binary("sgt");
			break;
		case KOOPA_RBO_LT:
			print_basic_binary("slt");
			break;
		// if not ge, then must lt
		// if lt = 1, then ge = 0, so ge = (lt == 0)
		case KOOPA_RBO_GE:
			std::cout << "slt t0, t0, t1" << std::endl;
			std::cout << "seqz t0, t0" << std::endl;
			break;
		// if not le, then must gt
		// if gt = 1, then le = 0, so le = (gt == 0)
		case KOOPA_RBO_LE:
			std::cout << "sgt t0, t0, t1" << std::endl;
			std::cout << "seqz t0, t0" << std::endl;
			break;
		case KOOPA_RBO_EQ:
			std::cout << "xor t0, t0, t1" << std::endl;
			std::cout << "seqz t0, t0" << std::endl;
			break;
		case KOOPA_RBO_NOT_EQ:
			std::cout << "xor t0, t0, t1" << std::endl;
			std::cout << "snez t0, t0" << std::endl;
			break;
		default:
			assert(false);
	}
	// return exp value to t0
	// store t0 to stack_ptr of the binaryexp
	store(value, "binary");
}

void Visit(const koopa_raw_load_t &load_val, const koopa_raw_value_t &value) {
	auto src = load_val.src;
	load(src, "load");
	store(value, "load");
}


void Visit(const koopa_raw_store_t &store_val, const koopa_raw_value_t &value) {
	auto dest = store_val.dest;
	auto load_from = store_val.value;
    load(load_from, "store");
    store(dest, "store");
}

void Visit(const koopa_raw_branch_t &branch, const koopa_raw_value_t &value) {
	// cond is a value
	auto cond = branch.cond;

	// what are these two mfs?
	// auto false_args = branch.false_args;
	// auto true_args = branch.true_args;

	// two basic blocks, should print their name
	auto false_bb = branch.false_bb;
	auto true_bb = branch.true_bb;

	// 1. load cond into t0
	// 2. branch t0
    load(cond, "branch");
    std::cout << TAB << "beqz t0, " << (false_bb->name + 1) << std::endl;
    std::cout << TAB << "j " << (true_bb->name + 1) << std::endl;
}

void Visit(const koopa_raw_jump_t &jump, const koopa_raw_value_t &value) {
	// target basic block
	// auto args = jump.args;
	auto target = jump.target;
	std::cout << TAB << "j " << (target->name + 1) << std::endl;
}

void Visit(const koopa_raw_global_alloc_t &jump, const koopa_raw_value_t &value)
{
/*
    const koopa_raw_value_kind_t &kind = value->kind;
    auto &alloc = kind.data.global_alloc;
    auto &init = alloc.init;
    if (init) {
        load_value(init, "t0");
        store_value(value, "t0");
    } else {
        assert(false);
    }
    */
}
#define LOAD_COMMAND(reg) switch (kind.tag) {	\
			case KOOPA_RVT_INTEGER:	\
				std::cout << TAB << "li " << reg << ", " << kind.data.integer.value << std::endl;	\
				break;	\
			default:	\
				std::cout << TAB << "lw " << reg << ", " << stack_size - value_map[value] << "(sp)" << std::endl;	\
				break;	\
		}

// since we only store value on stack,
// only t0 is used to load temporary value from stack
// no register schedule!
// it seems at most 2 reg is used in every riscv command, so we use a macro
void load(const koopa_raw_value_t &value, std::string type) {
	const auto &kind = value->kind;
	if (type == "ret") {
		// LOAD_COMMAND("a0")
		switch (kind.tag) {
			// li t0, 10
			case KOOPA_RVT_INTEGER:
				std::cout << TAB << "li a0, " << kind.data.integer.value << std::endl;
				break;
			// else, load stack
			default:
				std::cout << TAB << "lw a0, " << stack_size - value_map[value] << "(sp)" << std::endl;
				break;
		}
	}
	else if (type == "lhs") {
		LOAD_COMMAND("t0");
	}
	else if (type == "rhs") {
		LOAD_COMMAND("t1");
	}
	else if (type == "load") {
		LOAD_COMMAND("t0");
	}
	else if (type == "store") {
		LOAD_COMMAND("t0");
	}
	else if (type == "branch") {
		LOAD_COMMAND("t0");
	}
	else {
		assert(false);
	}
}

#define STORE_COMMAND(reg) std::cout << TAB << "sw " << reg << ", " << stack_size - value_map[value] << "(sp)" << std::endl;

// no integer should be store, so no specific
void store(const koopa_raw_value_t &value, std::string type) {
	// const auto &kind = value->kind;
	if (type == "binary") {
		// std::cout << TAB << "sw t0, " << stack_size - value_map[value] << "(sp)" << std::endl;
		STORE_COMMAND("t0");
	}
	else if (type == "load") {
		STORE_COMMAND("t0");
	}
	else if (type == "store") {
		STORE_COMMAND("t0");
	}
	else {
		assert(false);
	}
}