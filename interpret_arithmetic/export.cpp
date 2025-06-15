#include "export.h"
#include "support.h"

namespace arithmetic {

parse_expression::expression export_field(string str) {
	static const pair<int, int> op = parse_expression::expression::find(parse_expression::operation_set::MODIFIER, "", "[", ":", "]");
	if (op.first < 0 or op.second < 0) {
		internal("", "unable to find \"[]\" operator", __FILE__, __LINE__);
		return parse_expression::expression();
	}
 
	parse_expression::expression result;
	result.valid = true;
	result.level = op.first;

	string name = str;
	
	size_t open = name.find('[');
	if (open != string::npos) {
		name = str.substr(0u, open);
		result.operators.push_back(op.second);
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
	static const pair<int, int> op = parse_expression::expression::find(parse_expression::operation_set::MODIFIER, "", ".", "", "");
	if (op.first < 0 or op.second < 0 or str.empty()) {
		internal("", "unable to find \".\" operator", __FILE__, __LINE__);
		return parse_expression::expression();
	}

	size_t dot = str.rfind('.');
	if (dot != string::npos and dot < str.size()) {
		parse_expression::expression result;
		result.valid = true;
		result.level = op.first;
		result.arguments.push_back(export_member(str.substr(0, dot)));
		result.arguments.push_back(export_member(str.substr(dot+1)));
		result.operators.push_back(op.second);
		return result;
	}
	return export_field(str);
}

parse_expression::expression export_net(string str) {
	static const pair<int, int> op = parse_expression::expression::find(parse_expression::operation_set::MODIFIER, "", "'", "", "");
	if (op.first < 0 or op.second < 0) {
		internal("", "unable to find \"'\" operator", __FILE__, __LINE__);
		return parse_expression::expression();
	}

	size_t tic = str.rfind('\'');
	if (tic != string::npos) {
		parse_expression::expression result;
		result.valid = true;
		result.level = op.first;
		result.operators.push_back(op.second);
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
		for (int j = 0; j < (int)parse_expression::expression::precedence[i].symbols.size(); j++) {
			if (areSame(op, parse_expression::expression::precedence[i].symbols[j])) {
				return {i, j};
			}
		}
	}
	return {-1, -1};
}

string export_value(const Value &v) {
	if (v.type == Value::BOOL) {
		if (v.isNeutral()) {
			return "false";
		} else if (v.isValid()) {
			return "true";
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

parse_expression::expression export_expression(const Value &v, ucs::ConstNetlist nets) {
	parse_expression::expression result;
	result.valid = true;
	result.level = parse_expression::expression::precedence.size();
	result.arguments.push_back(parse_expression::argument::constantOf(export_value(v)));
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
				auto op = parse_expression::expression::find(parse_expression::operation_set::UNARY, "~", "", "", "");
				add.operators.push_back(op.second);
				add.level = op.first;
				add.arguments.resize(1);
				add.arguments[0].sub = export_net(i, nets);
			} else if (s.values[i].isValid()) {
				add.level = parse_expression::expression::precedence.size();
				add.arguments.resize(1);
				add.arguments[0].sub = export_net(i, nets);
			} else {
				auto op = parse_expression::expression::find(parse_expression::operation_set::BINARY, "", "", "==", "");
				add.operators.push_back(op.second);
				add.level = op.first;
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
		auto op = parse_expression::expression::find(parse_expression::operation_set::BINARY, "", "", "&", "");
		add.operators.push_back(op.second);
		add.level = op.first;
		add.arguments.resize(result.size());
		for (int i = 0; i < (int)result.size(); i++) {
			add.arguments[i].sub = result[i];
		}
	} else {
		add.arguments.push_back(parse_expression::argument::constantOf("true"));
	}

	return add;
}


parse_expression::composition export_composition(const State &s, ucs::ConstNetlist nets)
{
	parse_expression::composition result;
	result.valid = true;
	result.level = 1;

	for (int i = 0; i < (int)s.values.size(); i++) {
		if (not s.values[i].isUnknown()) {
			parse_expression::assignment assign;
			assign.valid = true;
			assign.lvalue.push_back(export_net(i, nets));
			if (s.values[i].type == Value::BOOL and s.values[i].isNeutral()) {
				assign.operation = "-";
			} else if (s.values[i].type == Value::BOOL and s.values[i].isValid()) {
				assign.operation = "+";
			} else if (s.values[i].type == Value::BOOL and s.values[i].isUnstable()) {
				assign.operation = "~";
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

parse_expression::expression export_expression(const Expression &expr, ucs::ConstNetlist nets) {
	vector<parse_expression::expression> result;
	for (int i = 0; i < (int)expr.operations.size(); i++) {
		parse_expression::expression add;
		add.valid = true;
		if (expr.operations[i].func == Operation::NEGATIVE) {
			auto op = export_operator(arithmetic::Operator("", "", "<", ""));

			add.level = op.first;
			add.operators.push_back(op.second);
			add.arguments.resize(2);
			if (expr.operations[i].operands[0].isConst()) {
				add.arguments[0].constant = export_value(expr.operations[i].operands[0].cnst);
			} else if (expr.operations[i].operands[0].isVar()) {
				add.arguments[0].sub = export_net(expr.operations[i].operands[0].index, nets);
			} else if (expr.operations[i].operands[0].isExpr()) {
				add.arguments[0].sub = result[expr.operations[i].operands[0].index];
			}
			add.arguments[1].constant = "0";
		} else if (expr.operations[i].func == Operation::VALIDITY) {
			auto op = export_operator(arithmetic::Operator("(bool)", "", "", ""));

			add.level = op.first;
			add.operators.push_back(op.second);
			add.arguments.resize(1);
			if (expr.operations[i].operands[0].isConst()) {
				add.arguments[0].constant = export_value(expr.operations[i].operands[0].cnst);
			} else if (expr.operations[i].operands[0].isVar()) {
				add.arguments[0].sub = export_net(expr.operations[i].operands[0].index, nets);
			} else if (expr.operations[i].operands[0].isExpr()) {
				add.arguments[0].sub = result[expr.operations[i].operands[0].index];
			}
		} else if (expr.operations[i].func == Operation::INVERSE) {
			auto op = export_operator(arithmetic::Operator("", "", "/", ""));

			add.level = op.first;
			add.operators.push_back(op.second);
			add.arguments.resize(2);
			add.arguments[1].constant = "1";
			if (expr.operations[i].operands[0].isConst()) {
				add.arguments[0].constant = export_value(expr.operations[i].operands[0].cnst);
			} else if (expr.operations[i].operands[0].isVar()) {
				add.arguments[0].sub = export_net(expr.operations[i].operands[0].index, nets);
			} else if (expr.operations[i].operands[0].isExpr()) {
				add.arguments[0].sub = result[expr.operations[i].operands[0].index];
			}
		} else if (expr.operations[i].func == Operation::IDENTITY) {
			auto op = export_operator(arithmetic::Operator("+", "", "", ""));

			add.level = op.first;
			add.arguments.resize(1);
			if (expr.operations[i].operands[0].isConst()) {
				add.arguments[0].constant = export_value(expr.operations[i].operands[0].cnst);
			} else if (expr.operations[i].operands[0].isVar()) {
				add.arguments[0].sub = export_net(expr.operations[i].operands[0].index, nets);
			} else if (expr.operations[i].operands[0].isExpr()) {
				add.arguments[0].sub = result[expr.operations[i].operands[0].index];
			}
		} else {
			auto op = export_operator(arithmetic::Operation::operators[expr.operations[i].func]);

			add.level = op.first;
			add.operators.push_back(op.second);
			add.arguments.resize(expr.operations[i].operands.size());
			for (int j = 0; j < (int)expr.operations[i].operands.size(); j++) {
				if (expr.operations[i].operands[j].isConst()) {
					add.arguments[j].constant = export_value(expr.operations[i].operands[j].cnst);
				} else if (expr.operations[i].operands[j].isVar()) {
					add.arguments[j].sub = export_net(expr.operations[i].operands[j].index, nets);
				} else if (expr.operations[i].operands[j].isExpr()) {
					add.arguments[j].sub = result[expr.operations[i].operands[j].index];
				}
			}
		}
		result.push_back(add);
	}

	if (result.size() > 0) {
		return result.back();
	} else {
		parse_expression::expression add;
		add.valid = true;
		add.arguments.push_back(parse_expression::argument::constantOf("false"));
		return add;
	}
}

parse_expression::assignment export_assignment(const Action &expr, ucs::ConstNetlist nets)
{
	parse_expression::assignment result;
	result.valid = true;

	if (expr.variable != -1)
		result.lvalue.push_back(export_net(expr.variable, nets));

	if (expr.expr.operations.size() > 0)
		result.rvalue = export_expression(expr.expr, nets);

	// TODO(edward.bingham) we need type information about the lvalue here
	bool isBool = (expr.expr.operations.empty() or
		(expr.expr.operations.back().func == arithmetic::Operation::IDENTITY
		and expr.expr.operations.back().operands.size() == 1u
		and expr.expr.operations.back().operands[0].isConst()
		and expr.expr.operations.back().operands[0].cnst.type == Value::BOOL));

	if (isBool and expr.expr.isNeutral()) {
		result.operation = "-";
		result.rvalue = parse_expression::expression();
	} else if (isBool and expr.expr.isValid()) {
		result.operation = "+";
		result.rvalue = parse_expression::expression();
	} else if (isBool and expr.expr.operations.back().operands[0].cnst.isUnstable()) {
		result.operation = "~";
		result.rvalue = parse_expression::expression();
	} else {
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
		if (expr.actions[i].variable < 0)
			result.guards.push_back(export_expression(expr.actions[i].expr, nets));
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
