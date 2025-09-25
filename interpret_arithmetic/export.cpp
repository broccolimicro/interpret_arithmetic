#include "export.h"
#include "support.h"
#include <arithmetic/algorithm.h>

namespace arithmetic {

parse_expression::expression export_field(string str) {
	static const auto op = parse_expression::expression::precedence.find(parse_expression::operation_set::MODIFIER, "", "[", ":", "]");
	if (op.level < 0 or op.index < 0) {
		internal("", "unable to find \"[]\" operator", __FILE__, __LINE__);
		return parse_expression::expression();
	}
 
	parse_expression::expression result;
	result.valid = true;
	result.level = op.level;

	string name = str;
	
	size_t open = name.find('[');
	if (open != string::npos) {
		name = str.substr(0u, open);
		result.operators.push_back(op.index);
	}

	result.arguments.push_back(parse_expression::argument::literalOf(name));
	while (open != string::npos and open < str.size()) {
		open += 1;
		size_t close = str.find(']', open);
		result.arguments.push_back(parse_expression::argument::constantOf(str.substr(open, close-open)));
		open = close+1;
	}

	return result;
}

parse_expression::expression export_member(string str) {
	static const auto op = parse_expression::expression::precedence.find(parse_expression::operation_set::MODIFIER, "", ".", "", "");
	if (op.level < 0 or op.index < 0 or str.empty()) {
		internal("", "unable to find \".\" operator", __FILE__, __LINE__);
		return parse_expression::expression();
	}

	size_t dot = str.rfind('.');
	if (dot != string::npos and dot < str.size()) {
		parse_expression::expression result;
		result.valid = true;
		result.level = op.level;
		result.arguments.push_back(export_member(str.substr(0, dot)));
		result.arguments.push_back(export_member(str.substr(dot+1)));
		result.operators.push_back(op.index);
		return result;
	}
	return export_field(str);
}

parse_expression::expression export_net(string str) {
	static const auto op = parse_expression::expression::precedence.find(parse_expression::operation_set::MODIFIER, "", "'", "", "");
	if (op.level < 0 or op.index < 0) {
		internal("", "unable to find \"'\" operator", __FILE__, __LINE__);
		return parse_expression::expression();
	}

	size_t tic = str.rfind('\'');
	if (tic != string::npos) {
		parse_expression::expression result;
		result.valid = true;
		result.level = op.level;
		result.operators.push_back(op.index);
		result.arguments.push_back(export_member(str.substr(0, tic)));
		result.arguments.push_back(parse_expression::argument::constantOf(str.substr(tic+1)));
		return result;
	}
	return export_member(str);
}


parse_expression::expression export_net(int uid, ucs::ConstNetlist nets) {
	string name = nets.netAt(uid);
	if (name.empty()) {
		return parse_expression::expression();
	}

	return export_net(name);
}


pair<int, int> export_operator(arithmetic::Operator op) {
	for (int i = 0; i < (int)parse_expression::expression::precedence.size(); i++) {
		for (int j = 0; j < (int)parse_expression::expression::precedence.at(i).size(); j++) {
			if (areSame(op, parse_expression::expression::precedence.at(i, j))) {
				return {i, j};
			}
		}
	}
	return {-1, -1};
}

string export_value(const Value &v) {
	if (v.isUndef()) {
		return "undef";
	} else if (v.isUnstable()) {
		return "unstable";
	} else if (v.isUnknown()) {
		return "unknown";
	} else if (v.type == Value::WIRE) {
		if (v.isNeutral()) {
			return "gnd";
		} else if (v.isValid()) {
			return "vdd";
		}
	} else if (v.type == Value::BOOL) {
		return ::to_string(v.bval);
	} else if (v.type == Value::INT) {
		return ::to_string(v.ival);
	} else if (v.type == Value::REAL) {
		return ::to_string(v.rval);
	} else if (v.type == Value::STRING) {
		return v.sval;
	}
	return "";
}

parse_expression::expression export_expression(const Value &v, ucs::ConstNetlist nets) {
	parse_expression::expression result;
	result.valid = true;
	if (v.isValid() and v.type == Value::ARRAY) {
		auto op = export_operator(arithmetic::Operator("[", "", ",", "]"));
		result.level = op.first;
		result.operators.push_back(op.second);
		for (size_t i = 0; i < v.arr.size(); i++) {
			result.arguments.push_back(parse_expression::argument(export_expression(v.arr[i], nets)));
		}
	} else if (v.isValid() and v.type == Value::STRUCT) {
		auto op = export_operator(arithmetic::Operator("{", "", ",", "}"));
		result.level = op.first;
		result.operators.push_back(op.second);
		for (size_t i = 0; i < v.arr.size(); i++) {
			result.arguments.push_back(parse_expression::argument(export_expression(v.arr[i], nets)));
		}
	} else {
		result.level = parse_expression::expression::precedence.size();
		result.arguments.push_back(parse_expression::argument::constantOf(export_value(v)));
	}
	return result;	
}

parse_expression::expression export_expression(const State &s, ucs::ConstNetlist nets)
{
	vector<parse_expression::expression> result;

	for (int i = 0; i < (int)s.values.size(); i++)
	{
		if (not s.values[i].isUnknown()) {
			parse_expression::expression add;
			add.valid = true;
			if (s.values[i].isNeutral()) {
				auto op = parse_expression::expression::precedence.find(parse_expression::operation_set::UNARY, "~", "", "", "");
				add.operators.push_back(op.index);
				add.level = op.level;
				add.arguments.resize(1);
				add.arguments[0].sub = export_net(i, nets);
			} else if (s.values[i].isValid()) {
				add.level = parse_expression::expression::precedence.size();
				add.arguments.resize(1);
				add.arguments[0].sub = export_net(i, nets);
			} else {
				auto op = parse_expression::expression::precedence.find(parse_expression::operation_set::BINARY, "", "", "==", "");
				add.operators.push_back(op.index);
				add.level = op.level;
				add.arguments.resize(2);
				add.arguments[0].sub = export_net(i, nets);
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
		auto op = parse_expression::expression::precedence.find(parse_expression::operation_set::BINARY, "", "", "&", "");
		add.operators.push_back(op.index);
		add.level = op.level;
		add.arguments.resize(result.size());
		for (int i = 0; i < (int)result.size(); i++) {
			add.arguments[i].sub = result[i];
		}
	} else {
		add.arguments.push_back(parse_expression::argument::constantOf("vdd"));
	}

	return add;
}


parse_expression::composition export_composition(const State &s, ucs::ConstNetlist nets)
{
	parse_expression::composition result;
	result.valid = true;
	result.level = 1;

	for (int i = 0; i < (int)s.values.size(); i++) {
		if (not s.values[i].isUnknown() and not s.values[i].isUndef()) {
			parse_expression::assignment assign;
			assign.valid = true;
			assign.lvalue.push_back(export_net(i, nets));
			if (s.values[i].isUnstable()) {
				assign.operation = "~";
			} else if (s.values[i].isNeutral()) {
				assign.operation = "-";
			} else if (s.values[i].type == Value::WIRE and s.values[i].isValid()) {
				assign.operation = "+";
			} else {
				assign.operation = "=";
				assign.rvalue = export_expression(s.values[i], nets);
			}
			result.literals.push_back(assign);
		}
	}

	return result;
}

parse_expression::composition export_composition(const Region &r, ucs::ConstNetlist nets)
{
	parse_expression::composition result;
	result.valid = true;
	result.level = 0;

	for (int i = 0; i < (int)r.states.size(); i++) {
		result.compositions.push_back(export_composition(r.states[i], nets));
	}

	return result;
}

parse_expression::argument export_argument(const vector<parse_expression::expression> &sub, Operand op, ucs::ConstNetlist nets) {
	parse_expression::argument result;
	if (op.isConst()) {
		result.constant = export_value(op.cnst);
	} else if (op.isVar()) {
		result.sub = export_net(op.index, nets);
	} else if (op.isExpr()) {
		result.sub = sub[op.index];
	} else if (op.isType()) {
		internal("", "unable to export type expression", __FILE__, __LINE__);
	} else {
		internal("", "unable to export undefined expression", __FILE__, __LINE__);
	}
	return result;
}

parse_expression::expression export_expression(const Expression &expr, ucs::ConstNetlist nets) {
	vector<parse_expression::expression> result;
	for (arithmetic::ConstUpIterator i(expr, {expr.top}); not i.done(); ++i) {
		parse_expression::expression add;
		add.valid = true;
		if (i->func == Operation::NEGATIVE) {
			auto op = export_operator(arithmetic::Operator("", "", "<", ""));

			add.level = op.first;
			add.operators.push_back(op.second);
			add.arguments.resize(2);
			add.arguments[0] = export_argument(result, i->operands[0], nets);
			add.arguments[1].constant = "0";
		} else if (i->func == Operation::VALIDITY) {
			auto op = export_operator(arithmetic::Operator("", "(", ",", ")"));

			add.level = op.first;
			add.operators.push_back(op.second);
			add.arguments.resize(2);
			add.arguments[0].constant = "valid";
			add.arguments[1] = export_argument(result, i->operands[0], nets);
		} else if (i->func == Operation::TRUTHINESS) {
			auto op = export_operator(arithmetic::Operator("", "(", ",", ")"));

			add.level = op.first;
			add.operators.push_back(op.second);
			add.arguments.resize(2);
			add.arguments[0].constant = "true";
			add.arguments[1] = export_argument(result, i->operands[0], nets);
		} else if (i->func == Operation::INVERSE) {
			auto op = export_operator(arithmetic::Operator("", "", "/", ""));

			add.level = op.first;
			add.operators.push_back(op.second);
			add.arguments.resize(2);
			add.arguments[1].constant = "1";
			add.arguments[0] = export_argument(result, i->operands[0], nets);
		} else if (i->func == Operation::IDENTITY) {
			auto op = export_operator(arithmetic::Operator("+", "", "", ""));

			add.level = op.first;
			add.arguments.resize(1);
			add.arguments[0] = export_argument(result, i->operands[0], nets);
		} else {
			auto op = export_operator(arithmetic::Operation::operators[i->func]);

			add.level = op.first;
			add.operators.push_back(op.second);
			add.arguments.resize(i->operands.size());
			for (int j = 0; j < (int)i->operands.size(); j++) {
				add.arguments[j] = export_argument(result, i->operands[j], nets);
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
			parse_expression::expression add;
			add.valid = true;
			add.arguments.push_back(parse_expression::argument::constantOf("gnd"));
			return add;
		}
	}

	auto op = export_operator(arithmetic::Operator("+", "", "", ""));
	
	parse_expression::expression add;
	add.valid = true;
	add.level = op.first;
	add.arguments.push_back(export_argument(result, expr.top, nets));
	return add;
}

parse_expression::assignment export_assignment(const Action &expr, ucs::ConstNetlist nets)
{
	parse_expression::assignment result;
	result.valid = true;

	if (not expr.lvalue.isUndef()) {
		result.lvalue.push_back(export_expression(expr.lvalue, nets));
	}

	// TODO(edward.bingham) we need type information about the lvalue here
	Operand top = expr.rvalue.top;
	if (top.isConst() and top.cnst.isNeutral()) {
		result.rvalue = parse_expression::expression();
		result.operation = "-";
	} else if (top.isConst() and top.cnst.isUnstable()) {
		result.rvalue = parse_expression::expression();
		result.operation = "~";
	} else if (top.isConst() and top.cnst.type == Value::WIRE and top.cnst.isValid()) {
		result.rvalue = parse_expression::expression();
		result.operation = "+";
	} else {
		result.rvalue = export_expression(expr.rvalue, nets);
		result.operation = "=";
	}

	return result;
}

parse_expression::composition export_composition(const Parallel &expr, ucs::ConstNetlist nets)
{
	parse_expression::composition result;
	result.valid = true;
	result.level = 1;

	for (int i = 0; i < (int)expr.actions.size(); i++)
	{
		if (expr.actions[i].lvalue.isUndef())
			result.guards.push_back(export_expression(expr.actions[i].rvalue, nets));
		else
			result.literals.push_back(export_assignment(expr.actions[i], nets));
	}

	return result;
}

parse_expression::composition export_composition(const Choice &expr, ucs::ConstNetlist nets)
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
