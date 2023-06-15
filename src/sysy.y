%code requires {
    #include <memory>
    #include <string>
    #include "AST.h"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include "AST.h"
// #define YYDEBUG 1
#define YYERROR_VERBOSE 1
// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, std::unique_ptr<ExpBaseAST> &exp_ast, const char *s);
using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
// %parse-param { std::unique_ptr<std::string> &ast }
%parse-param { std::unique_ptr<BaseAST> &ast }
%parse-param { std::unique_ptr<ExpBaseAST> &exp_ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<std::string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
    std::string *str_val;
    int int_val;
    BaseAST *ast_val;
    ExpBaseAST *exp_ast_val;
    std::vector<std::unique_ptr<BaseAST> > *vec_ast_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN CONST_TOKEN IF ELSE WHILE CONTINUE BREAK VOID
%token <str_val> IDENT
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> FuncDef BType Block Stmt LVal BasicStmt OpenStmt ClosedStmt
%type <int_val> Number
%type <ast_val> Decl ConstDecl ConstDef BlockItem ConstInitVal VarDecl VarDef InitVal
%type <ast_val> CompUnit FuncFParams FuncFParam FuncRParams
%type <str_val> UnaryOp MulOp AddOp RelOp EqOp LandOp LorOp
%type <exp_ast_val> Exp ConstExp PrimaryExp UnaryExp MulExp AddExp RelExp EqExp LAndExp LOrExp
%type <vec_ast_val> BlockItemVec ConstDefVec VarDefVec

%%

// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
// 之前我们定义了 FuncDef 会返回一个 str_val, 也就是字符串指针
// 而 parser 一旦解析完 CompUnit, 就说明所有的 token 都被解析了, 即解析结束了
// 此时我们应该把 FuncDef 返回的结果收集起来, 作为 AST 传给调用 parser 的函数
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值
CompUnitHead
  : CompUnit {
    // auto comp_unit = make_unique<CompUnitAST>();
    // comp_unit->func_def = unique_ptr<BaseAST>($1);
    auto comp_unit_head = unique_ptr<BaseAST>($1);
    ast = move(comp_unit_head);
  }
  ;

CompUnit
  : FuncDef {
    auto comp_unit = new CompUnitAST();
    comp_unit->func_def_vec.push_back(std::unique_ptr<BaseAST>($1));
    $$ = comp_unit;
  }
  | Decl {
    auto comp_unit = new CompUnitAST();
    comp_unit->decl_vec.push_back(std::unique_ptr<BaseAST>($1));
    $$ = comp_unit;
  }
  | CompUnit FuncDef {
    CompUnitAST *comp_unit = (CompUnitAST*)($1);
    comp_unit->func_def_vec.push_back(std::unique_ptr<BaseAST>($2));
    $$ = comp_unit;
  }
  | CompUnit Decl {
    CompUnitAST *comp_unit = (CompUnitAST*)($1);
    comp_unit->decl_vec.push_back(std::unique_ptr<BaseAST>($2));
    $$ = comp_unit;
  }
  ;

FuncDef
  : BType IDENT '(' ')' Block {
    auto func_def = new FuncDefAST();
    func_def->func_type = unique_ptr<BaseAST>($1);
    func_def->ident = *unique_ptr<std::string>($2);
    func_def->func_fparams = nullptr;
    func_def->block = unique_ptr<BaseAST>($5);
    $$ = func_def;
  }
  | BType IDENT '(' FuncFParams ')' Block {
    auto func_def = new FuncDefAST();
    func_def->func_type = unique_ptr<BaseAST>($1);
    func_def->ident = *unique_ptr<std::string>($2);
    func_def->func_fparams = unique_ptr<BaseAST>($4);
    func_def->block = unique_ptr<BaseAST>($6);
    $$ = func_def;
  }
  ;

FuncFParams
  : FuncFParam {
    auto func_fparams = new FuncFParamsAST();
    func_fparams->func_fparam_vec.push_back(std::unique_ptr<BaseAST>($1));
    $$ = func_fparams;
  }
  | FuncFParams ',' FuncFParam {
    FuncFParamsAST *func_fparams = (FuncFParamsAST*)($1);
    func_fparams->func_fparam_vec.push_back(std::unique_ptr<BaseAST>($3));
    $$ = func_fparams;
  }
  ;


FuncFParam
  : BType IDENT {
    auto func_fparam = new FuncFParamAST();
    func_fparam->b_type = std::unique_ptr<BaseAST>($1);
    func_fparam->ident = *std::unique_ptr<std::string>($2);
    $$ = func_fparam;
  }
  ;


FuncRParams
  : Exp {
    auto func_rparams = new FuncRParamsAST();
    func_rparams->func_rparam_vec.push_back(std::unique_ptr<ExpBaseAST>($1));
    $$ = func_rparams;
  }
  | FuncRParams ',' Exp {
    FuncRParamsAST *func_rparams = (FuncRParamsAST*)($1);
    func_rparams->func_rparam_vec.push_back(std::unique_ptr<ExpBaseAST>($3));
    $$ = func_rparams;
  }
  ;

BType
  : INT {
    auto ast = new FuncTypeAST();
    ast->ident = "int";
    $$ = ast;
  }
  | VOID {
    auto ast = new FuncTypeAST();
    ast->ident = "void";
    $$ = ast;
  }
  ;

LVal
  : IDENT {
    auto lval = new LValAST();
    lval->ident = *unique_ptr<std::string>($1);
    $$ = lval;
  }
  ;

BlockItem
  : Decl {
    auto block_item = new BlockItemAST();
    block_item->type = BlockItemAST::DECL;
    block_item->decl = unique_ptr<BaseAST>($1);
    $$ = block_item;
  }
  | Stmt {
    auto block_item = new BlockItemAST();
    block_item->type = BlockItemAST::STMT;
    block_item->stmt = unique_ptr<BaseAST>($1);
    $$ = block_item;
  }
  ;

BlockItemVec
  : %empty {
    std::vector<std::unique_ptr<BaseAST> > *block_item_vec = new std::vector<std::unique_ptr<BaseAST> >;
    // block_item_vec->push_back(std::unique_ptr<BaseAST>($1));
    $$ = block_item_vec;
  }
  | BlockItemVec BlockItem {
    std::vector<std::unique_ptr<BaseAST> > *block_item_vec = ($1);
    block_item_vec->push_back(std::unique_ptr<BaseAST>($2));
    $$ = block_item_vec;
  }
  ;


Block
    : '{' BlockItemVec '}' {
        auto block = new BlockAST();
        std::vector<std::unique_ptr<BaseAST> > *block_item_vec = ($2);
        for (auto &it : *block_item_vec) {
          block->block_item_vec.push_back(move(it));
        }
        $$ = block;
    }
    ;

// reference from https://en.wikipedia.org/wiki/Dangling_else
Stmt 
  : OpenStmt {
    auto stmt = ($1);
    $$ = stmt;
  }
  | ClosedStmt {
    auto stmt = ($1);
    $$ = stmt;
  }
  ;

OpenStmt
  : IF '(' Exp ')' Stmt {
    auto stmt = new IfElseStmtAST();
    stmt->type = IfElseStmtAST::IF;
    stmt->if_exp = unique_ptr<ExpBaseAST>($3);
    stmt->if_stmt = unique_ptr<BaseAST>($5);
    $$ = stmt;
  }
  | IF '(' Exp ')' ClosedStmt ELSE OpenStmt {
    auto stmt = new IfElseStmtAST();
    stmt->type = IfElseStmtAST::IF_ELSE;
    stmt->if_exp = unique_ptr<ExpBaseAST>($3);
    stmt->if_stmt = unique_ptr<BaseAST>($5);
    stmt->else_stmt = unique_ptr<BaseAST>($7);
    $$ = stmt;
  }
  | WHILE '(' Exp ')' OpenStmt {
    auto stmt = new WhileStmtAST();
    stmt->type = WhileStmtAST::WHILE;
    stmt->while_exp = unique_ptr<ExpBaseAST>($3);
    stmt->while_stmt = unique_ptr<BaseAST> ($5);
    $$ = stmt;
  }
  ;


ClosedStmt
    : BasicStmt {
        auto stmt = ($1);
        $$ = stmt;
    }
    | IF '(' Exp ')' ClosedStmt ELSE ClosedStmt {
        auto stmt = new IfElseStmtAST();
        stmt->type = IfElseStmtAST::IF_ELSE;
        stmt->if_exp = unique_ptr<ExpBaseAST>($3);
        stmt->if_stmt = unique_ptr<BaseAST>($5);
        stmt->else_stmt = unique_ptr<BaseAST>($7);
        $$ = stmt;
    }
    | WHILE '(' Exp ')' ClosedStmt {
        auto stmt = new WhileStmtAST();
        stmt->type = WhileStmtAST::WHILE;
        stmt->while_exp = unique_ptr<ExpBaseAST>($3);
        stmt->while_stmt = unique_ptr<BaseAST> ($5);
        $$ = stmt;
    }
    ;

BasicStmt
    : LVal '=' Exp ';' {
        auto stmt = new BasicStmtAST();
        stmt->type = BasicStmtAST::LVAL;
        stmt->lval = unique_ptr<BaseAST>($1);
        stmt->exp = unique_ptr<ExpBaseAST>($3);
        $$ = stmt;
    }
    | Exp ';' {
      auto stmt = new BasicStmtAST();
      stmt->type = BasicStmtAST::EXP;
      stmt->exp = unique_ptr<ExpBaseAST>($1);
      $$ = stmt;
    }
    | ';' {
      auto stmt = new BasicStmtAST();
      stmt->type = BasicStmtAST::EMPTY_EXP;
      stmt->exp = nullptr;
      $$ = stmt;
    }
    | Block {
      auto stmt = new BasicStmtAST();
      stmt->type = BasicStmtAST::BLOCK;
      stmt->block = unique_ptr<BaseAST>($1);
      $$ = stmt;
    }
    | RETURN Exp ';' {
        auto stmt = new BasicStmtAST();
        // ast->number = $2;
        stmt->exp = unique_ptr<ExpBaseAST>($2);
        stmt->type = BasicStmtAST::RET;
        $$ = stmt;
    }
    | RETURN ';' {
      auto stmt = new BasicStmtAST();
      stmt->type = BasicStmtAST::EMPTY_RET;
      stmt->exp = nullptr;
      $$ = stmt;
    }
    | CONTINUE ';' {
      auto stmt = new BasicStmtAST();
      stmt->type = BasicStmtAST::CONTINUE;
      stmt->exp = nullptr;
      $$ = stmt;
    }
    | BREAK ';' {
      auto stmt = new BasicStmtAST();
      stmt->type = BasicStmtAST::BREAK;
      stmt->exp = nullptr;
      $$ = stmt;
    }
    ;

Number
    : INT_CONST {
        $$ = $1;
    }
    ;

Decl
    : ConstDecl {
        auto decl = new DeclAST();
        decl->type = DeclAST::CONST;
        decl->const_decl = unique_ptr<BaseAST>($1);
        $$ = decl;
    }
    | VarDecl {
        auto decl = new DeclAST();
        decl->type = DeclAST::VAR;
        decl->var_decl = unique_ptr<BaseAST>($1);
        $$ = decl;
    }
    ;

ConstDef
  : IDENT '=' ConstInitVal {
    auto const_def = new ConstDefAST();
    const_def->ident = *unique_ptr<std::string>($1);
    const_def->const_init_val = unique_ptr<BaseAST>($3);
    $$ = const_def;
  }
  ;

ConstDefVec
  : ConstDef {
      std::vector<std::unique_ptr<BaseAST> > *const_def_vec = new std::vector<std::unique_ptr<BaseAST> >;
      const_def_vec->push_back(std::unique_ptr<BaseAST>($1));
      $$ = const_def_vec;
  }
  | ConstDefVec ',' ConstDef {
      std::vector<std::unique_ptr<BaseAST> > *const_def_vec = ($1);
      const_def_vec->push_back(std::unique_ptr<BaseAST>($3));
      $$ = const_def_vec;
  }
  ;


ConstDecl
  : CONST_TOKEN BType ConstDefVec ';' {
      auto const_decl = new ConstDeclAST();
      const_decl->b_type = unique_ptr<BaseAST>($2);
      std::vector<std::unique_ptr<BaseAST> > *const_def_vec = ($3);
      for (auto &it : *const_def_vec) {
        const_decl->const_def_vec.push_back(move(it));
      }
      $$ = const_decl;
  }
  ;

ConstInitVal
  : ConstExp {
    auto const_init_val = new ConstInitValAST();
    const_init_val->const_exp = unique_ptr<ExpBaseAST>($1);
    $$ = const_init_val;
  }
  ;

VarDef
  : IDENT {
    auto var_def = new VarDefAST();
    var_def->type = VarDefAST::iDENT;
    var_def->ident = *unique_ptr<std::string>($1);
    $$ = var_def;
  }
  | IDENT '=' InitVal {
    auto var_def = new VarDefAST();
    var_def->type = VarDefAST::INIT_VAL;
    var_def->ident = *unique_ptr<std::string>($1);
    var_def->init_val = unique_ptr<BaseAST>($3);
    $$ = var_def;
  }
  ;

VarDefVec
  : VarDef {
    std::vector<std::unique_ptr<BaseAST> > *var_vec = new std::vector<std::unique_ptr<BaseAST> >;
    var_vec->push_back(std::unique_ptr<BaseAST>($1));
    $$ = var_vec;
  }
  | VarDefVec ',' VarDef {
    std::vector<std::unique_ptr<BaseAST> > *var_vec = ($1);
    var_vec->push_back(std::unique_ptr<BaseAST>($3));
    $$ = var_vec;
  }
  ;


VarDecl
  : BType VarDefVec ';' {
    auto var_decl = new VarDeclAST();
    var_decl->b_type = unique_ptr<BaseAST>($1);
    std::vector<std::unique_ptr<BaseAST> > *var_def_vec = ($2); // still ptr
    for (auto &it : *var_def_vec) {
        var_decl->var_def_vec.push_back(move(it));
    }
    $$ = var_decl;
  }
  ;


InitVal
  : Exp {
    auto init_val = new InitValAST();
    init_val->exp = unique_ptr<ExpBaseAST>($1);
    $$ = init_val;
  }
  ;

Exp
  : LOrExp {
    auto exp = new ExpAST();
    exp->lor_exp = unique_ptr<ExpBaseAST>($1);
    $$ = exp;
  }
  ;

PrimaryExp
  : '(' Exp ')' {
    auto primary_exp = new PrimaryExpAST();
    primary_exp->type = PrimaryExpAST::EXP;
    primary_exp->exp = unique_ptr<ExpBaseAST>($2);
    $$ = primary_exp;
  }
  | Number {
    auto primary_exp = new PrimaryExpAST();
    primary_exp->type = PrimaryExpAST::NUMBER;
    primary_exp->number = ($1);
    $$ = primary_exp;
  }
  | LVal {
    auto primary_exp = new PrimaryExpAST();
    primary_exp->type = PrimaryExpAST::LVAL;
    primary_exp->lval = unique_ptr<BaseAST>($1);
    $$ = primary_exp;
  }
  ;


UnaryExp
  : PrimaryExp {
    auto unary_exp = new UnaryExpAST();
    unary_exp->type = UnaryExpAST::PRIMARY;
    unary_exp->exp = unique_ptr<ExpBaseAST>($1);
    $$ = unary_exp;
  }
  | UnaryOp UnaryExp {
    auto unary_exp = new UnaryExpAST();
    unary_exp->type = UnaryExpAST::UNARY;
    unary_exp->op = *unique_ptr<std::string>($1);
    unary_exp->exp = unique_ptr<ExpBaseAST>($2);
    $$ = unary_exp;
  }
  | IDENT '(' FuncRParams ')' {
    auto unary_exp = new UnaryExpAST();
    unary_exp->type = UnaryExpAST::FUNCR;
    unary_exp->ident = *unique_ptr<std::string>($1);
    unary_exp->func_rparams = unique_ptr<BaseAST>($3);
    $$ = unary_exp;
  }
  | IDENT '(' ')' {
    auto unary_exp = new UnaryExpAST();
    unary_exp->type = UnaryExpAST::FUNCR;
    unary_exp->ident = *unique_ptr<std::string>($1);
    unary_exp->func_rparams = nullptr;
    $$ = unary_exp;
  }
  ;

UnaryOp
  : '+' {
    string *op = new string("+");
    $$ = op;
  }
  | '-' {
    string *op = new string("-");
    $$ = op;
  }
  | '!' {
    string *op = new string("!");
    $$ = op;
  }
  ;

MulOp
  : '*' {
    string *op = new string("*");
    $$ = op;
  }
  | '/' {
    string *op = new string("/");
    $$ = op;
  }
  | '%' {
    string *op = new string("%");
    $$ = op;
  }
  ;

AddOp
  : '+' {
    string *op = new string("+");
    $$ = op;
  }
  | '-' {
    string *op = new string("-");
    $$ = op;
  }
  ;

RelOp
  : '<' {
    string *op = new string("<");
    $$ = op;
  }
  | '>' {
    string *op = new string(">");
    $$ = op;
  }
  | '<' '=' {
    string *op = new string("<=");
    $$ = op;
  }
  | '>' '=' {
    string *op = new string(">=");
    $$ = op;
  }
  ;

EqOp
  : '=' '=' {
    string *op = new string("==");
    $$ = op;
  }
  | '!' '=' {
    string *op = new string("!=");
    $$ = op;
  }
  ;

LandOp
  : '&' '&' {
    string *op = new string("&&");
    $$ = op;
  }
  ;

LorOp
  : '|' '|' {
    string *op = new string("||");
    $$ = op;
  }
  ;

MulExp
  : UnaryExp {
    auto mul_exp = new MulExpAST();
    mul_exp->type = MulExpAST::UNARY;
    mul_exp->unary_exp = unique_ptr<ExpBaseAST>($1);
    mul_exp->op = "";
    $$ = mul_exp;
  }
  | MulExp MulOp UnaryExp {
    auto mul_exp = new MulExpAST();
    mul_exp->type = MulExpAST::MUL;
    mul_exp->mul_exp = std::unique_ptr<ExpBaseAST>($1);
    mul_exp->op = *unique_ptr<std::string>($2);
    mul_exp->unary_exp = unique_ptr<ExpBaseAST>($3);
    $$ = mul_exp;
  }
  ;

AddExp
  : MulExp {
    auto add_exp = new AddExpAST();
    add_exp->type = AddExpAST::MUL;
    add_exp->mul_exp = std::unique_ptr<ExpBaseAST>($1);
    add_exp->op = "";
    $$ = add_exp;
  }
  | AddExp AddOp MulExp {
    auto add_exp = new AddExpAST();
    add_exp->type = AddExpAST::ADD;
    add_exp->add_exp = std::unique_ptr<ExpBaseAST>($1);
    add_exp->op = *unique_ptr<std::string>($2);
    add_exp->mul_exp = std::unique_ptr<ExpBaseAST>($3);
    $$ = add_exp;
  }
  ;


RelExp
  : AddExp {
    auto rel_exp = new RelExpAST();
    rel_exp->type = RelExpAST::ADD;
    rel_exp->add_exp = std::unique_ptr<ExpBaseAST>($1);
    rel_exp->op = "";
    $$ = rel_exp;
  }
  | RelExp RelOp AddExp {
    auto rel_exp = new RelExpAST();
    rel_exp->type = RelExpAST::REL;
    rel_exp->rel_exp = std::unique_ptr<ExpBaseAST>($1);
    rel_exp->op = *unique_ptr<std::string>($2);
    rel_exp->add_exp = std::unique_ptr<ExpBaseAST>($3);
    $$ = rel_exp;
  }
  ;

EqExp
  : RelExp {
    auto eq_exp = new EqExpAST();
    eq_exp->type = EqExpAST::REL;
    eq_exp->rel_exp = std::unique_ptr<ExpBaseAST>($1);
    eq_exp->op = "";
    $$ = eq_exp;
  }
  | EqExp EqOp RelExp {
    auto eq_exp = new EqExpAST();
    eq_exp->type = EqExpAST::EQ;
    eq_exp->eq_exp = std::unique_ptr<ExpBaseAST>($1);
    eq_exp->op = *unique_ptr<std::string>($2);
    eq_exp->rel_exp = std::unique_ptr<ExpBaseAST>($3);
    $$ = eq_exp;
  }
  ;

LAndExp
  : EqExp {
    auto land_exp = new LAndExpAST();
    land_exp->type = LAndExpAST::EQ;
    land_exp->eq_exp = std::unique_ptr<ExpBaseAST>($1);
    land_exp->op = "";
    $$ = land_exp;
  }
  | LAndExp LandOp EqExp {
    auto land_exp = new LAndExpAST();
    land_exp->type = LAndExpAST::LAND;
    land_exp->land_exp = std::unique_ptr<ExpBaseAST>($1);
    land_exp->op = *unique_ptr<std::string>($2);
    land_exp->eq_exp = std::unique_ptr<ExpBaseAST>($3);
    $$ = land_exp;
  }
  ;

LOrExp
  : LAndExp {
    auto lor_exp = new LOrExpAST();
    lor_exp->type = LOrExpAST::LAND;
    lor_exp->land_exp = std::unique_ptr<ExpBaseAST>($1);
    lor_exp->op = "";
    $$ = lor_exp;
  }
  | LOrExp LorOp LAndExp {
    auto lor_exp = new LOrExpAST();
    lor_exp->type = LOrExpAST::LOR;
    lor_exp->lor_exp = std::unique_ptr<ExpBaseAST>($1);
    lor_exp->op = *unique_ptr<std::string>($2);
    lor_exp->land_exp = std::unique_ptr<ExpBaseAST>($3);
    $$ = lor_exp;
  }
  ;

ConstExp
  : Exp {
    auto const_exp = new ConstExpAST();
    const_exp->exp = unique_ptr<ExpBaseAST>($1);
    $$ = const_exp;
  }

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, unique_ptr<ExpBaseAST> &exp_ast, const char *s) {
    cerr << "error: " << s << endl;
}