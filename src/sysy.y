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
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
    std::string *str_val;
    int int_val;
    BaseAST *ast_val;
    ExpBaseAST *exp_ast_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN
%token <str_val> IDENT
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block Stmt
%type <int_val> Number
%type <str_val> UnaryOp MulOp AddOp RelOp EqOp LandOp LorOp
%type <exp_ast_val> Exp PrimaryExp UnaryExp MulExp AddExp RelExp EqExp LAndExp LOrExp

%%

// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
// 之前我们定义了 FuncDef 会返回一个 str_val, 也就是字符串指针
// 而 parser 一旦解析完 CompUnit, 就说明所有的 token 都被解析了, 即解析结束了
// 此时我们应该把 FuncDef 返回的结果收集起来, 作为 AST 传给调用 parser 的函数
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值
CompUnit
    : FuncDef {
        auto comp_unit = make_unique<CompUnitAST>();
        comp_unit->func_def = unique_ptr<BaseAST>($1);
        ast = move(comp_unit);
    }
    ;

// FuncDef ::= FuncType IDENT '(' ')' Block;
// 我们这里可以直接写 '(' 和 ')', 因为之前在 lexer 里已经处理了单个字符的情况
// 解析完成后, 把这些符号的结果收集起来, 然后拼成一个新的字符串, 作为结果返回
// $$ 表示非终结符的返回值, 我们可以通过给这个符号赋值的方法来返回结果
// 你可能会问, FuncType, IDENT 之类的结果已经是字符串指针了
// 为什么还要用 unique_ptr 接住它们, 然后再解引用, 把它们拼成另一个字符串指针呢
// 因为所有的字符串指针都是我们 new 出来的, new 出来的内存一定要 delete
// 否则会发生内存泄漏, 而 unique_ptr 这种智能指针可以自动帮我们 delete
// 虽然此处你看不出用 unique_ptr 和手动 delete 的区别, 但当我们定义了 AST 之后
// 这种写法会省下很多内存管理的负担
FuncDef
    : FuncType IDENT '(' ')' Block {
        auto ast = new FuncDefAST();
        ast->func_type = unique_ptr<BaseAST>($1);
        ast->ident = *unique_ptr<string>($2);
        ast->block = unique_ptr<BaseAST>($5);
        $$ = ast;
    }
    ;

// 同上, 不再解释
FuncType
    : INT {
        auto ast = new FuncTypeAST();
        ast->ident = "int";
        $$ = ast;
    }
    ;

Block
    : '{' Stmt '}' {
        auto ast = new BlockAST();
        ast->statement = unique_ptr<BaseAST>($2);
        $$ = ast;
    }
    ;

Stmt
    : RETURN Exp ';' {
        auto ast = new StmtAST();
        // ast->number = $2;
        ast->exp = unique_ptr<ExpBaseAST>($2);
        $$ = ast;
    }
    ;

Number
    : INT_CONST {
        // just return the first param since the type has changed
        $$ = $1;
    }
    ;

// why can't u tell me Exp ::= LorExp earlier??
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
    unary_exp->op = *unique_ptr<string>($1);
    unary_exp->exp = unique_ptr<ExpBaseAST>($2);
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
    mul_exp->op = *unique_ptr<string>($2);
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
    add_exp->op = *unique_ptr<string>($2);
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
    rel_exp->op = *unique_ptr<string>($2);
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
    eq_exp->op = *unique_ptr<string>($2);
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
    land_exp->op = *unique_ptr<string>($2);
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
    lor_exp->op = *unique_ptr<string>($2);
    lor_exp->land_exp = std::unique_ptr<ExpBaseAST>($3);
    $$ = lor_exp;
  }
  ;

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, unique_ptr<ExpBaseAST> &exp_ast, const char *s) {
    cerr << "error: " << s << endl;
}