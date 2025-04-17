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
	if (v.type == arithmetic::Value::BOOL) {
		if (v.isNeutral()) {
			return "0";
		} else if (v.isValid()) {
			return "1";
		} else if (v.isUnstable()) {
			return "X";
		} else {
			return "U";
		}
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
	if (op == arithmetic::Operation::BITWISE_NOT) {
		return "~";
	} else if (op == arithmetic::Operation::IDENTITY) {
		return "+";
	} else if (op == arithmetic::Operation::NEGATION) {
		return "-";
	} else if (op == arithmetic::Operation::VALIDITY) {
		return "valid";
	} else if (op == arithmetic::Operation::BOOLEAN_NOT) {
		return "!";
	} else if (op == arithmetic::Operation::INVERSE) {
		return "1/";
	} else if (op == arithmetic::Operation::BITWISE_OR) {
		return "|";
	} else if (op == arithmetic::Operation::BITWISE_AND) {
		return "&";
	} else if (op == arithmetic::Operation::BITWISE_XOR) {
		return "^";
	} else if (op == arithmetic::Operation::EQUAL) {
		return "==";
	} else if (op == arithmetic::Operation::NOT_EQUAL) {
		return "!=";
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
		return "||";
	} else if (op == arithmetic::Operation::BOOLEAN_AND) {
		return "&&";
	} else if (op == arithmetic::Operation::ARRAY) {
		return ",";
	}
	return "";
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
