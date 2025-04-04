/*
 * export.cpp
 *
 *  Created on: Feb 6, 2015
 *      Author: nbingham
 */

#include "export.h"
#include <interpret_ucs/export.h>

parse_expression::expression export_expression(const arithmetic::value &v, const ucs::variable_set &variables) {
	parse_expression::expression result;
	result.valid = true;
	result.level = parse_expression::expression::get_level("");
	if (v.data == arithmetic::value::neutral) {
		result.arguments.push_back(parse_expression::argument("gnd"));
	} else if (v.data == arithmetic::value::valid) {
		result.arguments.push_back(parse_expression::argument("vdd"));
	} else if (v.data == arithmetic::value::unstable) {
		result.arguments.push_back(parse_expression::argument("unstable"));
	} else {
		result.arguments.push_back(parse_expression::argument(::to_string(v.data)));
	}
	return result;	
}

parse_expression::expression export_expression(const arithmetic::state &s, const ucs::variable_set &variables)
{
	vector<parse_expression::expression> result;

	for (int i = 0; i < (int)s.values.size(); i++)
	{
		if (s.values[i].data != arithmetic::value::unknown) {
			parse_expression::expression add;
			add.valid = true;
			if (s.values[i].data == arithmetic::value::neutral) {
				add.operations.push_back("~");
				add.level = parse_expression::expression::get_level(add.operations[0]);
				add.arguments.resize(1);
				add.arguments[0].literal = export_variable_name(i, variables);
			} else if (s.values[i].data == arithmetic::value::valid) {
				add.operations.push_back("");
				add.level = parse_expression::expression::get_level(add.operations[0]);
				add.arguments.resize(1);
				add.arguments[0].literal = export_variable_name(i, variables);
			} else {
				add.operations.push_back("==");
				add.level = parse_expression::expression::get_level(add.operations[0]);
				add.arguments.resize(2);
				add.arguments[0].literal = export_variable_name(i, variables);
				if (s.values[i].data == arithmetic::value::unstable) {
					add.arguments[1].constant = "unstable";
				} else {
					add.arguments[1].constant = ::to_string(s.values[i].data);
				}
			}

			result.push_back(add);
		}
	}

	if (result.size() == 1) {
		return result.back();
	}

	parse_expression::expression add;
	add.valid = true;

	if (result.size() > 1) {
		add.operations.push_back("&");
		add.level = parse_expression::expression::get_level(add.operations[0]);
		add.arguments.resize(result.size());
		for (int i = 0; i < (int)result.size(); i++) {
			add.arguments[i].sub = result[i];
		}
	} else {
		add.arguments.push_back(parse_expression::argument("vdd"));
	}

	return add;
}


parse_expression::composition export_composition(const arithmetic::state &s, const ucs::variable_set &variables)
{
	parse_expression::composition result;
	result.valid = true;
	result.level = 1;

	for (int i = 0; i < (int)s.values.size(); i++) {
		if (s.values[i].data != arithmetic::value::unknown) {
			parse_expression::assignment assign;
			assign.valid = true;
			assign.names.push_back(export_variable_name(i, variables));
			if (s.values[i].data == arithmetic::value::neutral) {
				assign.operation = "-";
			} else if (s.values[i].data == arithmetic::value::valid) {
				assign.operation = "+";
			} else if (s.values[i].data == arithmetic::value::unstable) {
				assign.operation = "~";
			} else {
				assign.operation = "=";
				assign.expressions.push_back(export_expression(s.values[i], variables));
			}
			result.literals.push_back(assign);
		}
	}

	return result;
}

parse_expression::composition export_composition(const arithmetic::region &r, const ucs::variable_set &variables)
{
	parse_expression::composition result;
	result.valid = true;
	result.level = 0;

	for (int i = 0; i < (int)r.states.size(); i++) {
		result.compositions.push_back(export_composition(r.states[i], variables));
	}

	return result;
}

parse_expression::expression export_expression(const arithmetic::expression &expr, const ucs::variable_set &variables)
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
			if (expr.operations[i].operands[j].type == arithmetic::operand::neutral)
				add.arguments[j].constant = "gnd";
			else if (expr.operations[i].operands[j].type == arithmetic::operand::valid)
				add.arguments[j].constant = "vdd";
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
		add.arguments.push_back(parse_expression::argument("gnd"));
		return add;
	}
}

parse_expression::assignment export_assignment(const arithmetic::action &expr, const ucs::variable_set &variables)
{
	parse_expression::assignment result;
	result.valid = true;

	if (expr.channel != -1)
		result.names.push_back(export_variable_name(expr.channel, variables));
	if (expr.variable != -1)
		result.names.push_back(export_variable_name(expr.variable, variables));

	if (expr.expr.operations.size() > 0)
		result.expressions.push_back(export_expression(expr.expr, variables));

	if (expr.behavior == arithmetic::action::assign) {
		if (expr.expr.is_neutral()) {
			result.operation = "-";
			result.expressions.clear();
		} else if (expr.expr.is_valid()) {
			result.operation = "+";
			result.expressions.clear();
		} else {
			result.operation = "=";
		}
	} else if (expr.behavior == arithmetic::action::send)
	{
		result.operation = "!";
		if (expr.variable != -1)
			result.operation += "?";
	}
	else if (expr.behavior == arithmetic::action::receive)
	{
		result.operation = "?";
		if (expr.expr.operations.size() != 0)
			result.operation += "!";
	}

	return result;
}

parse_expression::composition export_composition(const arithmetic::parallel &expr, const ucs::variable_set &variables)
{
	parse_expression::composition result;
	result.valid = true;
	result.level = 1;

	for (int i = 0; i < (int)expr.actions.size(); i++)
	{
		if (expr.actions[i].variable < 0 and expr.actions[i].channel < 0)
			result.guards.push_back(export_expression(expr.actions[i].expr, variables));
		else
			result.literals.push_back(export_assignment(expr.actions[i], variables));
	}

	return result;
}

parse_expression::composition export_composition(const arithmetic::choice &expr, const ucs::variable_set &variables)
{
	parse_expression::composition result;
	result.valid = true;
	result.level = 0;

	for (int i = 0; i < (int)expr.terms.size(); i++) {
		result.compositions.push_back(export_composition(expr.terms[i], variables));
	}

	return result;
}
