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
	virtual parse_expression::expression::argument export_operand(Operand arg) const;
	virtual parse_expression::expression::argument export_action(const Action &act) const;

	virtual pair<int, int> export_operator(int func) const = 0;
	virtual const parse_expression::precedence_set &precedence() const = 0;

	virtual parse_expression::expression export_expression(int type, const vector<Value> &arr) const;
	virtual parse_expression::expression::argument export_argument(Operand op, const vector<parse_expression::expression> *sub) const;
	virtual vector<parse_expression::expression::argument> export_arguments(int func, const vector<Operand> &args, const vector<parse_expression::expression> *sub) const;
	virtual parse_expression::expression export_expression(int func, const vector<Operand> &args, const vector<parse_expression::expression> *sub = nullptr) const;
	virtual parse_expression::expression export_expression(const Expression &expr) const;
	virtual parse_expression::assignment export_assignment(const Action &expr) const;
	virtual parse_expression::expression export_expression(const Parallel &expr) const;
	virtual parse_expression::expression export_composition(const Choice &expr) const;
};

}
