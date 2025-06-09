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

string import_constant(const parse_expression::expression &syntax, tokenizer *tokens);
string import_constant(const parse_expression::argument &syntax, tokenizer *tokens);
string import_net_name(const parse_expression::argument &syntax, tokenizer *tokens);
string import_net_name(const parse_expression::expression &syntax, tokenizer *tokens);

int import_net(string syntax, ucs::Netlist nets, tokenizer *tokens, bool auto_define);
int import_net(const parse_expression::expression &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define);
// TODO(edward.bingham) all above functions are identical to interpret_boolean implementations 

State import_state(const parse_expression::composition &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define);
Expression import_argument(const parse_expression::argument &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define);
Expression import_expression(const parse_expression::expression &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define);
Action import_action(const parse_expression::assignment &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define);
Parallel import_parallel(const parse_expression::composition &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define);
Choice import_choice(const parse_expression::composition &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define);

}
