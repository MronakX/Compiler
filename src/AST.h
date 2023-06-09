#pragma once

#include <iostream>

// global count for variables.
inline int var_cnt = 0;

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


class ExpBaseAST {
public:
    virtual ~ExpBaseAST() = default;
    virtual std::string ExpDump2KooPa() const = 0;
};

class ExpAST : public ExpBaseAST{
    public:
    std::unique_ptr<ExpBaseAST> exp;

    std::string ExpDump2KooPa() const override {
        auto cur_var = exp->ExpDump2KooPa();
        return cur_var;
    }
};

class StmtAST : public BaseAST {
public:
    // std::unique_ptr<BaseAST> statement;
    // int number;
    std::unique_ptr<ExpBaseAST> exp;

    void Dump2RawAST() const override {
    }

    void Dump2KooPa() const override {
        // std::cout << "ret ";
        auto cur_var = exp->ExpDump2KooPa();
        std::cout << "ret ";
        std::cout << cur_var << std::endl;
    }
};

// number is JUST the goddarn number, so no 'NumberAST' is required


// PrimaryExp  ::= "(" Exp ")" | Number;
class PrimaryExpAST : public ExpBaseAST {
    public:
    enum Type {
        EXP, 
        NUMBER,
    } type;
    std::unique_ptr<ExpBaseAST> exp;
    int number;
    std::string ExpDump2KooPa() const override {
        if (type == EXP){
            auto cur_var = exp->ExpDump2KooPa();
            return cur_var;
        }
        else if (type == NUMBER) {
            auto cur_var = std::to_string(number);
            return cur_var;
        }
    }
};

// UnaryExp    ::= PrimaryExp | UnaryOp UnaryExp;
// UnaryOp is just like a number,  just a single mark, so no AST definition is required
class UnaryExpAST : public ExpBaseAST {
public:
    enum Type {
        PRIMARY,
        UNARY
    } type;
    std::unique_ptr<ExpBaseAST> exp;
    std::string op;
    std::string ExpDump2KooPa() const override {
        if (type == PRIMARY) {
            auto cur_var = exp->ExpDump2KooPa();
            return cur_var;
        }
        else if (type == UNARY) {
            auto prev_var = exp->ExpDump2KooPa();   // could be a variable(%1) or a number(6)
            std::string cur_var = "%" + std::to_string(var_cnt++);
            switch(op[0]) {
                case '+':   // no IR for add
                    // break;
                    var_cnt--;
                    return prev_var;
                case '-':
                    std::cout << cur_var << " = sub 0, " << prev_var << std::endl;
                    break;
                case '!':
                    std::cout << cur_var << " = eq " << prev_var << ", 0" << std::endl;
                    break;
                default:
                    break;
            }
            return cur_var;
        }
    }
};

