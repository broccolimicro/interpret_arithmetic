#pragma once

#include <common/standard.h>

#include "expression.h"
#include <interpret_arithmetic/import.h>

struct ExpressionInterpreter : arithmetic::ExpressionInterpreter<expr_group> {
	ExpressionInterpreter(tokenizer *tokens=nullptr, bool auto_define=true);
	ExpressionInterpreter();

	arithmetic::Expression import_unary(parse_expression::operation op, arithmetic::Expression expr);
	arithmetic::Expression import_binary(parse_expression::operation op, arithmetic::Expression left, arithmetic::Expression right);
	arithmetic::Expression import_group(parse_expression::operation op, vector<arithmetic::Expression> args);
	arithmetic::Expression import_modifier(parse_expression::operation op, const vector<argument> &arguments, ucs::Netlist nets, int default_id);
};

arithmetic::Expression import_expression(const expression &syntax, ucs::Netlist nets, int default_id=0, tokenizer *tokens=nullptr, bool auto_define=true);
arithmetic::State import_state(const composition &syntax, ucs::Netlist nets, int default_id=0, tokenizer *tokens=nullptr, bool auto_define=true);

arithmetic::Action import_action(const assignment &syntax, ucs::Netlist nets, int default_id=0, tokenizer *tokens=nullptr, bool auto_define=true);
arithmetic::Parallel import_parallel(const composition &syntax, ucs::Netlist nets, int default_id=0, tokenizer *tokens=nullptr, bool auto_define=true);
arithmetic::Choice import_choice(const composition &syntax, ucs::Netlist nets, int default_id=0, tokenizer *tokens=nullptr, bool auto_define=true);

