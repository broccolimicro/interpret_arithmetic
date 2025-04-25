/*
 * export.cpp
 *
 *  Created on: Feb 6, 2015
 *      Author: nbingham
 */

#include "export.h"

namespace arithmetic {

parse_ucs::variable_name export_net(int uid, ConstNetlist nets) {
	parse_ucs::variable_name result;
	result.valid = true;
	result.names.push_back(parse_ucs::member_name());
	result.names.back().valid = true;
	arithmetic::Net net = nets.netAt(uid);
	result.names.back().name = net.first;
	result.region = ::to_string(net.second);
	return result;
}

string export_operator(int op) {
	if (op == Operation::BITWISE_NOT) {
		return "!";
	} else if (op == Operation::IDENTITY) {
		return "+";
	} else if (op == Operation::NEGATION) {
		return "-";
	} else if (op == Operation::VALIDITY) {
		return "(bool)";
	} else if (op == Operation::BOOLEAN_NOT) {
		return "~";
	} else if (op == Operation::INVERSE) {
		return "1/";
	} else if (op == Operation::BITWISE_OR) {
		return "||";
	} else if (op == Operation::BITWISE_AND) {
		return "&&";
	} else if (op == Operation::BITWISE_XOR) {
		return "^^";
	} else if (op == Operation::EQUAL) {
		return "==";
	} else if (op == Operation::NOT_EQUAL) {
		return "~=";
	} else if (op == Operation::LESS) {
		return "<";
	} else if (op == Operation::GREATER) {
		return ">";
	} else if (op == Operation::LESS_EQUAL) {
		return "<=";
	} else if (op == Operation::GREATER_EQUAL) {
		return ">=";
	} else if (op == Operation::SHIFT_LEFT) {
		return "<<";
	} else if (op == Operation::SHIFT_RIGHT) {
		return ">>";
	} else if (op == Operation::ADD) {
		return "+";
	} else if (op == Operation::SUBTRACT) {
		return "-";
	} else if (op == Operation::MULTIPLY) {
		return "*";
	} else if (op == Operation::DIVIDE) {
		return "/";
	} else if (op == Operation::MOD) {
		return "%";
	} else if (op == Operation::BOOLEAN_OR) {
		return "|";
	} else if (op == Operation::BOOLEAN_AND) {
		return "&";
	} else if (op == Operation::BOOLEAN_XOR) {
		return "^";
	} else if (op == Operation::ARRAY) {
		return ",";
	} else if (op == Operation::CALL) {
		return "(";
	}
	return "";
}

string export_value(const Value &v) {
	if (v.type == Value::BOOL) {
		if (v.isNeutral()) {
			return "gnd";
		} else if (v.isValid()) {
			return "vdd";
		} else if (v.isUnstable()) {
			return "unstable";
		} else if (v.isUnknown()) {
			return "undefined";
		}
	} else if (v.type == Value::INT) {
		return ::to_string(v.ival);
	} else if (v.type == Value::REAL) {
		return ::to_string(v.rval);
	}
	return "";
}

parse_expression::expression export_expression(const Value &v, ConstNetlist nets) {
	parse_expression::expression result;
	result.valid = true;
	result.level = parse_expression::expression::get_level("");
	result.arguments.push_back(parse_expression::argument(export_value(v)));
	return result;	
}

parse_expression::expression export_expression(const State &s, ConstNetlist nets)
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
				add.arguments[0].literal.valid = true;
				add.arguments[0].literal = export_net(i, nets);
			} else if (s.values[i].isValid()) {
				add.operations.push_back("");
				add.level = parse_expression::expression::get_level(add.operations[0]);
				add.arguments.resize(1);
				add.arguments[0].literal.valid = true;
				add.arguments[0].literal = export_net(i, nets);
			} else {
				add.operations.push_back("==");
				add.level = parse_expression::expression::get_level(add.operations[0]);
				add.arguments.resize(2);
				add.arguments[0].literal.valid = true;
				add.arguments[0].literal = export_net(i, nets);
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


parse_expression::composition export_composition(const State &s, ConstNetlist nets)
{
	parse_expression::composition result;
	result.valid = true;
	result.level = 1;

	for (int i = 0; i < (int)s.values.size(); i++) {
		if (not s.values[i].isUnknown()) {
			parse_expression::assignment assign;
			assign.valid = true;
			assign.names.push_back(export_net(i, nets));
			if (s.values[i].isNeutral()) {
				assign.operation = "-";
			} else if (s.values[i].isValid()) {
				assign.operation = "+";
			} else if (s.values[i].isUnstable()) {
				assign.operation = "~";
			} else {
				assign.operation = "=";
				assign.expressions.push_back(export_expression(s.values[i], nets));
			}
			result.literals.push_back(assign);
		}
	}

	return result;
}

parse_expression::composition export_composition(const Region &r, ConstNetlist nets)
{
	parse_expression::composition result;
	result.valid = true;
	result.level = 0;

	for (int i = 0; i < (int)r.states.size(); i++) {
		result.compositions.push_back(export_composition(r.states[i], nets));
	}

	return result;
}

parse_expression::expression export_expression(const Expression &expr, ConstNetlist nets) {
	vector<parse_expression::expression> result;
	for (int i = 0; i < (int)expr.operations.size(); i++) {
		parse_expression::expression add;
		add.valid = true;
		add.operations.push_back(export_operator(expr.operations[i].func));
		add.level = parse_expression::expression::get_level(add.operations[0]);
		if (expr.operations[i].func == Operation::CALL) {
			for (int j = 2; j < (int)expr.operations[i].operands.size(); j++) {
				add.operations.push_back(parse_expression::expression::precedence[add.level].symbols[1]);
			}
			add.operations.push_back(parse_expression::expression::precedence[add.level].symbols[2]);
		}
		add.arguments.resize(expr.operations[i].operands.size());
		for (int j = 0; j < (int)expr.operations[i].operands.size(); j++) {
			if (expr.operations[i].operands[j].isConst()) {
				add.arguments[j].constant = export_value(expr.operations[i].operands[j].cnst);
			} else if (expr.operations[i].operands[j].isVar()) {
				add.arguments[j].literal.valid = true;
				add.arguments[j].literal = export_net(expr.operations[i].operands[j].index, nets);
			} else if (expr.operations[i].operands[j].isExpr()) {
				add.arguments[j].sub = result[expr.operations[i].operands[j].index];
			}
		}

		result.push_back(add);
	}

	if (result.size() > 0) {
		return result.back();
	} else {
		parse_expression::expression add;
		add.valid = true;
		add.arguments.push_back(parse_expression::argument("gnd"));
		return add;
	}
}

parse_expression::assignment export_assignment(const Action &expr, ConstNetlist nets)
{
	parse_expression::assignment result;
	result.valid = true;

	if (expr.variable != -1)
		result.names.push_back(export_net(expr.variable, nets));

	if (expr.expr.operations.size() > 0)
		result.expressions.push_back(export_expression(expr.expr, nets));

	if (expr.expr.isNeutral()) {
		result.operation = "-";
		result.expressions.clear();
	} else if (expr.expr.isValid()) {
		result.operation = "+";
		result.expressions.clear();
	} else {
		result.operation = "=";
	}

	return result;
}

parse_expression::composition export_composition(const Parallel &expr, ConstNetlist nets)
{
	parse_expression::composition result;
	result.valid = true;
	result.level = 1;

	for (int i = 0; i < (int)expr.actions.size(); i++)
	{
		if (expr.actions[i].variable < 0)
			result.guards.push_back(export_expression(expr.actions[i].expr, nets));
		else
			result.literals.push_back(export_assignment(expr.actions[i], nets));
	}

	return result;
}

parse_expression::composition export_composition(const Choice &expr, ConstNetlist nets)
{
	parse_expression::composition result;
	result.valid = true;
	result.level = 0;

	for (int i = 0; i < (int)expr.terms.size(); i++) {
		result.compositions.push_back(export_composition(expr.terms[i], nets));
	}

	return result;
}

}
