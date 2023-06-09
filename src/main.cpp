#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>

#include "AST.h"
#include "IR.h"

#include <stdio.h>
using namespace std;

#define DEBUG
// 声明 lexer 的输入, 以及 parser 函数
// 为什么不引用 sysy.tab.hpp 呢? 因为首先里面没有 yyin 的定义
// 其次, 因为这个文件不是我们自己写的, 而是被 Bison 生成出来的
// 你的代码编辑器/IDE 很可能找不到这个文件, 然后会给你报错 (虽然编译不会出错)
// 看起来会很烦人, 于是干脆采用这种看起来 dirty 但实际很有效的手段
extern FILE *yyin;
int yyparse (std::unique_ptr<BaseAST> &ast, std::unique_ptr<ExpBaseAST> &exp_ast);

int main(int argc, const char *argv[]) {
    // 解析命令行参数. 测试脚本/评测平台要求你的编译器能接收如下参数:
    // compiler 模式 输入文件 -o 输出文件
    assert(argc == 5);
    string mode = string(argv[1]);
    auto input = argv[2];
    auto output = argv[4];
    
    // 打开输入文件, 并且指定 lexer 在解析的时候读取这个文件
    yyin = fopen(input, "r");
    assert(yyin);

// here is for console debug
// show raw input in console
#ifdef DEBUG
    char buf[1000];
    auto tmpin = fopen(input, "r");
    fread(buf, 100, 1, tmpin);
    std::cout << "(---------- Raw Input ----------)" << std::endl;
    printf("%s\n", buf);
#endif

    // 调用 parser 函数, parser 函数会进一步调用 lexer 解析输入文件的
    unique_ptr<BaseAST> ast;
    unique_ptr<ExpBaseAST> exp_ast;
    auto ret = yyparse(ast, exp_ast);
    assert(!ret);

    if (output && mode == "-riscv")
    {
        // 1. save koopaIR ast in koopa_ir(string)
        stringstream ast_ss;
        streambuf *stdout_buffer = cout.rdbuf();   //save stdout
        cout.rdbuf(ast_ss.rdbuf());
        ast->Dump2KooPa();
        string koopa_ir = ast_ss.str();
        cout.rdbuf(stdout_buffer);  //reset 

        // 2. parsing string
        // 解析字符串 str, 得到 Koopa IR 程序
        koopa_program_t program;
        koopa_error_code_t ret = koopa_parse_from_string(koopa_ir.c_str(), &program);
        assert(ret == KOOPA_EC_SUCCESS);  // 确保解析时没有出错
        // 创建一个 raw program builder, 用来构建 raw program
        koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
        // 将 Koopa IR 程序转换为 raw program
        koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
        // 释放 Koopa IR 程序占用的内存
        koopa_delete_program(program);

        // 处理 raw program
        freopen(output, "w", stdout); 
        Visit(raw);
        freopen("/dev/console", "w", stdout);

#ifdef DEBUG
        std::cout << "(---------- RISC-V ----------)" << std::endl;
        Visit(raw);
        cout << endl;

        koopa_delete_raw_program_builder(builder);
    }
#endif


    if(output && mode == "-koopa")
    {
        freopen(output, "w", stdout); 
        ast->Dump2KooPa();
        cout << endl;
        freopen("/dev/console", "w", stdout);
    }
// show koopaIR in console
#ifdef DEBUG
    std::cout << "(---------- KooPa IR ----------)" << std::endl;
    ast->Dump2KooPa();
    cout << endl;

#endif
    return 0;
}