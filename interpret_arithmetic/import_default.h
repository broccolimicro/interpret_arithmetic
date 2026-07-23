#pragma once

#include <common/standard.h>
#include <common/net.h>

#include <parse/tokenizer.h>
#include <parse_expression/expression.h>
#include <arithmetic/expression.h>

namespace arithmetic {

Expression import_literal(string name, ucs::Netlist symbols, tokenizer *tokens, bool auto_define);
Expression import_constant(string cnst, tokenizer *tokens);

}
