#pragma once

#include <iostream>
#include <memory>

// global count for variables.
inline int var_cnt = 0;

#define TAB "  "

// 所有 AST 的基类
class BaseAST {
public:
    virtual ~BaseAST() = default;
    virtual void Dump2KooPa() const = 0;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
public:
    // 用智能指针管理对象
    std::unique_ptr<BaseAST> func_def;
    void Dump2KooPa() const override;

};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;

    void Dump2KooPa() const override;
};
// ...


class FuncTypeAST : public BaseAST {
public:
    std::string ident;

    void Dump2KooPa() const override;
};

class BlockAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> statement;

    void Dump2KooPa() const override;
};


class ExpBaseAST {
public:
    virtual ~ExpBaseAST() = default;
    virtual std::string ExpDump2KooPa() const = 0;

    enum OpHashCode {
        eADD,
        eSUB,
        eNOT,
        eMUL,
        eDIV,
        eMOD,
        eLESS,
        eGREAT,
        eLEQ,
        eGEQ,
        eEQ,
        eNEQ,
        eLAND,
        eLOR
    };

    OpHashCode _OpHash (std::string const& op_mark) const {
    if (op_mark == "+") return eADD;
    if (op_mark == "-") return eSUB;
    if (op_mark == "!") return eNOT;
    if (op_mark == "*") return eMUL;
    if (op_mark == "/") return eDIV;
    if (op_mark == "%") return eMOD;
    if (op_mark == "<") return eLESS;
    if (op_mark == ">") return eGREAT;
    if (op_mark == "<=") return eLEQ;
    if (op_mark == ">=") return eGEQ;
    if (op_mark == "==") return eEQ;
    if (op_mark == "!=") return eNEQ;
    if (op_mark == "&&") return eLAND;
    if (op_mark == "||") return eLOR;
    
    return eADD;
}
};

class ExpAST : public ExpBaseAST{
    public:
    std::unique_ptr<ExpBaseAST> lor_exp;

    std::string ExpDump2KooPa() const override;
};

class StmtAST : public BaseAST {
public:
    // std::unique_ptr<BaseAST> statement;
    // int number;
    std::unique_ptr<ExpBaseAST> exp;

    void Dump2KooPa() const override;
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
    std::string ExpDump2KooPa() const override;
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
    std::string ExpDump2KooPa() const override;
};

class MulExpAST : public ExpBaseAST{
    public:

    enum Type {
        UNARY,
        MUL
    } type;
    std::unique_ptr<ExpBaseAST> unary_exp;
    std::unique_ptr<ExpBaseAST> mul_exp;
    std::string op;

    std::string ExpDump2KooPa() const override;
};

class AddExpAST : public ExpBaseAST{
    public:

    enum Type {
        MUL,
        ADD
    } type;
    std::unique_ptr<ExpBaseAST> mul_exp;
    std::unique_ptr<ExpBaseAST> add_exp;
    std::string op;

    std::string ExpDump2KooPa() const override;
};

class RelExpAST : public ExpBaseAST{
    public:

    enum Type {
        ADD,
        REL
    } type;
    std::unique_ptr<ExpBaseAST> add_exp;
    std::unique_ptr<ExpBaseAST> rel_exp;
    std::string op;

    std::string ExpDump2KooPa() const override;
};

class EqExpAST : public ExpBaseAST{
    public:

    enum Type {
        REL,
        EQ
    } type;
    std::unique_ptr<ExpBaseAST> rel_exp;
    std::unique_ptr<ExpBaseAST> eq_exp;
    std::string op;

    std::string ExpDump2KooPa() const override;
};

class LAndExpAST : public ExpBaseAST{
    public:

    enum Type {
        EQ,
        LAND
    } type;
    std::unique_ptr<ExpBaseAST> eq_exp;
    std::unique_ptr<ExpBaseAST> land_exp;
    std::string op;

    std::string ExpDump2KooPa() const override;
};

class LOrExpAST : public ExpBaseAST{
    public:

    enum Type {
        LAND,
        LOR
    } type;
    std::unique_ptr<ExpBaseAST> land_exp;
    std::unique_ptr<ExpBaseAST> lor_exp;
    std::string op;

    std::string ExpDump2KooPa() const override;
};