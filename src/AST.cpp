#include <iostream>
#include "AST.h"

// CompUnitAST
void CompUnitAST::Dump2KooPa() const {
    // std::cout << "CompUnitAST { ";
    func_def->Dump2KooPa();
    // std::cout << " }";
}

// FuncDefAST
void FuncDefAST::Dump2KooPa() const {
    std::cout << "fun @" << ident;
    func_type->Dump2KooPa();
    std::cout << " {" << std::endl;
    block->Dump2KooPa();
    std::cout << "}";
}


// FuncTypeAST
void FuncTypeAST::Dump2KooPa() const {
    std::cout << "(): ";
    if (true)
        std::cout << "i32";
    else
        std::cout << ident;
}


// BlockAST
void BlockAST::Dump2KooPa() const {
    std::cout << "%entry :" << std::endl;
    statement->Dump2KooPa();
}


// ExpAST
std::string ExpAST::ExpDump2KooPa() const {
    auto cur_var = lor_exp->ExpDump2KooPa();
    return cur_var;
}


// StmtAST
void StmtAST::Dump2KooPa() const {
    // std::cout << "ret ";
    auto cur_var = exp->ExpDump2KooPa();
    std::cout << TAB << "ret ";
    std::cout << cur_var << std::endl;
}

// PrimaryExpAST
std::string PrimaryExpAST::ExpDump2KooPa() const {
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

// UnaryExpAST
std::string UnaryExpAST::ExpDump2KooPa() const {
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

std::string MulExpAST::ExpDump2KooPa() const {
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

std::string AddExpAST::ExpDump2KooPa() const {
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


std::string RelExpAST::ExpDump2KooPa() const {
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

std::string EqExpAST::ExpDump2KooPa() const {
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


std::string LAndExpAST::ExpDump2KooPa() const {
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

std::string LOrExpAST::ExpDump2KooPa() const {
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