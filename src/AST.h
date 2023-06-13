#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include "type.h"

// global count for variables.
inline int var_cnt = 0;
inline int if_cnt = 0;
inline int dummy_cnt = 0;

#define TAB "  "

#define DERIVED_PTR(derived_class, src_ptr)  \
    dynamic_cast<derived_class*>(src_ptr.get())

#define GETSCOPEIDX() symbol_table_vec.size() - 1

inline std::vector<symbol_table_t> symbol_table_vec;
inline std::map<std::string, int> ident_cnt_map;  //for dupicate symbol_name (inside diffrent func scope)


// 所有 AST 的基类
class BaseAST {
public:
    virtual ~BaseAST() = default;

    // virtual std::string Dump2RawAST() const = 0;
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

class FuncTypeAST : public BaseAST {
public:
    std::string ident;

    void Dump2KooPa() const override;
};

class BTypeAST : public BaseAST {
public:
    std::string ident;

    void Dump2KooPa() const override;
};

class LValAST : public BaseAST {
public:
    std::string ident;

    void Dump2KooPa() const override;
};

class DeclAST : public BaseAST {
public:
    enum Type {
        CONST,
        VAR
    } type;
    std::unique_ptr<BaseAST> const_decl;
    std::unique_ptr<BaseAST> var_decl;

    void Dump2KooPa() const override;
};


class ConstDeclAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> b_type;
    std::vector<std::unique_ptr<BaseAST> > const_def_vec;

    void Dump2KooPa() const override;
};

class ConstDefAST : public BaseAST {
public:
    std::string ident;
    std::unique_ptr<BaseAST> const_init_val;

    void Dump2KooPa() const override;
};


class BlockAST : public BaseAST {
public:
    std::vector<std::unique_ptr<BaseAST> > block_item_vec;

    void Dump2KooPa() const override;
};

class BlockItemAST : public BaseAST {
    public:
    enum Type {
        DECL,
        STMT,
    } type;
    std::unique_ptr<BaseAST> decl;
    std::unique_ptr<BaseAST> stmt;

    void Dump2KooPa() const override;
};

class ExpBaseAST : public BaseAST {
public:

    void Dump2KooPa() const override {
    }

    virtual ~ExpBaseAST() = default;
    virtual std::string ExpDump2KooPa() const = 0;
    virtual int CalcExpVal() const = 0;

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

class ConstInitValAST : public BaseAST {
public:
    std::unique_ptr<ExpBaseAST> const_exp;

    void Dump2KooPa() const override;
    
    int CalcInitVal();
};

class VarDeclAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> b_type;
    std::vector<std::unique_ptr<BaseAST> > var_def_vec;

    void Dump2KooPa() const override;
};

class VarDefAST : public BaseAST {
    public:
    enum Type {
        iDENT,
        INIT_VAL
    } type;

    std::string ident;
    std::unique_ptr<BaseAST> init_val;

    void Dump2KooPa() const override;
};

class InitValAST : public BaseAST {
public:
    std::unique_ptr<ExpBaseAST> exp;

    void Dump2KooPa() const override;
    std::string ExpDump2Koopa();
    int CalcInitVal();
};

class ConstExpAST : public ExpBaseAST {
    public:
    std::unique_ptr<ExpBaseAST> exp;

    // const exp do not require
    // std::string ExpDump2KooPa() const override;
    std::string ExpDump2KooPa() const override {
        return "";
    }

    int CalcExpVal() const override;
};

class ExpAST : public ExpBaseAST{
    public:
    std::unique_ptr<ExpBaseAST> lor_exp;

    std::string ExpDump2KooPa() const override;

    int CalcExpVal() const override;
};

class IfElseStmtAST : public BaseAST {
public:
    enum Type {
        IF,
        IF_ELSE,
    } type;
    std::unique_ptr<ExpBaseAST> if_exp;
    std::unique_ptr<BaseAST> if_stmt;
    std::unique_ptr<BaseAST> else_stmt;

    void Dump2KooPa() const override;
};

class BasicStmtAST : public BaseAST {
public:
    enum Type {
        LVAL,
        EXP,
        BLOCK,
        RET,
        EMPTY_RET,
        EMPTY_EXP,
    } type;
    std::unique_ptr<BaseAST> lval;
    std::unique_ptr<ExpBaseAST> exp;
    std::unique_ptr<BaseAST> block;
    void Dump2KooPa() const override;
};

// number is JUST the goddarn number, so no 'NumberAST' is required

// PrimaryExp  ::= "(" Exp ")" | Number;
class PrimaryExpAST : public ExpBaseAST {
    public:
    enum Type {
        EXP, 
        NUMBER,
        LVAL,
    } type;
    std::unique_ptr<ExpBaseAST> exp;
    std::unique_ptr<BaseAST> lval;
    int number;
    std::string ExpDump2KooPa() const override;
    int CalcExpVal() const override;
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
    int CalcExpVal() const override;
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
    int CalcExpVal() const override;
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
    int CalcExpVal() const override;
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
    int CalcExpVal() const override;
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
    int CalcExpVal() const override;
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
    int CalcExpVal() const override;
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
    int CalcExpVal() const override;
};