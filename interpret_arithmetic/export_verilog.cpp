/*
 * export.cpp
 *
 *  Created on: Feb 6, 2015
 *      Author: nbingham
 */

#include "export_verilog.h"

namespace parse_verilog {

variable_name export_variable_name(int variable, arithmetic::ConstNetlist nets)
{
	auto net = nets.netAt(variable);

	variable_name result;
	result.valid = true;
	member_name name;
	name.valid = true;
	name.name = net.first;
	result.names.push_back(name);
	if (net.second != 0) {
		result.region = ::to_string(net.second);
	}
	return result;
}

string export_value(const arithmetic::Value &v) {
	if (v.isNeutral()) {
		return "0";
	} else if (v.isValid()) {
		return "1";
	} else if (v.isUnstable()) {
		return "X";
	} else if (v.isUnknown()) {
		return "U";
	} else if (v.type == arithmetic::Value::INT) {
		return ::to_string(v.ival);
	} else if (v.type == arithmetic::Value::REAL) {
		return ::to_string(v.rval);
	}
	return "";
}

expression export_expression(const arithmetic::Value &v) {
	expression result;
	result.valid = true;
	result.level = expression::get_level("");
	result.arguments.push_back(argument(export_value(v)));
	return result;	
}

expression export_expression(const arithmetic::State &s, arithmetic::ConstNetlist nets)
{
	vector<expression> result;

	for (int i = 0; i < (int)s.values.size(); i++)
	{
		if (not s.values[i].isUnknown()) {
			expression add;
			add.valid = true;
			if (s.values[i].isNeutral()) {
				add.operations.push_back("~");
				add.level = expression::get_level(add.operations[0]);
				add.arguments.resize(1);
				add.arguments[0].literal = export_variable_name(i, nets);
			} else if (s.values[i].isValid()) {
				add.operations.push_back("");
				add.level = expression::get_level(add.operations[0]);
				add.arguments.resize(1);
				add.arguments[0].literal = export_variable_name(i, nets);
			} else {
				add.operations.push_back("==");
				add.level = expression::get_level(add.operations[0]);
				add.arguments.resize(2);
				add.arguments[0].literal = export_variable_name(i, nets);
				add.arguments[1].constant = export_value(s.values[i]);
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

expression export_expression(const arithmetic::Expression &expr, arithmetic::ConstNetlist nets)
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
			if (expr.operations[i].operands[j].isConst())
				add.arguments[j].constant = export_value(expr.operations[i].operands[j].cnst);
			else if (expr.operations[i].operands[j].isVar())
				add.arguments[j].literal = export_variable_name(expr.operations[i].operands[j].index, nets);
			else if (expr.operations[i].operands[j].isExpr())
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
