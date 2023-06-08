#pragma once

#include "AST.h"
#include "koopa.h"

void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_slice_t &slice);
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_value_t &value);

void Visit(const koopa_raw_return_t &ret);
void Visit(const koopa_raw_integer_t &integer);

void Visit(const koopa_raw_program_t &program) {
	std::cout << ".text" << std::endl;
	// 访问所有全局变量
	Visit(program.values);
	// 访问所有函数
	Visit(program.funcs);
}


// 访问 raw slice
void Visit(const koopa_raw_slice_t &slice) {
  	for (size_t i = 0; i < slice.len; ++i) {
    	auto ptr = slice.buffer[i];
    	// 根据 slice 的 kind 决定将 ptr 视作何种元素
    	switch (slice.kind) {
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
void Visit(const koopa_raw_function_t &func) {
	// .globl main
  	std::cout << ".globl ";
  	printf("%s\n", func->name + 1);	//func->name is "@main", should ignore the @
	// main:
	printf("%s:\n", func->name + 1);
  	Visit(func->bbs);
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb) {
	Visit(bb->insts);
}

// 访问指令
void Visit(const koopa_raw_value_t &value) {
  // 根据指令类型判断后续需要如何访问
  const auto &kind = value->kind;
  switch (kind.tag) {
    case KOOPA_RVT_RETURN:
      // 访问 return 指令
      Visit(kind.data.ret);
      break;
    case KOOPA_RVT_INTEGER:
      // 访问 integer 指令
      Visit(kind.data.integer);
      break;
    default:
      // 其他类型暂时遇不到
      assert(false);
  }
}

// 访问对应类型指令的函数定义略
// 视需求自行实现
// ...

void Visit(const koopa_raw_return_t &ret) {
	auto ret_val = ret.value;
	std::cout << "li a0, ";
	Visit(ret_val);

	std::cout << "ret" << std::endl;
}

void Visit(const koopa_raw_integer_t &integer) {
	int int_val = integer.value;
	std::cout << int_val << std::endl;
}