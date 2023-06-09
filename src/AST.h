#pragma once

#include <iostream>

// global count for variables.
inline int var_cnt = 0;

#define TAB "  "

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

    std::string ExpDump2KooPa() const override {
        auto cur_var = lor_exp->ExpDump2KooPa();
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
        std::cout << TAB << "ret ";
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
        return "";
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
            switch(_OpHash(op)) {
                case eADD:   // no IR for add
                    // break;
                    var_cnt--;
                    return prev_var;
                case eSUB:
                    std::cout << TAB << cur_var << " = sub 0, " << prev_var << std::endl;
                    break;
                case eNOT:
                    std::cout << TAB << cur_var << " = eq " << prev_var << ", 0" << std::endl;
                    break;
                default:
                    break;
            }
            return cur_var;
        }
        return "";
    }
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

    std::string ExpDump2KooPa() const override {
        if (type == UNARY) {
            auto cur_var = unary_exp->ExpDump2KooPa();
            return cur_var;
        }
        else if (type == MUL) {
            auto mul_var = mul_exp->ExpDump2KooPa();   // could be a variable(%1) or a number(6)
            auto unary_var = unary_exp->ExpDump2KooPa();
            std::string cur_var = "%" + std::to_string(var_cnt++);
            std::string op_letter;
            switch(_OpHash(op)) {
                case eMUL: 
                    op_letter = " = mul ";
                    // std::cout << TAB << cur_var << " = mul " << mul_var << ", " << unary_var << std::endl;
                    break;
                case eDIV:
                    op_letter = " = div ";
                    break;
                case eMOD:
                    op_letter = " = mod ";
                    break;
                default:
                    break;
            }
            // std::cout << "in mulexp" << std::endl;
            std::cout << TAB << cur_var << op_letter << mul_var << ", " << unary_var << std::endl;
            return cur_var;
        }
        return "";
    }
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

    std::string ExpDump2KooPa() const override {
        if (type == MUL) {
            auto cur_var = mul_exp->ExpDump2KooPa();
            return cur_var;
        }
        else if (type == ADD) {
            auto add_var = add_exp->ExpDump2KooPa();   // could be a variable(%1) or a number(6)
            auto mul_var = mul_exp->ExpDump2KooPa();
            std::string cur_var = "%" + std::to_string(var_cnt++);
            std::string op_letter;
            switch(_OpHash(op)) {
                case eADD: 
                    op_letter = " = add ";
                    // std::cout << TAB << cur_var << " = mul " << mul_var << ", " << unary_var << std::endl;
                    break;
                case eSUB:
                    op_letter = " = sub ";
                    break;
                default:
                    break;
            }
            // std::cout << "in addexp" << std::endl;
            std::cout << TAB << cur_var << op_letter << add_var << ", " << mul_var << std::endl;
            return cur_var;
        }
        return "";
    }
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

    std::string ExpDump2KooPa() const override {
        if (type == ADD) {
            auto cur_var = add_exp->ExpDump2KooPa();
            return cur_var;
        }
        else if (type == REL) {
            auto rel_var = rel_exp->ExpDump2KooPa();   // could be a variable(%1) or a number(6)
            auto add_var = add_exp->ExpDump2KooPa();
            std::string cur_var = "%" + std::to_string(var_cnt++);
            std::string op_letter;
            // switch dont support std::string... stupid implement
            switch(_OpHash(op)) {
                case eLESS: 
                    op_letter = " = lt ";
                    // std::cout << TAB << cur_var << " = mul " << mul_var << ", " << unary_var << std::endl;
                    break;
                case eGREAT:
                    op_letter = " = gt ";
                    break;
                case eLEQ:
                    op_letter = " = le ";
                    break;
                case eGEQ:
                    op_letter = " = ge ";
                    break;
                default:
                    break;
            }
            std::cout << TAB << cur_var << op_letter << rel_var << ", " << add_var << std::endl;
            return cur_var;
        }
        return "";
    }
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

    std::string ExpDump2KooPa() const override {
        if (type == REL) {
            auto cur_var = rel_exp->ExpDump2KooPa();
            return cur_var;
        }
        else if (type == EQ) {
            auto rel_var = rel_exp->ExpDump2KooPa();   // could be a variable(%1) or a number(6)
            auto eq_var = eq_exp->ExpDump2KooPa();
            std::string cur_var = "%" + std::to_string(var_cnt++);
            std::string op_letter;
            // switch dont support std::string... stupid implement
            switch(_OpHash(op)) {
                case eEQ: 
                    op_letter = " = eq ";
                    // std::cout << TAB << cur_var << " = mul " << mul_var << ", " << unary_var << std::endl;
                    break;
                case eNEQ:
                    op_letter = " = ne ";
                    break;
                default:
                    break;
            }
            std::cout << TAB << cur_var << op_letter << eq_var << ", " << rel_var << std::endl;
            return cur_var;
        }
        return "";
    }
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

    std::string ExpDump2KooPa() const override {
        if (type == EQ) {
            auto cur_var = eq_exp->ExpDump2KooPa();
            return cur_var;
        }
        else if (type == LAND) {
            auto land_var = land_exp->ExpDump2KooPa();   // could be a variable(%1) or a number(6)
            auto eq_var = eq_exp->ExpDump2KooPa();
            std::string tmp_land_var = "%" + std::to_string(var_cnt++);
            std::string tmp_eq_var = "%" + std::to_string(var_cnt++);
            std::string cur_var = "%" + std::to_string(var_cnt++);
            // %0 = (land_var ne 0)
            // %1 = (eq_var ne 0)
            // %2 = (%0 and %1), 'and' here is bit_and. 
            // %2 is true iff. (landvar != 0 and eqvar != 0), %1 = 1 and %2 = 1
            switch(_OpHash(op)) {
                case eLAND: 
                    // TAB %1 = ne %0, 0\n
                    // TAB %3 = and %1, %2\n
                    std::cout << TAB << tmp_land_var << " = ne " << land_var << ", 0" << std::endl;
                    std::cout << TAB << tmp_eq_var << " = ne " << eq_var << ", 0" << std::endl;
                    std::cout << TAB << cur_var << " = and " << tmp_land_var << ", " << tmp_eq_var << std::endl;
                    break;
                default:
                    break;
            }
            return cur_var;
        }
        return "";
    }
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

    std::string ExpDump2KooPa() const override {
        if (type == LAND) {
            auto cur_var = land_exp->ExpDump2KooPa();
            return cur_var;
        }
        else if (type == LOR) {
            auto lor_var = lor_exp->ExpDump2KooPa();   // could be a variable(%1) or a number(6)
            auto land_var = land_exp->ExpDump2KooPa();
            std::string tmp_or_var = "%" + std::to_string(var_cnt++);
            std::string cur_var = "%" + std::to_string(var_cnt++);
            // %0 = (lor_var or %land_var), 'and' here is bit_or. 
            // %1 = (%0 != 0)
            // %1 is true iff. (landvar != 0 or eqvar != 0), %0 = 1 (1 != 0)
            switch(_OpHash(op)) {
                case eLOR: 
                    std::cout << TAB << tmp_or_var << " = or " << lor_var << ", " << land_var << std::endl;
                    std::cout << TAB << cur_var << " = ne " << tmp_or_var << ", 0" << std::endl;
                    break;
                default:
                    break;
            }
            return cur_var;
        }
        return "";
    }
};