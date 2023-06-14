#include <iostream>
#include "AST.h"
#include <ctype.h>
#include <algorithm>

void initialize_library_func() {
    std::cout << "decl @getint(): i32" << std::endl;
    std::cout << "decl @getch(): i32" << std::endl;
    std::cout << "decl @getarray(*i32): i32" << std::endl;
    std::cout << "decl @putint(i32)" << std::endl;
    std::cout << "decl @putch(i32)" << std::endl;
    std::cout << "decl @putarray(i32, *i32)" << std::endl;
    std::cout << "decl @starttime()" << std::endl;
    std::cout << "decl @stoptime()" << std::endl;

    func_table["getint"] = "int";
    func_table["getch"] = "int";
    func_table["getarray"] = "int";
    func_table["putint"] = "void";
    func_table["putch"] = "void";
    func_table["putarray"] = "void";
    func_table["starttime"] = "void";
    func_table["stoptime"] = "void";
}

void VarDefAST::GlobalDump2KooPa() {
    //  @x = alloc i32
    int ident_cnt = ++ident_cnt_map[ident];
    symbol_name_t symbol_name = "@" + ident + "_" + std::to_string(ident_cnt);
    int idx = GETSCOPEIDX();
    symbol_table_vec[idx][ident] = symbol_name;
    std::cout << "global " << symbol_name << " = alloc i32, ";
    // int x ---- global @var = alloc i32, zeroinit
    // OR 
    // int x = 1；(the rhs must be const expression)
    // ... store %1, 1;
    // global @var = alloc i32, zeroinit, 
    // store
    // OR
    // global @var = alloc i32, number (will this happen?)
    auto is_digit = [&] (const std::string& str) {
        return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
    };

    if (type == INIT_VAL) {
        std::string ret_val = DERIVED_PTR(InitValAST, init_val)->ExpDump2Koopa();
        if (ret_val == "0") {
            std::cout << "zeroinit" << std::endl;
        }
        else if (is_digit(ret_val)) {
            std::cout << ret_val << std::endl;
        }
        else {
            std::cout << "zeroinit" << std::endl;
            std::cout << TAB << "store " << ret_val << ", " << symbol_name << std::endl;
        }
    }
    else if (type == iDENT) {
        std::cout << "zeroinit" << std::endl;
    }
}

void VarDeclAST::GlobalDump2KooPa() {
    int size = var_def_vec.size();
    for (int i = 0; i < size; i++) {
        DERIVED_PTR(VarDefAST, var_def_vec[i])->GlobalDump2KooPa();
    }
}

void DeclAST::DeclInGlobal() {
    // if const, just store in symbol table.
    // since we always store const in the last scope, nothing more needs to be done.
    if (type == CONST) {
        const_decl->Dump2KooPa();
    }
    else if (type == VAR) {
        // this will store all non-const symbol into last symbol table
        // note that, non-const symbol is string, while const symbol is int32
        // so we could separte them with get<int>/get<string>
        DERIVED_PTR(VarDeclAST, var_decl)->GlobalDump2KooPa();
    }
}

// CompUnitAST
void CompUnitAST::Dump2KooPa() const {
    symbol_table_t global_symbol_table;
    symbol_table_vec.push_back(global_symbol_table);

    initialize_library_func();

    for (auto &it : decl_vec) {
        DERIVED_PTR(DeclAST, it)->DeclInGlobal();
    }
    
    for (auto &it : func_def_vec) {
        it->Dump2KooPa();
    }
    // std::cout << " }";
    symbol_table_vec.pop_back();
}

void allocate_function_params() {
    int idx = GETSCOPEIDX();
    auto cur_symbol_table = symbol_table_vec[idx];
    for (auto it : symbol_table_vec[idx]) {
        symbol_name_t ident = it.first;
        symbol_name_t at_symbol_name = "@" + ident + "_p";
        symbol_name_t percent_symbol_name = "%" + ident + "_p";
        std::string alloc_stmt = percent_symbol_name + " = alloc i32";
        std::string store_stmt = "store " + at_symbol_name + ", " + percent_symbol_name;
        // %x = alloc i32
        // store @x, %x
        std::cout << TAB << alloc_stmt << std::endl;
        std::cout << TAB << store_stmt << std::endl;
    }
}

// FuncDefAST
void FuncDefAST::Dump2KooPa() const {
    // in koopa, no duplicate symbol_name in each function
    // so whenever we change function, reset var_cnt & ident_cnt_map
    std::map<std::string, int> prev_ident_cnt_map = ident_cnt_map;
    int prev_var_cnt = var_cnt;
    int prev_if_cnt = if_cnt;
    int prev_dummy_cnt = dummy_cnt;
    
    symbol_table_t func_symbol_table;
    symbol_table_vec.push_back(func_symbol_table);

    // fun @main
    std::cout << "fun @" << ident;
    std::string func_type_ident = DERIVED_PTR(FuncTypeAST, func_type)->ident;
    func_table[ident] = func_type_ident;

    // () or (@x: i32, @y: i32)
    std::cout << "(";
    if (func_fparams){
        func_fparams->Dump2KooPa();
    }
    std::cout << ")";
    func_type->Dump2KooPa();
    std::cout << " {" << std::endl;
    std::cout << "%entry :" << std::endl;

    // alloc for function params
    allocate_function_params();

    block->Dump2KooPa();
    // add ret for last @dummy, seems stupid but no corner emitted case, i guess
    if (func_type_ident == "int")
        std::cout << TAB << "ret 0" << std::endl;
    if (func_type_ident == "void")
        std::cout << TAB << "ret" << std::endl;
    std::cout << "}" << std::endl;

    var_cnt = prev_var_cnt;
    if_cnt = prev_if_cnt;
    dummy_cnt = prev_dummy_cnt;
    ident_cnt_map = prev_ident_cnt_map; // backtrace when changing scope
    symbol_table_vec.pop_back();
}


// FuncTypeAST
void FuncTypeAST::Dump2KooPa() const {
    if (ident == "int")
        std::cout << ": i32";
    else if (ident == "void") {
        // if void, skip print
        // std::cout << "";
    }
}

void FuncFParamsAST::Dump2KooPa() const {
    int size = func_fparam_vec.size();
    // we need to determine when to print comma, 
    // so use size as loop indicator
    // sorry, "for (auto)" :(
    if (size != 0)
        func_fparam_vec[0]->Dump2KooPa();
    for (int i = 1; i < size; i++) {
        std::cout << ", ";
        func_fparam_vec[i]->Dump2KooPa();
    }
}

void FuncFParamAST::Dump2KooPa() const {
    symbol_name_t at_symbol_name = "@" + ident + "_p";
    symbol_name_t percent_symbol_name = "%" + ident + "_p";
    std::cout << at_symbol_name << ": ";
    // btype only i32
    std::cout << "i32";
    int idx = GETSCOPEIDX();
    symbol_table_vec[idx][ident] = percent_symbol_name;
}

void FuncRParamsAST::Dump2KooPa() const {
    return;
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


// BasicStmtAST
void BasicStmtAST::Dump2KooPa() const {
    // std::cout << "ret ";
    if (type == RET) {
        auto cur_var = exp->ExpDump2KooPa();
        std::cout << TAB << "ret ";
        std::cout << cur_var << std::endl;
        // add a dummy block for every ret.
        // dummy is unreachable, to guarantee only one "br/jump/ret" per block
        // stupid but seems work...
        std::string dummy_ident = "%dummy_" + std::to_string(dummy_cnt++);
        std::cout << dummy_ident << ":" << std::endl;
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
        exp->ExpDump2KooPa();
    }
    else if (type == EMPTY_EXP) {
        // do nothing
    }
    else if (type == BLOCK) {
        block->Dump2KooPa();
    }
    else if (type == CONTINUE) {
        int last_idx = GET_LAST_ELEMENT_FROM_VECTOR(while_table_vec);
        int last_while_num = while_table_vec[last_idx];
        std::string last_while_entry = "%while_entry_" + std::to_string(last_while_num);
        std::string ident_continue =  "%while_continue_" + std::to_string(last_while_num);
        std::cout << "  " << "jump " << last_while_entry << std::endl;
        // dummy here
        std::cout << ident_continue << ":" << std::endl;
    }
    else if (type == BREAK) {
        int last_idx = GET_LAST_ELEMENT_FROM_VECTOR(while_table_vec);
        int last_while_num = while_table_vec[last_idx];
        std::string last_while_end = "%while_end_" + std::to_string(last_while_num);
        std::string ident_break = "%while_break_" + std::to_string(last_while_num);
        std::cout << "  " << "jump " << last_while_end << std::endl;
        // dummy here
        std::cout << ident_break << ":" << std::endl;
    }
}

void IfElseStmtAST::Dump2KooPa() const {
    if (type == IF) {
        std::string exp_var = DERIVED_PTR(ExpAST, if_exp)->ExpDump2KooPa();
        std::string then_ident = "%then_" + std::to_string(if_cnt);
        std::string end_ident = "%end_" + std::to_string(if_cnt++);
        std::cout << TAB << "br " << exp_var << ", " << then_ident << ", " << end_ident << std::endl;

        std::cout << then_ident << ":" << std::endl;
        if_stmt->Dump2KooPa();
        std::cout << TAB << "jump " << end_ident << std::endl;
        std::cout << end_ident << ":" << std::endl;
    }
    else if (type == IF_ELSE) {
        std::string exp_var = DERIVED_PTR(ExpAST, if_exp)->ExpDump2KooPa();
        std::string then_ident = "%then_" + std::to_string(if_cnt);
        std::string else_ident = "%else_" + std::to_string(if_cnt);
        std::string end_ident = "%end_" + std::to_string(if_cnt++);
        std::cout << TAB << "br " << exp_var << ", " << then_ident << ", " << else_ident << std::endl;

        std::cout << then_ident << ":" << std::endl;
        if_stmt->Dump2KooPa();
        std::cout << TAB << "jump " << end_ident << std::endl;

        std::cout << else_ident << ":" << std::endl;
        else_stmt->Dump2KooPa();
        std::cout << TAB << "jump " << end_ident << std::endl;
        std::cout << end_ident << ":" << std::endl;
    }
}

void WhileStmtAST::Dump2KooPa() const {
    if(type == WHILE) {
        // get the closest index for the closest "while"
        while_table_vec.push_back(while_cnt);
        std::string ident_while_entry = "%while_entry_" + std::to_string(while_cnt);
        std::string ident_while_body = "%while_body_" + std::to_string(while_cnt);
        std::string ident_while_end = "%while_end_" + std::to_string(while_cnt++);

        std::cout << "  " << "jump " << ident_while_entry << std::endl;
        std::cout << ident_while_entry << ":" << std::endl;
        std::string ret_var = while_exp->ExpDump2KooPa();
        std::cout << "  " << "br " << ret_var << ", " << ident_while_body << ", " << ident_while_end << std::endl;
        std::cout << ident_while_body << ":" << std::endl;
        while_stmt->Dump2KooPa();
        std::cout << "  " << "jump " << ident_while_entry << std::endl;
        std::cout << ident_while_end << ":" << std::endl;

        while_table_vec.pop_back();
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
            std::cout << TAB << cur_var << " = load " << symbol_name << std::endl;
            return cur_var;
        }
    }
    return "";
}

// UnaryExpAST
std::string UnaryExpAST::ExpDump2KooPa() const {
    // std::cout << "type = " << type << std::endl;
    // std::cout << "ident = " << ident << std::endl;
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
    else if (type == FUNCR) {
        // store for call @half(10)
        // 1. get all expression dumped
        // 2. store all return variable
        std::vector<std::string> rparams_vec;
        if (func_rparams) {
            int func_rparam_size = DERIVED_PTR(FuncRParamsAST, func_rparams)->func_rparam_vec.size();
            for (int i = 0; i < func_rparam_size; i++) {
                std::string ret_var = DERIVED_PTR(FuncRParamsAST, func_rparams)->func_rparam_vec[i]->ExpDump2KooPa();
                rparams_vec.push_back(ret_var);
            }
        }
        std::string cur_var = "";
        if (func_table[ident] == "int") {
            cur_var = "%" + std::to_string(var_cnt++);
            std::cout << TAB << cur_var << " = call " << "@" + ident << "(";
        }
        else if (func_table[ident] == "void") {
            std::cout << TAB << "call " << "@" + ident << "(";
        }
        else {
            std::cout << "UNKNOWN FUNC TYPE: " << func_table[ident] << std::endl;
        }
        int size = rparams_vec.size();
        if (size != 0) {
            std::cout << rparams_vec[0];
        }
        for (int i = 1; i < size; i++) {
            std::cout << ", ";
            std::cout << rparams_vec[i];
        }
        std::cout << ")" << std::endl;
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
        // int result = 0;
        // if (lhs == 1) {
        // result = (rhs != 0);
        // }
        auto land_var = land_exp->ExpDump2KooPa();   // could be a variable(%1) or a number(6)
        std::string cur_var = "%" + std::to_string(var_cnt++);
        std::string tmp_res = "%" + std::to_string(var_cnt++);

        std::string then_ident = "%then_" + std::to_string(if_cnt);
        std::string else_ident = "%else_" + std::to_string(if_cnt);
        std::string end_ident = "%end_" + std::to_string(if_cnt++);

        std::string res_var = "%" + std::to_string(var_cnt++);
        
        std::cout << "  " << res_var << " = alloc i32" << std::endl;
        std::cout << "  " << "store 0, " << res_var << std::endl;
        std::cout << "  " << "br " << land_var << ", " << then_ident << ", " << else_ident << std::endl;
        // if lhs == 1, tmp_res = (rhs != 0), res = tmp_res
        std::cout << then_ident << ":" << std::endl;
        std::string tmp_res_var = "%" + std::to_string(var_cnt++);
        auto eq_var = eq_exp->ExpDump2KooPa();
        std::cout << "  " << tmp_res_var << " = ne " << eq_var << ", 0" << std::endl;
        std::cout << "  " << "store " << tmp_res_var << ", " << res_var << std::endl;
        std::cout << "  " << "jump " << end_ident << std::endl;
        // else if lhs == 0, do nothing.
        std::cout << else_ident << ":" << std::endl;
        std::cout << "  " << "jump " << end_ident << std::endl;
        std::cout << end_ident << ":" << std::endl;
        std::string ret_var = "%" + std::to_string(var_cnt++);
        std::cout << "  " << ret_var << " = load " << res_var << std::endl;
        return ret_var;
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
        std::string cur_var = "%" + std::to_string(var_cnt++);
        std::string res_var = "%" + std::to_string(var_cnt++);
        std::string then_ident = "%then_" + std::to_string(if_cnt);
        std::string else_ident = "%else_" + std::to_string(if_cnt);
        std::string end_ident = "%end_" + std::to_string(if_cnt++);
        
        std::cout << "  " << res_var << " = alloc i32" << std::endl;
        std::cout << "  " << "store 1, " << res_var << std::endl;
        std::cout << "  " << "br " << lor_var << ", " << then_ident << ", " << else_ident << std::endl;
        // if lor == 1, return res_var, do nothing
        std::cout << then_ident << ":" << std::endl;
        std::cout << "  " << "jump " << end_ident << std::endl;
        // else if lor == 0, tmp_result = (rhs != 0), result = tmp_result
        std::cout << else_ident << ":" << std::endl;
        std::string tmp_res_var = "%" + std::to_string(var_cnt++);
        auto land_var = land_exp->ExpDump2KooPa();
        std::cout << "  " << tmp_res_var << " = ne " << land_var << ", 0" << std::endl;
        std::cout << "  " << "store " << tmp_res_var << ", " << res_var << std::endl;
        std::cout << "  " << "jump " << end_ident << std::endl;
        std::cout << end_ident << ":" << std::endl;
        std::string ret_var = "%" + std::to_string(var_cnt++);
        std::cout << "  " << ret_var << " = load " << res_var << std::endl;
        // std::cout << TAB << tmp_or_var << " = or " << lor_var << ", " << land_var << std::endl;
        // std::cout << TAB << cur_var << " = ne " << tmp_or_var << ", 0" << std::endl;
        return ret_var;
    }
    return "";
}
