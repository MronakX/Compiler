#include <iostream>
#include "AST.h"

// CompUnitAST
void CompUnitAST::Dump2KooPa() const {
    // std::cout << "CompUnitAST { ";

    symbol_table_t global_symbol_table;
    symbol_table_vec.push_back(global_symbol_table);

    func_def->Dump2KooPa();
    // std::cout << " }";
    symbol_table_vec.pop_back();
}


// FuncDefAST
void FuncDefAST::Dump2KooPa() const {
    // in koopa, no duplicate symbol_name in each function
    // so whenever we change function, reset var_cnt & ident_cnt_map
    std::map<std::string, int> prev_ident_cnt_map = ident_cnt_map;
    int prev_var_cnt = var_cnt;

    symbol_table_t func_symbol_table;
    symbol_table_vec.push_back(func_symbol_table);

    std::cout << "fun @" << ident;
    func_type->Dump2KooPa();
    std::cout << " {" << std::endl;
    std::cout << "%entry :" << std::endl;
    block->Dump2KooPa();
    std::cout << "}";

    var_cnt = prev_var_cnt;
    ident_cnt_map = prev_ident_cnt_map; // backtrace when changing scope
    symbol_table_vec.pop_back();
}


// FuncTypeAST
void FuncTypeAST::Dump2KooPa() const {
    std::cout << "(): ";
    if (true)
        std::cout << "i32";
    else
        std::cout << ident;
}

void BTypeAST::Dump2KooPa() const {
    return;
}

void LValAST::Dump2KooPa() const {
    return;
}



// DeclAST
void DeclAST::Dump2KooPa() const {
    // std::unique_ptr<ConstDeclAST> const_decl_ptr(dynamic_cast<ConstDeclAST*>(const_decl.get()));
    // const_decl_ptr->constdeclfunc();

    if (type == CONST) {
        const_decl->Dump2KooPa();
    }
    else if (type == VAR) { 
        var_decl->Dump2KooPa();
    }

}

void ConstDeclAST::Dump2KooPa() const {
    int size = const_def_vec.size();
    for (int i = 0; i < size; i++) {
        const_def_vec[i]->Dump2KooPa();
    }
}


void ConstDefAST::Dump2KooPa() const {
    int idx = GETSCOPEIDX();
    symbol_table_vec[idx][ident] = DERIVED_PTR(ConstInitValAST, const_init_val)->CalcInitVal();
}

void ConstInitValAST::Dump2KooPa() const {
    return;
}

void InitValAST::Dump2KooPa() const {
    return;
}


void VarDeclAST::Dump2KooPa() const {
    int size = var_def_vec.size();
    for (int i = 0; i < size; i++) {
        var_def_vec[i]->Dump2KooPa();
    }
}

std::string InitValAST::ExpDump2Koopa() {
    std::string ret;
    ret = exp->ExpDump2KooPa();
    return ret;
}

void VarDefAST::Dump2KooPa() const {
    //  @x = alloc i32
    int ident_cnt = ++ident_cnt_map[ident];
    symbol_name_t symbol_name = "@" + ident + "_" + std::to_string(ident_cnt);
    std::cout << TAB << symbol_name << " = alloc i32" << std::endl;
    int idx = GETSCOPEIDX();
    symbol_table_vec[idx][ident] = symbol_name;
    if (type == INIT_VAL) {
        std::string ret_val = DERIVED_PTR(InitValAST, init_val)->ExpDump2Koopa();
        std::cout << TAB << "store " << ret_val << ", " << symbol_name << std:: endl;
    }
}


// BlockAST
void BlockAST::Dump2KooPa() const {
    // each block has only ONE scope, NOT scope per blockitem
    symbol_table_t symbol_table;
    symbol_table_vec.push_back(symbol_table);
    int size = block_item_vec.size();
    for (int i = 0; i < size; i++) {
        block_item_vec[i]->Dump2KooPa();
    }

    symbol_table_vec.pop_back();
}

// BlockItem
void BlockItemAST::Dump2KooPa() const {
    if (type == DECL) {
        decl->Dump2KooPa();
    }
    else if (type == STMT) {
        stmt->Dump2KooPa();
    }
}

// ExpAST
std::string ExpAST::ExpDump2KooPa() const {
    auto cur_var = lor_exp->ExpDump2KooPa();
    return cur_var;
}


// StmtAST
void StmtAST::Dump2KooPa() const {
    // std::cout << "ret ";
    if (type == RET) {
        auto cur_var = exp->ExpDump2KooPa();
        std::cout << TAB << "ret ";
        std::cout << cur_var << std::endl;
    }
    else if (type == EMPTY_RET) {
        std::cout << TAB << "ret" << std::endl;
    }
    else if (type == LVAL) {
        std::string lval_ident = DERIVED_PTR(LValAST, lval)->ident;
        // search bottom-up to find the closest variable.
        symbol_t symbol;
        int scope_num = symbol_table_vec.size();
        for (int i = scope_num - 1; i >= 0; i--) {
            if (symbol_table_vec[i].count(lval_ident) != 0) {
                symbol = symbol_table_vec[i][lval_ident];
                break;
            }
        }
        if (const int* ptr = std::get_if<int>(&symbol) ) {
            // should not be integer..
            std::string res = std::to_string(std::get<int>(symbol));
            // return res;
        }
        else if (const std::string* ptr = std::get_if<std::string>(&symbol)) {
            std::string res = std::get<std::string>(symbol);
            std::string cur_var = exp->ExpDump2KooPa();
            std::cout << TAB << "store " << cur_var << ", " << res << std::endl;
        }
    }
    else if (type == EXP) {
        exp->Dump2KooPa();
    }
    else if (type == EMPTY_EXP) {
        // do nothing
    }
    else if (type == BLOCK) {
        block->Dump2KooPa();
    }
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
    else if (type == LVAL) {
        std::string lval_ident = DERIVED_PTR(LValAST, lval)->ident;
        symbol_t symbol;
        int scope_num = symbol_table_vec.size();
        for (int i = scope_num - 1; i >= 0; i--) {
            if (symbol_table_vec[i].count(lval_ident) != 0) {
                symbol = symbol_table_vec[i][lval_ident];
                break;
            }
        }
        if (const int* ptr = std::get_if<int>(&symbol) ) {
            auto res = std::to_string(std::get<int>(symbol));
            return res;
        }
        else if (const std::string* ptr = std::get_if<std::string>(&symbol)) {
            std::string res = std::get<std::string>(symbol);
            std::string cur_var = "%" + std::to_string(var_cnt++);
            symbol_name_t symbol_name = std::get<std::string>(symbol); 
            std::cout << "  " << cur_var << " = load " << symbol_name << std::endl;
            return cur_var;
        }
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


// RelExpAST
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


// EqExpAST
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


// LAndExpAST
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

// LOrExpAST
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
