/*
 * export.cpp
 *
 *  Created on: Feb 6, 2015
 *      Author: nbingham
 */

#include "export.h"
#include <interpret_ucs/export.h>

parse_expression::expression export_expression(const arithmetic::expression &expr, ucs::variable_set &variables)
{
	vector<parse_expression::expression> result;

	for (int i = 0; i < (int)expr.operations.size(); i++)
	{
		parse_expression::expression add;
		add.valid = true;
		add.operations.push_back(expr.operations[i].get());
		add.level = parse_expression::expression::get_level(add.operations[0]);
		add.arguments.resize(expr.operations[i].operands.size());
		for (int j = 0; j < (int)expr.operations[i].operands.size(); j++)
		{
			if (expr.operations[i].operands[j].type == arithmetic::operand::invalid)
				add.arguments[j].constant = "null";
			else if (expr.operations[i].operands[j].type == arithmetic::operand::constant)
				add.arguments[j].constant = ::to_string(expr.operations[i].operands[j].index);
			else if (expr.operations[i].operands[j].type == arithmetic::operand::variable)
				add.arguments[j].literal = export_variable_name(expr.operations[i].operands[j].index, variables);
			else if (expr.operations[i].operands[j].type == arithmetic::operand::expression)
				add.arguments[j].sub = result[expr.operations[i].operands[j].index];
		}

		result.push_back(add);
	}

	if (result.size() > 0)
		return result.back();
	else
	{
		parse_expression::expression add;
		add.valid = true;
		add.arguments.push_back(parse_expression::argument("null"));
		return add;
	}
}

parse_expression::assignment export_assignment(const arithmetic::assignment &expr, ucs::variable_set &variables)
{
	parse_expression::assignment result;
	result.valid = true;

	if (expr.channel != -1)
		result.names.push_back(export_variable_name(expr.channel, variables));
	if (expr.variable != -1)
		result.names.push_back(export_variable_name(expr.variable, variables));

	if (expr.expr.operations.size() > 0)
		result.expressions.push_back(export_expression(expr.expr, variables));

	if (expr.behavior == arithmetic::assignment::assign)
		result.operation = ":=";
	else if (expr.behavior == arithmetic::assignment::guard)
		result.operation = "";
	else if (expr.behavior == arithmetic::assignment::send)
	{
		result.operation = "!";
		if (expr.variable != -1)
			result.operation += "?";
	}
	else if (expr.behavior == arithmetic::assignment::receive)
	{
		result.operation = "?";
		if (expr.expr.operations.size() != 0)
			result.operation += "!";
	}

	return result;
}

parse_expression::composition export_composition(const vector<arithmetic::assignment> &expr, ucs::variable_set &variables)
{
	parse_expression::composition result;
	result.valid = true;
	result.level = 1;

	for (int i = 0; i < (int)expr.size(); i++)
	{
		if (expr[i].behavior == arithmetic::assignment::guard)
			result.guards.push_back(export_expression(expr[i].expr, variables));
		else
			result.literals.push_back(export_assignment(expr[i], variables));
	}

	return result;
}

parse_expression::composition export_composition(const vector<vector<arithmetic::assignment> > &expr, ucs::variable_set &variables)
{
	parse_expression::composition result;
	result.valid = true;
	result.level = 0;

	for (int i = 0; i < (int)expr.size(); i++)
		result.compositions.push_back(export_composition(expr[i], variables));

	return result;
}
