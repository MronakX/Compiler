#pragma once

#include <iostream>

// 所有 AST 的基类
class BaseAST {
public:
    virtual ~BaseAST() = default;

    virtual void Dump2RawAST() const = 0;
    virtual void Dump2KooPa() const = 0;
    };

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
public:
    // 用智能指针管理对象
    std::unique_ptr<BaseAST> func_def;

    void Dump2RawAST() const override {
        std::cout << "CompUnitAST { ";
        func_def->Dump2RawAST();
        std::cout << " }";
    }

    void Dump2KooPa() const override {
        // std::cout << "CompUnitAST { ";
        func_def->Dump2KooPa();
        // std::cout << " }";
    }

};


// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;

    void Dump2RawAST() const override {
        std::cout << "FuncDefAST { ";
        func_type->Dump2RawAST();
        std::cout << ", " << ident << ", ";
        block->Dump2RawAST();
        std::cout << " }";
    }

    void Dump2KooPa() const override {
        std::cout << "fun @" << ident;
        func_type->Dump2KooPa();
        std::cout << " {" << std::endl;
        block->Dump2KooPa();
        std::cout << "}";
    }
};
// ...


class FuncTypeAST : public BaseAST {
public:
    std::string ident;

    void Dump2RawAST() const override {
        std::cout << "FuncTypeAST { " << ident << " }";
    }

    void Dump2KooPa() const override {
        std::cout << "(): ";
        if (true)
            std::cout << "i32";
        else
            std::cout << ident;
    }
};

class BlockAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> statement;

    void Dump2RawAST() const override {
        std::cout << "BlockAST { ";
        statement->Dump2RawAST();
        std::cout << " }";
    }

    void Dump2KooPa() const override {
        std::cout << "%entry :" << std::endl;
        statement->Dump2KooPa();
    }
};

class StmtAST : public BaseAST {
public:
    // std::unique_ptr<BaseAST> statement;
    int number;

    void Dump2RawAST() const override {
        std::cout << "StmtAST { " << number << " }" ;
    }

    void Dump2KooPa() const override {
        std::cout << "ret " << number << std::endl;
    }
};

// number is JUST the darn number, so no 'NumberAST' is required