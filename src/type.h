#include <iostream>
#include <vector>
#include <map>
#include <variant>
#include <string>

// store all type of symbol;
// use std::variant to auto getting the symbol type, better than using std::optional!
// thank Siyuan Chen for providing the hint using variant!
typedef std::variant<int, std::string, double> symbol_t;
typedef std::string symbol_name_t;

typedef std::map<symbol_name_t, symbol_t> symbol_table_t;

