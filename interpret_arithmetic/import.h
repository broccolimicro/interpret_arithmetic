/*
 * import.h
 *
 *  Created on: Feb 6, 2015
 *      Author: nbingham
 */

#pragma once

#include <common/standard.h>

#include <parse/tokenizer.h>

#include <parse_expression/expression.h>
#include <parse_expression/assignment.h>
#include <parse_expression/composition.h>

#include <arithmetic/expression.h>
#include <arithmetic/action.h>

#include "interface.h"

namespace arithmetic {

int import_net(const parse_ucs::variable_name &syntax, Netlist nets, int default_id, tokenizer *tokens, bool auto_define);
State import_state(const parse_expression::composition &syntax, Netlist nets, int default_id, tokenizer *tokens, bool auto_define);
Expression import_expression(const parse_expression::expression &syntax, Netlist nets, int default_id, tokenizer *tokens, bool auto_define);
Action import_action(const parse_expression::assignment &syntax, Netlist nets, int default_id, tokenizer *tokens, bool auto_define);
Parallel import_parallel(const parse_expression::composition &syntax, Netlist nets, int default_id, tokenizer *tokens, bool auto_define);
Choice import_choice(const parse_expression::composition &syntax, Netlist nets, int default_id, tokenizer *tokens, bool auto_define);

}
