#include <iostream>
#include "AST.h"

int ConstInitValAST::CalcInitVal() {
    return const_exp->CalcExpVal();
}

int InitValAST::CalcInitVal() {
    return exp->CalcExpVal();
}

int ConstExpAST::CalcExpVal() const {
    return exp->CalcExpVal();
}

int ExpAST::CalcExpVal() const {
    return lor_exp->CalcExpVal();
}

// PrimaryExpAST
int PrimaryExpAST::CalcExpVal() const {
    int res = -1;
    if (type == EXP){
        res = exp->CalcExpVal();
    }
    else if (type == NUMBER) {
        res = number;
    }
    else if (type == LVAL) {
        int idx = GETSCOPEIDX();
        std::string lval_ident = DERIVED_PTR(LValAST, lval)->ident;
        symbol_t symbol = symbol_table_vec[idx][lval_ident];
        if (const int* ptr = std::get_if<int>(&symbol) )
            res = std::get<int>(symbol);
        // if string, do not bother
    }
    return res;
}

// UnaryExpAST
int UnaryExpAST::CalcExpVal() const {
    if (type == PRIMARY) {
        int res = exp->CalcExpVal();
        return res;
    }
    else if (type == UNARY) {
        auto unary_val = exp->CalcExpVal();   // could be a variable(%1) or a number(6)
        int res = -1;
        switch(_OpHash(op)) {
            case eADD:   // no IR for add
                res = +unary_val;
                break;
            case eSUB:
                res = -unary_val;
                break;
            case eNOT:
                res = !unary_val;
                break;
            default:
                break;
        }
        return res;
    }
    return -1;
}

int MulExpAST::CalcExpVal() const {
    if (type == UNARY) {
        int res = unary_exp->CalcExpVal();
        return res;
    }
    else if (type == MUL) {
        int mul_val = mul_exp->CalcExpVal();   // could be a variable(%1) or a number(6)
        int unary_val = unary_exp->CalcExpVal();
        int res = -1;
        switch(_OpHash(op)) {
            case eMUL: 
                res = mul_val * unary_val;
                break;
            case eDIV:
                res = mul_val / unary_val;
                break;
            case eMOD:
                res = mul_val % unary_val;
                break;
            default:
                break;
        }
        return res;
    }
    return -1;
}

int AddExpAST::CalcExpVal() const {
    if (type == MUL) {
        int res = mul_exp->CalcExpVal();
        return res;
    }
    else if (type == ADD) {
        int add_val = add_exp->CalcExpVal();   // could be a variable(%1) or a number(6)
        int mul_val = mul_exp->CalcExpVal();
        int res = -1;
        switch(_OpHash(op)) {
            case eADD: 
                res = add_val + mul_val;
                break;
            case eSUB:
                res = add_val - mul_val;
                break;
            default:
                break;
        }
        return res;
    }
    return -1;
}

int RelExpAST::CalcExpVal() const {
    if (type == ADD) {
        int res = add_exp->CalcExpVal();
        return res;
    }
    else if (type == REL) {
        int rel_val = rel_exp->CalcExpVal();   // could be a variable(%1) or a number(6)
        int add_val = add_exp->CalcExpVal();
        int res = -1;
        switch(_OpHash(op)) {
            case eLESS: 
                res = (rel_val < add_val);
                break;
            case eGREAT:
                res = (rel_val > add_val);
                break;
            case eLEQ:
                res = (rel_val <= add_val);
                break;
            case eGEQ:
                res = (rel_val <= add_val);
                break;
            default:
                break;
        }
        return res;
    }
    return -1;
}


int EqExpAST::CalcExpVal() const {
    if (type == REL) {
        int res = rel_exp->CalcExpVal();
        return res;
    }
    else if (type == EQ) {
        int rel_val = rel_exp->CalcExpVal();   // could be a variable(%1) or a number(6)
        int eq_val = eq_exp->CalcExpVal();
        int res = -1;
        switch(_OpHash(op)) {
            case eEQ: 
                res = (rel_val == eq_val);
                break;
            case eNEQ:
                res = (rel_val != eq_val);
                break;
            default:
                break;
        }
        return res;
    }
    return -1;
}

int LAndExpAST::CalcExpVal() const {
    if (type == EQ) {
        int res = eq_exp->CalcExpVal();
        return res;
    }
    else if (type == LAND) {
        int land_val = land_exp->CalcExpVal();   // could be a variable(%1) or a number(6)
        int eq_val = eq_exp->CalcExpVal();
        int res = land_val && eq_val;
        return res;
    }
    return -1;
}

int LOrExpAST::CalcExpVal() const {
    if (type == LAND) {
        int res = land_exp->CalcExpVal();
        return res;
    }
    else if (type == LOR) {
        int lor_val = lor_exp->CalcExpVal();
        int land_val = land_exp->CalcExpVal();
        int res = lor_val || land_val;
        return res;
    }
    return -1;
}