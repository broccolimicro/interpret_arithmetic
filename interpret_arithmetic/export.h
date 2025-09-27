#pragma once

#include <common/standard.h>
#include <common/net.h>

#include <parse_expression/expression.h>
#include <parse_expression/assignment.h>
#include <parse_expression/composition.h>

#include <arithmetic/expression.h>
#include <arithmetic/action.h>
#include <arithmetic/algorithm.h>

#include "support.h"

namespace arithmetic {

typedef string (*export_value_t)(const Value &);

string export_value(const Value &v);

template <typename expression>
expression export_field(string str) {
	static const auto op = expression::precedence.find(parse_expression::operation_set::MODIFIER, "", "[", ":", "]");
 
	expression result;
	result.valid = true;
	result.level = op.level;

	string name = str;
	
	size_t open = name.find('[');
	if (open != string::npos) {
		if (op.level < 0 or op.index < 0) {
			internal("", "unable to find \"[]\" operator", __FILE__, __LINE__);
			return expression();
		}
		name = str.substr(0u, open);
		result.operators.push_back(op.index);
	}

	result.arguments.push_back(expression::argument::literalOf(name));
	while (open != string::npos and open < str.size()) {
		open += 1;
		size_t close = str.find(']', open);
		result.arguments.push_back(expression::argument::constantOf(str.substr(open, close-open)));
		open = close+1;
	}

	return result;
}

template <typename expression>
expression export_member(string str) {
	static const auto op = expression::precedence.find(parse_expression::operation_set::MODIFIER, "", ".", "", "");

	size_t dot = str.rfind('.');
	if (dot != string::npos and dot < str.size()) {
		if (op.level < 0 or op.index < 0) {
			internal("", "unable to find \".\" operator", __FILE__, __LINE__);
			return expression();
		}
		expression result;
		result.valid = true;
		result.level = op.level;
		result.arguments.push_back(export_member<expression>(str.substr(0, dot)));
		result.arguments.push_back(export_member<expression>(str.substr(dot+1)));
		result.operators.push_back(op.index);
		return result;
	}
	return export_field<expression>(str);
}

template <typename expression>
expression export_net(string str) {
	static const auto op = expression::precedence.find(parse_expression::operation_set::MODIFIER, "", "'", "", "");

	size_t tic = str.rfind('\'');
	if (tic != string::npos) {
		if (op.level < 0 or op.index < 0) {
			internal("", "unable to find \"'\" operator", __FILE__, __LINE__);
			return expression();
		}
		expression result;
		result.valid = true;
		result.level = op.level;
		result.operators.push_back(op.index);
		result.arguments.push_back(export_member<expression>(str.substr(0, tic)));
		result.arguments.push_back(expression::argument::constantOf(str.substr(tic+1)));
		return result;
	}
	return export_member<expression>(str);
}

template <typename expression>
expression export_net(int uid, ucs::ConstNetlist nets) {
	string name = nets.netAt(uid);
	if (name.empty()) {
		return expression();
	}

	return export_net<expression>(name);
}

template <typename expression>
pair<int, int> export_operator(arithmetic::Operator op) {
	for (int i = 0; i < (int)expression::precedence.size(); i++) {
		for (int j = 0; j < (int)expression::precedence.at(i).size(); j++) {
			if (areSame(op, expression::precedence.at(i, j))) {
				return {i, j};
			}
		}
	}
	return {-1, -1};
}

template <typename expression>
expression export_expression(const Value &v, export_value_t export_value_f=export_value) {
	expression result;
	result.valid = true;
	if (v.isValid() and v.type == Value::ARRAY) {
		auto op = export_operator<expression>(arithmetic::Operator("[", "", ",", "]"));
		result.level = op.first;
		result.operators.push_back(op.second);
		for (size_t i = 0; i < v.arr.size(); i++) {
			result.arguments.push_back(typename expression::argument(export_expression<expression>(v.arr[i], export_value_f)));
		}
	} else if (v.isValid() and v.type == Value::STRUCT) {
		auto op = export_operator<expression>(arithmetic::Operator("{", "", ",", "}"));
		result.level = op.first;
		result.operators.push_back(op.second);
		for (size_t i = 0; i < v.arr.size(); i++) {
			result.arguments.push_back(typename expression::argument(export_expression<expression>(v.arr[i], export_value_f)));
		}
	} else {
		result.level = expression::expression::precedence.size();
		result.arguments.push_back(expression::argument::constantOf(export_value_f(v)));
	}
	return result;	
}

template <typename expression>
expression export_expression(const State &s, ucs::ConstNetlist nets, export_value_t export_value_f=export_value)
{
	vector<expression> result;

	for (int i = 0; i < (int)s.values.size(); i++)
	{
		if (not s.values[i].isUnknown()) {
			expression add;
			add.valid = true;
			if (s.values[i].isNeutral()) {
				auto op = expression::precedence.find(parse_expression::operation_set::UNARY, "~", "", "", "");
				add.operators.push_back(op.index);
				add.level = op.level;
				add.arguments.resize(1);
				add.arguments[0].sub = export_net<expression>(i, nets);
			} else if (s.values[i].isValid()) {
				add.level = expression::precedence.size();
				add.arguments.resize(1);
				add.arguments[0].sub = export_net<expression>(i, nets);
			} else {
				auto op = expression::precedence.find(parse_expression::operation_set::BINARY, "", "", "==", "");
				add.operators.push_back(op.index);
				add.level = op.level;
				add.arguments.resize(2);
				add.arguments[0].sub = export_net<expression>(i, nets);
				add.arguments[1].constant = export_value_f(s.values[i]);
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
		auto op = expression::precedence.find(parse_expression::operation_set::BINARY, "", "", "&", "");
		add.operators.push_back(op.index);
		add.level = op.level;
		add.arguments.resize(result.size());
		for (int i = 0; i < (int)result.size(); i++) {
			add.arguments[i].sub = result[i];
		}
	} else {
		add.arguments.push_back(expression::argument::constantOf("vdd"));
	}

	return add;
}

template <typename composition>
composition export_composition(const State &s, ucs::ConstNetlist nets, export_value_t export_value_f=export_value)
{
	composition result;
	result.valid = true;
	result.level = 1;

	for (int i = 0; i < (int)s.values.size(); i++) {
		if (not s.values[i].isUnknown() and not s.values[i].isUndef()) {
			typename composition::assignment assign;
			assign.valid = true;
			assign.lvalue.push_back(export_net<typename composition::expression>(i, nets));
			if (s.values[i].isUnstable()) {
				assign.operation = "~";
			} else if (s.values[i].isNeutral()) {
				assign.operation = "-";
			} else if (s.values[i].type == Value::WIRE and s.values[i].isValid()) {
				assign.operation = "+";
			} else {
				assign.operation = "=";
				assign.rvalue = export_expression<typename composition::expression>(s.values[i], export_value_f);
			}
			result.literals.push_back(assign);
		}
	}

	return result;
}

template <typename composition>
composition export_composition(const Region &r, ucs::ConstNetlist nets, export_value_t export_value_f=export_value)
{
	composition result;
	result.valid = true;
	result.level = 0;

	for (int i = 0; i < (int)r.states.size(); i++) {
		result.compositions.push_back(export_composition<composition>(r.states[i], nets, export_value_f));
	}

	return result;
}

template <typename expression>
typename expression::argument export_argument(const vector<expression> &sub, Operand op, ucs::ConstNetlist nets, export_value_t export_value_f=export_value) {
	typename expression::argument result;
	if (op.isConst()) {
		result.constant = export_value_f(op.cnst);
	} else if (op.isVar()) {
		result.sub = export_net<expression>(op.index, nets);
	} else if (op.isExpr()) {
		result.sub = sub[op.index];
	} else if (op.isType()) {
		internal("", "unable to export type expression", __FILE__, __LINE__);
	} else {
		internal("", "unable to export undefined expression", __FILE__, __LINE__);
	}
	return result;
}

template <typename expression>
expression export_expression(const Expression &expr, ucs::ConstNetlist nets, export_value_t export_value_f=export_value) {
	//cout << "exporting " << expr.to_string(true) << endl;

	vector<expression> result;
	for (arithmetic::ConstUpIterator i(expr, {expr.top}); not i.done(); ++i) {
		expression add;
		add.valid = true;
		if (i->func == Operation::NEGATIVE) {
			auto op = export_operator<expression>(arithmetic::Operator("", "", "<", ""));

			add.level = op.first;
			add.operators.push_back(op.second);
			add.arguments.resize(2);
			add.arguments[0] = export_argument<expression>(result, i->operands[0], nets, export_value_f);
			add.arguments[1].constant = "0";
		} else if (i->func == Operation::VALIDITY) {
			auto op = export_operator<expression>(arithmetic::Operator("", "(", ",", ")"));

			add.level = op.first;
			add.operators.push_back(op.second);
			add.arguments.resize(2);
			add.arguments[0].constant = "valid";
			add.arguments[1] = export_argument<expression>(result, i->operands[0], nets, export_value_f);
		} else if (i->func == Operation::TRUTHINESS) {
			auto op = export_operator<expression>(arithmetic::Operator("", "(", ",", ")"));

			add.level = op.first;
			add.operators.push_back(op.second);
			add.arguments.resize(2);
			add.arguments[0].constant = "true";
			add.arguments[1] = export_argument<expression>(result, i->operands[0], nets, export_value_f);
		} else if (i->func == Operation::INVERSE) {
			auto op = export_operator<expression>(arithmetic::Operator("", "", "/", ""));

			add.level = op.first;
			add.operators.push_back(op.second);
			add.arguments.resize(2);
			add.arguments[1].constant = "1";
			add.arguments[0] = export_argument<expression>(result, i->operands[0], nets, export_value_f);
		} else if (i->func == Operation::IDENTITY) {
			auto op = export_operator<expression>(arithmetic::Operator("+", "", "", ""));

			add.level = op.first;
			add.arguments.resize(1);
			add.arguments[0] = export_argument<expression>(result, i->operands[0], nets, export_value_f);
		} else {
			auto op = export_operator<expression>(arithmetic::Operation::operators[i->func]);

			add.level = op.first;
			add.operators.push_back(op.second);
			add.arguments.resize(i->operands.size());
			for (int j = 0; j < (int)i->operands.size(); j++) {
				add.arguments[j] = export_argument<expression>(result, i->operands[j], nets, export_value_f);
			}
		}
		if (i->exprIndex >= result.size()) {
			result.resize(i->exprIndex+1);
		}
		result[i->exprIndex] = add;
	}

	if (expr.top.isExpr()) {
		if (expr.top.index < result.size()) {
			return result[expr.top.index];
		} else {
			expression add;
			add.valid = true;
			add.arguments.push_back(expression::argument::constantOf("gnd"));
			return add;
		}
	}

	auto op = export_operator<expression>(arithmetic::Operator("+", "", "", ""));
	
	expression add;
	add.valid = true;
	add.level = op.first;
	add.arguments.push_back(export_argument<expression>(result, expr.top, nets, export_value_f));
	return add;
}

template <typename assignment>
assignment export_assignment(const Action &expr, ucs::ConstNetlist nets, export_value_t export_value_f=export_value)
{
	assignment result;
	result.valid = true;

	if (not expr.lvalue.isUndef()) {
		result.lvalue.push_back(export_expression<typename assignment::expression>(expr.lvalue, nets, export_value_f));
	}

	// TODO(edward.bingham) we need type information about the lvalue here
	Operand top = expr.rvalue.top;
	if (top.isConst() and top.cnst.isNeutral()) {
		result.rvalue = typename assignment::expression();
		result.operation = "-";
	} else if (top.isConst() and top.cnst.isUnstable()) {
		result.rvalue = typename assignment::expression();
		result.operation = "~";
	} else if (top.isConst() and top.cnst.type == Value::WIRE and top.cnst.isValid()) {
		result.rvalue = typename assignment::expression();
		result.operation = "+";
	} else {
		result.rvalue = export_expression<typename assignment::expression>(expr.rvalue, nets, export_value_f);
		result.operation = "=";
	}

	return result;
}

template <typename composition>
composition export_composition(const Parallel &expr, ucs::ConstNetlist nets, export_value_t export_value_f=export_value) {
	composition result;
	result.valid = true;
	result.level = 1;

	for (int i = 0; i < (int)expr.actions.size(); i++)
	{
		if (expr.actions[i].lvalue.isUndef())
			result.guards.push_back(export_expression<typename composition::expression>(expr.actions[i].rvalue, nets, export_value_f));
		else
			result.literals.push_back(export_assignment<typename composition::assignment>(expr.actions[i], nets, export_value_f));
	}

	return result;
}

template <typename composition>
composition export_composition(const Choice &expr, ucs::ConstNetlist nets, export_value_t export_value_f=export_value) {
	composition result;
	result.valid = true;
	result.level = 0;

	for (int i = 0; i < (int)expr.terms.size(); i++) {
		result.compositions.push_back(export_composition<composition>(expr.terms[i], nets, export_value_f));
	}

	return result;
}

}
