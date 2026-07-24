#pragma once

#include <common/standard.h>

#include <parse_expression/expression.h>
#include <parse_expression/assignment.h>

#include <arithmetic/expression.h>
#include <arithmetic/action.h>
#include <arithmetic/algorithm.h>

namespace arithmetic {

string export_value(const Value &v);

struct ExpressionExporter {
	// override these
	virtual parse_expression::operation export_operator(int func) const = 0;
	virtual const parse_expression::precedence_set &precedence() const = 0;
	virtual parse_expression::expression::argument export_constant(Value value) const = 0;
	virtual parse_expression::expression::argument export_literal(size_t index) const = 0;
	virtual parse_expression::expression::argument export_action(const Action &act) const;
	virtual parse_expression::expression export_special(int func, const vector<parse_expression::expression::argument> &args) const;

	// these don't need to be overridden
	virtual parse_expression::expression::argument export_operand(Operand arg) const;
	virtual parse_expression::expression export_expression(int type, const vector<Value> &arr) const;
	virtual parse_expression::expression::argument export_argument(Operand op, const vector<parse_expression::expression> *sub) const;
	virtual vector<parse_expression::expression::argument> export_arguments(const vector<Operand> &args, const vector<parse_expression::expression> *sub) const;
	virtual parse_expression::expression export_expression(int func, vector<Operand> args, const vector<parse_expression::expression> *sub = nullptr) const;
	virtual parse_expression::expression export_expression(const Expression &expr) const;
	virtual parse_expression::assignment export_expression(const Action &expr) const;
	virtual parse_expression::expression export_expression(const Parallel &expr) const;
	virtual parse_expression::expression export_expression(const Choice &expr) const;
};

}
