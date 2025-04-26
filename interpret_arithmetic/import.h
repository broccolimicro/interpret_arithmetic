#pragma once

#include <common/standard.h>
#include <common/net.h>

#include <parse/tokenizer.h>

#include <parse_expression/expression.h>
#include <parse_expression/assignment.h>
#include <parse_expression/composition.h>

#include <arithmetic/expression.h>
#include <arithmetic/action.h>

namespace arithmetic {

int import_net(const parse_ucs::variable_name &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define);
State import_state(const parse_expression::composition &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define);
Expression import_argument(const parse_expression::argument &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define);
Expression import_expression(const parse_expression::expression &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define);
Action import_action(const parse_expression::assignment &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define);
Parallel import_parallel(const parse_expression::composition &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define);
Choice import_choice(const parse_expression::composition &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define);

}
