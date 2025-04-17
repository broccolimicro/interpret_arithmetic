/*
 * export.cpp
 *
 *  Created on: Feb 6, 2015
 *      Author: nbingham
 */

#include "export.h"
#include <interpret_ucs/export.h>

string export_operator(int op) {
	if (op == arithmetic::Operation::BITWISE_NOT) {
		return "!";
	} else if (op == arithmetic::Operation::IDENTITY) {
		return "+";
	} else if (op == arithmetic::Operation::NEGATION) {
		return "-";
	} else if (op == arithmetic::Operation::VALIDITY) {
		return "(bool)";
	} else if (op == arithmetic::Operation::BOOLEAN_NOT) {
		return "~";
	} else if (op == arithmetic::Operation::INVERSE) {
		return "1/";
	} else if (op == arithmetic::Operation::BITWISE_OR) {
		return "||";
	} else if (op == arithmetic::Operation::BITWISE_AND) {
		return "&&";
	} else if (op == arithmetic::Operation::BITWISE_XOR) {
		return "^^";
	} else if (op == arithmetic::Operation::EQUAL) {
		return "==";
	} else if (op == arithmetic::Operation::NOT_EQUAL) {
		return "~=";
	} else if (op == arithmetic::Operation::LESS) {
		return "<";
	} else if (op == arithmetic::Operation::GREATER) {
		return ">";
	} else if (op == arithmetic::Operation::LESS_EQUAL) {
		return "<=";
	} else if (op == arithmetic::Operation::GREATER_EQUAL) {
		return ">=";
	} else if (op == arithmetic::Operation::SHIFT_LEFT) {
		return "<<";
	} else if (op == arithmetic::Operation::SHIFT_RIGHT) {
		return ">>";
	} else if (op == arithmetic::Operation::ADD) {
		return "+";
	} else if (op == arithmetic::Operation::SUBTRACT) {
		return "-";
	} else if (op == arithmetic::Operation::MULTIPLY) {
		return "*";
	} else if (op == arithmetic::Operation::DIVIDE) {
		return "/";
	} else if (op == arithmetic::Operation::MOD) {
		return "%";
	} else if (op == arithmetic::Operation::BOOLEAN_OR) {
		return "|";
	} else if (op == arithmetic::Operation::BOOLEAN_AND) {
		return "&";
	} else if (op == arithmetic::Operation::BOOLEAN_XOR) {
		return "^";
	} else if (op == arithmetic::Operation::ARRAY) {
		return ",";
	}
	return "";
}

string export_value(const arithmetic::Value &v) {
	if (v.type == arithmetic::Value::BOOL) {
		if (v.isNeutral()) {
			return "gnd";
		} else if (v.isValid()) {
			return "vdd";
		} else if (v.isUnstable()) {
			return "unstable";
		} else if (v.isUnknown()) {
			return "undefined";
		}
	} else if (v.type == arithmetic::Value::INT) {
		return ::to_string(v.ival);
	} else if (v.type == arithmetic::Value::REAL) {
		return ::to_string(v.rval);
	}
	return "";
}

parse_expression::expression export_expression(const arithmetic::Value &v, const ucs::variable_set &variables) {
	parse_expression::expression result;
	result.valid = true;
	result.level = parse_expression::expression::get_level("");
	result.arguments.push_back(parse_expression::argument(export_value(v)));
	return result;	
}

parse_expression::expression export_expression(const arithmetic::State &s, const ucs::variable_set &variables)
{
	vector<parse_expression::expression> result;

	for (int i = 0; i < (int)s.values.size(); i++)
	{
		if (not s.values[i].isUnknown()) {
			parse_expression::expression add;
			add.valid = true;
			if (s.values[i].isNeutral()) {
				add.operations.push_back("~");
				add.level = parse_expression::expression::get_level(add.operations[0]);
				add.arguments.resize(1);
				add.arguments[0].literal = export_variable_name(i, variables);
			} else if (s.values[i].isValid()) {
				add.operations.push_back("");
				add.level = parse_expression::expression::get_level(add.operations[0]);
				add.arguments.resize(1);
				add.arguments[0].literal = export_variable_name(i, variables);
			} else {
				add.operations.push_back("==");
				add.level = parse_expression::expression::get_level(add.operations[0]);
				add.arguments.resize(2);
				add.arguments[0].literal = export_variable_name(i, variables);
				add.arguments[1].constant = export_value(s.values[i]);
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


parse_expression::composition export_composition(const arithmetic::State &s, const ucs::variable_set &variables)
{
	parse_expression::composition result;
	result.valid = true;
	result.level = 1;

	for (int i = 0; i < (int)s.values.size(); i++) {
		if (not s.values[i].isUnknown()) {
			parse_expression::assignment assign;
			assign.valid = true;
			assign.names.push_back(export_variable_name(i, variables));
			if (s.values[i].isNeutral()) {
				assign.operation = "-";
			} else if (s.values[i].isValid()) {
				assign.operation = "+";
			} else if (s.values[i].isUnstable()) {
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

parse_expression::composition export_composition(const arithmetic::Region &r, const ucs::variable_set &variables)
{
	parse_expression::composition result;
	result.valid = true;
	result.level = 0;

	for (int i = 0; i < (int)r.states.size(); i++) {
		result.compositions.push_back(export_composition(r.states[i], variables));
	}

	return result;
}

parse_expression::expression export_expression(const arithmetic::Expression &expr, const ucs::variable_set &variables)
{
	vector<parse_expression::expression> result;

	for (int i = 0; i < (int)expr.operations.size(); i++)
	{
		parse_expression::expression add;
		add.valid = true;
		add.operations.push_back(export_operator(expr.operations[i].func));
		add.level = parse_expression::expression::get_level(add.operations[0]);
		add.arguments.resize(expr.operations[i].operands.size());
		for (int j = 0; j < (int)expr.operations[i].operands.size(); j++)
		{
			if (expr.operations[i].operands[j].isConst())
				add.arguments[j].constant = export_value(expr.operations[i].operands[j].cnst);
			else if (expr.operations[i].operands[j].isVar())
				add.arguments[j].literal = export_variable_name(expr.operations[i].operands[j].index, variables);
			else if (expr.operations[i].operands[j].isExpr())
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

parse_expression::assignment export_assignment(const arithmetic::Action &expr, const ucs::variable_set &variables)
{
	parse_expression::assignment result;
	result.valid = true;

	if (expr.channel != -1)
		result.names.push_back(export_variable_name(expr.channel, variables));
	if (expr.variable != -1)
		result.names.push_back(export_variable_name(expr.variable, variables));

	if (expr.expr.operations.size() > 0)
		result.expressions.push_back(export_expression(expr.expr, variables));

	if (expr.behavior == arithmetic::Action::ASSIGN) {
		if (expr.expr.isNeutral()) {
			result.operation = "-";
			result.expressions.clear();
		} else if (expr.expr.isValid()) {
			result.operation = "+";
			result.expressions.clear();
		} else {
			result.operation = "=";
		}
	} else if (expr.behavior == arithmetic::Action::SEND)
	{
		result.operation = "!";
		if (expr.variable != -1)
			result.operation += "?";
	}
	else if (expr.behavior == arithmetic::Action::RECEIVE)
	{
		result.operation = "?";
		if (expr.expr.operations.size() != 0)
			result.operation += "!";
	}

	return result;
}

parse_expression::composition export_composition(const arithmetic::Parallel &expr, const ucs::variable_set &variables)
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

parse_expression::composition export_composition(const arithmetic::Choice &expr, const ucs::variable_set &variables)
{
	parse_expression::composition result;
	result.valid = true;
	result.level = 0;

	for (int i = 0; i < (int)expr.terms.size(); i++) {
		result.compositions.push_back(export_composition(expr.terms[i], variables));
	}

	return result;
}
