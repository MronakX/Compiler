#pragma once

#include <iostream>

// 所有 AST 的基类
class BaseAST {
public:
    virtual ~BaseAST() = default;

    virtual void Dump() const = 0;
    };

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
public:
    // 用智能指针管理对象
    std::unique_ptr<BaseAST> func_def;
    void Dump() const override {
    std::cout << "CompUnitAST { ";
    func_def->Dump();
    std::cout << " }";
    }
};


// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;

    void Dump() const override {
        std::cout << "FuncDefAST { ";
        func_type->Dump();
        std::cout << ", " << ident << ", ";
        block->Dump();
        std::cout << " }";
    }
};
// ...


class FuncTypeAST : public BaseAST {
public:
    std::string ident;

    void Dump() const override {
        std::cout << "FuncTypeAST { " << ident << " }";
    }
};

class BlockAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> statement;

    void Dump() const override {
        std::cout << "BlockAST { ";
        statement->Dump();
        std::cout << " }";
    }
};

class StmtAST : public BaseAST {
public:
    // std::unique_ptr<BaseAST> statement;
    int number;

    void Dump() const override {
        std::cout << "StmtAST { " << number << " }" ;
    }
};

// number is JUST the darn number, so no 'NumberAST' is required