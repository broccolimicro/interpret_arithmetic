/*
 * export.cpp
 *
 *  Created on: Feb 6, 2015
 *      Author: nbingham
 */

#include "export_verilog.h"

namespace parse_verilog {

slice export_slice(int lb, int ub, const ucs::variable_set &variables)
{
	slice result;
	result.valid = true;
	result.lower = shared_ptr<parse::syntax>(new expression(export_expression(arithmetic::value(lb), variables)));
	result.upper = shared_ptr<parse::syntax>(new expression(export_expression(arithmetic::value(ub), variables)));
	return result;
}

member_name export_member_name(ucs::instance instance, const ucs::variable_set &variables)
{
	member_name result;
	result.valid = true;
	result.name = instance.name;
	for (int i = 0; i < (int)instance.slice.size(); i++)
		result.slices.push_back(export_slice(instance.slice[i], instance.slice[i], variables));
	return result;
}

variable_name export_variable_name(ucs::variable variable, const ucs::variable_set &variables)
{
	variable_name result;
	result.valid = true;
	for (int i = 0; i < (int)variable.name.size(); i++)
		result.names.push_back(export_member_name(variable.name[i], variables));
	return result;
}

variable_name export_variable_name(int variable, const ucs::variable_set &variables)
{
	variable_name result;
	if (variable >= 0 && variable < (int)variables.nodes.size()) {
		result = export_variable_name(variables.nodes[variable], variables);
	} else if (variable < 0) {
		result = export_variable_name(ucs::variable("_"+to_string(-variable-1)), variables);
	}
	return result;
}

expression export_expression(const arithmetic::value &v) {
	expression result;
	result.valid = true;
	result.level = expression::get_level("");
	if (v.data == arithmetic::value::neutral) {
		result.arguments.push_back(argument("0"));
	} else if (v.data == arithmetic::value::valid) {
		result.arguments.push_back(argument("1"));
	} else if (v.data == arithmetic::value::unstable) {
		result.arguments.push_back(argument("X"));
	} else {
		result.arguments.push_back(argument(::to_string(v.data)));
	}
	return result;	
}

expression export_expression(const arithmetic::state &s, const ucs::variable_set &variables)
{
	vector<expression> result;

	for (int i = 0; i < (int)s.values.size(); i++)
	{
		if (s.values[i].data != arithmetic::value::unknown) {
			expression add;
			add.valid = true;
			if (s.values[i].data == arithmetic::value::neutral) {
				add.operations.push_back("~");
				add.level = expression::get_level(add.operations[0]);
				add.arguments.resize(1);
				add.arguments[0].literal = export_variable_name(i, variables);
			} else if (s.values[i].data == arithmetic::value::valid) {
				add.operations.push_back("");
				add.level = expression::get_level(add.operations[0]);
				add.arguments.resize(1);
				add.arguments[0].literal = export_variable_name(i, variables);
			} else {
				add.operations.push_back("==");
				add.level = expression::get_level(add.operations[0]);
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

	expression add;
	add.valid = true;

	if (result.size() > 1) {
		add.operations.push_back("&");
		add.level = expression::get_level(add.operations[0]);
		add.arguments.resize(result.size());
		for (int i = 0; i < (int)result.size(); i++) {
			add.arguments[i].sub = result[i];
		}
	} else {
		add.arguments.push_back(argument("1"));
	}

	return add;
}

string export_operation(int op) {
	switch (op)
	{
	case 0: return "~";  // bitwise not
	case 1: return "+";
	case 2: return "-";
	case 3: return "valid"; // boolean check
	case 4: return "!";  // boolean not
	case 5: return "|"; // bitwise or
	case 6: return "&"; // bitwise and
	case 7: return "^";  // bitwise xor
	case 8: return "==";
	case 9: return "!=";
	case 10: return "<";
	case 11: return ">";
	case 12: return "<=";
	case 13: return ">=";
	case 14: return "<<";
	case 15: return ">>";
	case 16: return "+";
	case 17: return "-";
	case 18: return "*";
	case 19: return "/";
	case 20: return "%";
	case 21: return "recv";
	case 22: return "&&"; // boolean or
	case 23: return "||"; // boolean and
	default: return "";
	}
}

expression export_expression(const arithmetic::expression &expr, const ucs::variable_set &variables)
{
	vector<expression> result;

	for (int i = 0; i < (int)expr.operations.size(); i++)
	{
		expression add;
		add.valid = true;
		add.operations.push_back(export_operation(expr.operations[i].func));
		add.level = expression::get_level(add.operations[0]);
		add.arguments.resize(expr.operations[i].operands.size());
		for (int j = 0; j < (int)expr.operations[i].operands.size(); j++)
		{
			if (expr.operations[i].operands[j].type == arithmetic::operand::neutral)
				add.arguments[j].constant = "0";
			else if (expr.operations[i].operands[j].type == arithmetic::operand::valid)
				add.arguments[j].constant = "1";
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
		expression add;
		add.valid = true;
		add.arguments.push_back(argument("0"));
		return add;
	}
}

}
