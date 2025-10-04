#pragma once

#include <common/standard.h>
#include <common/net.h>

#include <parse/tokenizer.h>

#include <parse_expression/expression.h>
#include <parse_expression/assignment.h>
#include <parse_expression/composition.h>

#include <arithmetic/expression.h>
#include <arithmetic/action.h>

#include "support.h"

namespace arithmetic {

int import_net(string syntax, ucs::Netlist nets, tokenizer *tokens, bool auto_define);

template <int group, typename number_t=parse::number, typename instance_t=parse::instance>
struct ExpressionInterpreter {
	using argument = parse_expression::argument_t<group, number_t, instance_t>;
	using expression = parse_expression::expression_t<group, number_t, instance_t>;
	using assignment = parse_expression::assignment_t<group, number_t, instance_t>;
	using composition = parse_expression::composition_t<group, number_t, instance_t>;
	using operation = parse_expression::operation;

	ExpressionInterpreter(tokenizer *tokens = nullptr, bool auto_define = true) {
		this->tokens = tokens;
		this->auto_define = auto_define;
	}

	~ExpressionInterpreter() {
	}

	tokenizer *tokens;
	bool auto_define;

	virtual arithmetic::Expression import_unary(operation op, arithmetic::Expression expr) = 0;
	virtual arithmetic::Expression import_binary(operation op, arithmetic::Expression left, arithmetic::Expression right) = 0;
	virtual arithmetic::Expression import_group(operation op, vector<arithmetic::Expression> args) = 0;
	virtual arithmetic::Expression import_modifier(operation op, const vector<argument> &arguments, ucs::Netlist nets, int default_id) = 0;

	string import_constant(const argument &syntax);
	string import_constant(const expression &syntax);

	string import_net_name(const argument &syntax);
	string import_net_name(const expression &syntax);

	string import_literal(const argument &syntax);
	string import_literal(const expression &syntax);

	int import_net(const expression &syntax, ucs::Netlist nets, int default_id);

	arithmetic::Expression import_argument(const argument &syntax, ucs::Netlist nets, int default_id);
	vector<arithmetic::Expression> import_arguments(const vector<argument> &syntax, ucs::Netlist nets, int default_id);

	arithmetic::Expression import_members(const vector<argument> &syntax, ucs::Netlist nets, int default_id);
	vector<arithmetic::Expression> import_call(const vector<argument> &syntax, ucs::Netlist nets, int default_id);

	Expression import_expression(const expression &syntax, ucs::Netlist nets, int default_id);

	Action import_action(const assignment &syntax, ucs::Netlist nets, int default_id);
	Parallel import_parallel(const composition &syntax, ucs::Netlist nets, int default_id);
	Choice import_choice(const composition &syntax, ucs::Netlist nets, int default_id);

	State import_state(const assignment &syntax, ucs::Netlist nets, int default_id);
	State import_state(const composition &syntax, ucs::Netlist nets, int default_id);
};

// parse_expression::argument_t<group, number_t, instance_t>

template <int group, typename number_t, typename instance_t>
string ExpressionInterpreter<group, number_t, instance_t>::import_constant(const argument &syntax) {
	if (syntax.sub.valid) {
		return import_constant(syntax.sub);
	} else if (not syntax.literal.empty()) {
		if (tokens != nullptr) {
			tokens->internal("expected constant-valued expression", __FILE__, __LINE__);
		} else {
			internal("", "expected constant-valued expression", __FILE__, __LINE__);
		}
		return "0";
	}
	return syntax.constant;
}

template <int group, typename number_t, typename instance_t>
string ExpressionInterpreter<group, number_t, instance_t>::import_constant(const parse_expression::expression_t<group, number_t, instance_t> &syntax) {
	if (tokens != nullptr) {
		tokens->load(&syntax);
	}

	if (not syntax.valid or syntax.level < 0 or syntax.arguments.empty()) {
		if (tokens != nullptr) {
			tokens->internal("invalid expression", __FILE__, __LINE__);
		} else {
			internal("", "invaid expression", __FILE__, __LINE__);
		}
		return "0";
	}

	string result = "";
	if (syntax.operators.empty()) {
		result += import_constant(syntax.arguments[0]);
	} else {
		if (tokens != nullptr) {
			tokens->internal("sub expressions in constants not supported", __FILE__, __LINE__);
		} else {
			internal("", "sub expressions in constants not supported", __FILE__, __LINE__);
		}
		return "0";
	}

	return result;
}

template <int group, typename number_t, typename instance_t>
string ExpressionInterpreter<group, number_t, instance_t>::import_net_name(const parse_expression::argument_t<group, number_t, instance_t> &syntax) {
	if (syntax.sub.valid) {
		return import_net_name(syntax.sub);
	} else if (not syntax.literal.empty()) {
		return syntax.literal;
	}
	internal(syntax.constant, "expected instance", __FILE__, __LINE__);
	return "_";
}

template <int group, typename number_t, typename instance_t>
string ExpressionInterpreter<group, number_t, instance_t>::import_net_name(const parse_expression::expression_t<group, number_t, instance_t> &syntax) {
	if (tokens != nullptr) {
		tokens->load(&syntax);
	}

	if (not syntax.valid or syntax.level < 0 or syntax.arguments.empty()) {
		if (tokens != nullptr) {
			tokens->internal("invalid expression", __FILE__, __LINE__);
		} else {
			internal("", "invaid expression", __FILE__, __LINE__);
		}
		return "_";
	}

	string result = "";
	if (syntax.operators.empty()) {
		result += import_net_name(syntax.arguments[0]);
	} else if (syntax.precedence.at(syntax.level, syntax.operators[0]).is("", "[", ":", "]")) {
		result += import_net_name(syntax.arguments[0]) + syntax.precedence.at(syntax.level, syntax.operators[0]).trigger;
		for (int i = 1; i < (int)syntax.arguments.size(); i++) {
			if (i != 1) {
				result += syntax.precedence.at(syntax.level, syntax.operators[0]).infix;
			}
			result += import_constant(syntax.arguments[i]);
		}
		result += syntax.precedence.at(syntax.level, syntax.operators[0]).postfix;
	} else if (syntax.precedence.at(syntax.level, syntax.operators[0]).is("", ".", "", "")
			or syntax.precedence.at(syntax.level, syntax.operators[0]).is("", "::", "", "")) {
		result = import_net_name(syntax.arguments[0]);
		result += syntax.precedence.at(syntax.level, syntax.operators[0]).trigger;
		result += import_net_name(syntax.arguments[1]);
	} else {
		if (tokens != nullptr) {
			tokens->load(&syntax);
			tokens->internal("sub expressions in variable names not supported " + syntax.precedence.at(syntax.level, syntax.operators[0]).to_string(), __FILE__, __LINE__);
		} else {
			internal("", "sub expressions in variable names not supported " + syntax.precedence.at(syntax.level, syntax.operators[0]).to_string(), __FILE__, __LINE__);
		}
		return "_";
	}

	return result;
}

template <int group, typename number_t, typename instance_t>
string ExpressionInterpreter<group, number_t, instance_t>::import_literal(const parse_expression::argument_t<group, number_t, instance_t> &syntax) {
	if (syntax.sub.valid) {
		return import_literal(syntax.sub);
	} else if (syntax.literal != "") {
		return syntax.literal;
	}
	return "";
}

template <int group, typename number_t, typename instance_t>
string ExpressionInterpreter<group, number_t, instance_t>::import_literal(const parse_expression::expression_t<group, number_t, instance_t> &syntax) {
	if (not syntax.operators.empty() or syntax.arguments.size() != 1u) {
		return "";
	}
	return import_literal(syntax.arguments[0]);	
}

template <int group, typename number_t, typename instance_t>
int ExpressionInterpreter<group, number_t, instance_t>::import_net(const parse_expression::expression_t<group, number_t, instance_t> &syntax, ucs::Netlist nets, int default_id) {
	string name = import_net_name(syntax);
	if (default_id != 0) {
		name += "'" + ::to_string(default_id);
	}

	return arithmetic::import_net(name, nets, tokens, auto_define);
}

template <int group, typename number_t, typename instance_t>
Expression ExpressionInterpreter<group, number_t, instance_t>::import_argument(const parse_expression::argument_t<group, number_t, instance_t> &syntax, ucs::Netlist nets, int default_id) {
	if (syntax.sub.valid) {
		return import_expression(syntax.sub, nets, default_id);
	} else if (syntax.literal != "") {
		string name = syntax.literal;
		if (default_id != 0) {
			name += "'" + ::to_string(default_id);
		}
		return Expression::varOf(arithmetic::import_net(name, nets, tokens, auto_define));
	} else if (syntax.constant == "false") {
		return Expression::boolOf(false);
	} else if (syntax.constant == "true") {
		return Expression::boolOf(true);
	} else if (syntax.constant == "gnd") {
		return Expression::gnd();
	} else if (syntax.constant == "vdd") {
		return Expression::vdd();
	} else if (syntax.constant != "") {
		return Expression::intOf(atoi(syntax.constant.c_str()));
	}
	return Expression::X();
}

template <int group, typename number_t, typename instance_t>
vector<Expression> ExpressionInterpreter<group, number_t, instance_t>::import_arguments(const vector<parse_expression::argument_t<group, number_t, instance_t> > &syntax, ucs::Netlist nets, int default_id) {
	vector<Expression> result;
	for (size_t i = 0; i < syntax.size(); i++) {
		result.push_back(import_argument(syntax[i], nets, default_id));
	}
	return result;
}

template <int group, typename number_t, typename instance_t>
Expression ExpressionInterpreter<group, number_t, instance_t>::import_members(const vector<parse_expression::argument_t<group, number_t, instance_t> > &syntax, ucs::Netlist nets, int default_id) {
	Expression result;
	if (not syntax.empty()) {
		result = import_argument(syntax[0], nets, default_id);
	}
	for (size_t i = 1; i < syntax.size(); i++) {
		string lit = import_literal(syntax[i]);
		if (lit == "") {
			internal("", "member operator '.' expects literals", __FILE__, __LINE__);
			break;
		}
		result.push(arithmetic::Operation::MEMBER, {result.top, Operand::stringOf(lit)});
	}
	return result;
}

template <int group, typename number_t, typename instance_t>
vector<Expression> ExpressionInterpreter<group, number_t, instance_t>::import_call(const vector<parse_expression::argument_t<group, number_t, instance_t> > &syntax, ucs::Netlist nets, int default_id) {
	vector<Expression> result;
	if (not syntax.empty()) {
		string first = import_literal(syntax[0]);
		if (not first.empty()) {
			result.push_back(Expression::stringOf(first));
		} else {
			result.push_back(import_argument(syntax[0], nets, default_id));
		}
	}

	for (size_t i = 1; i < syntax.size(); i++) {
		result.push_back(import_argument(syntax[i], nets, default_id));
	}
	return result;
}

template <int group, typename number_t, typename instance_t>
Expression ExpressionInterpreter<group, number_t, instance_t>::import_expression(const parse_expression::expression_t<group, number_t, instance_t> &syntax, ucs::Netlist nets, int default_id) {
	if (tokens != NULL) {
		tokens->load(&syntax);
	}

	if (not syntax.precedence.isValidLevel(syntax.level)) {
		if (tokens != NULL) {
			tokens->error("unrecognized operation", __FILE__, __LINE__);
		} else {
			internal("", "unrecognized operation", __FILE__, __LINE__);
		}
		return arithmetic::Expression();
	}

	if (syntax.operators.empty()) {
		if (syntax.arguments.size() == 1u) {
			return import_argument(syntax.arguments[0], nets, default_id);
		} else if (tokens != NULL) {
			tokens->error("malformed expression", __FILE__, __LINE__);
		} else {
			internal("", "malformed expression", __FILE__, __LINE__);
		}
	}

	operation op = syntax.precedence.at(syntax.level, syntax.operators.back());
	if (syntax.precedence.isGroup(syntax.level)) {
		return import_group(op, import_arguments(syntax.arguments, nets, default_id));
	} else if (syntax.precedence.isModifier(syntax.level)) {
		return import_modifier(op, syntax.arguments, nets, default_id);
	} else if (syntax.precedence.isBinary(syntax.level) or syntax.precedence.isUnary(syntax.level)) {
		Expression result;
		if (not syntax.arguments.empty()) {
			result = import_argument(syntax.arguments[0], nets, default_id);
		}

		if (syntax.arguments.size() == 1u) {
			if (syntax.precedence.isUnary(syntax.level)) {
				for (int i = (int)syntax.operators.size()-1; i >= 0; i--) {
					parse_expression::operation op = syntax.precedence.at(syntax.level, syntax.operators[i]);

					result = import_unary(op, result);
				}
			}
		} else {
			for (size_t i = 1; i < syntax.arguments.size(); i++) {
				parse_expression::operation op = syntax.precedence.at(syntax.level, syntax.operators[i-1]);
				
				Expression sub = import_argument(syntax.arguments[i], nets, default_id);

				result = import_binary(op, result, sub);
			}
		}

		return result;
	}
	return Expression();
}

template <int group, typename number_t, typename instance_t>
Action ExpressionInterpreter<group, number_t, instance_t>::import_action(const parse_expression::assignment_t<group, number_t, instance_t> &syntax, ucs::Netlist nets, int default_id) {
	int region = default_id;
	if (syntax.region != "") {
		region = atoi(syntax.region.c_str());
	}

	//cout << "import action " << syntax.to_string("") << endl;

	Action result;
	if (syntax.operation.empty()) {
		result.lvalue = Expression::undef();
		if (syntax.lvalue[0].valid) {
			result.rvalue = import_expression(syntax.lvalue[0], nets, region);
		}
	} else if (syntax.operation == "+") {
		if (syntax.lvalue.size() > 0) {
			result.lvalue = import_expression(syntax.lvalue[0], nets, region);
		}
		result.rvalue = Expression::vdd();
	} else if (syntax.operation == "-") {
		if (syntax.lvalue.size() > 0) {
			result.lvalue = import_expression(syntax.lvalue[0], nets, region);
		}
		result.rvalue = Expression::gnd();
	} else if (syntax.operation == "=") {
		if (syntax.lvalue.size() > 0) {
			result.lvalue = import_expression(syntax.lvalue[0], nets, region);
		}
		if (syntax.rvalue.valid) {
			result.rvalue = import_expression(syntax.rvalue, nets, region);
		}
	}
	//cout << result.lvalue << " = " << result.rvalue << endl;

	return result;
}

template <int group, typename number_t, typename instance_t>
Parallel ExpressionInterpreter<group, number_t, instance_t>::import_parallel(const parse_expression::composition_t<group, number_t, instance_t> &syntax, ucs::Netlist nets, int default_id) {
	if (syntax.level == 0 and (syntax.literals.size() + syntax.guards.size() + syntax.compositions.size()) > 1u) {
		if (tokens != NULL) {
			tokens->load(&syntax);
			tokens->error("expected parallel composition", __FILE__, __LINE__);
		} else {
			error(syntax.to_string(), "expected parallel composition", __FILE__, __LINE__);
		}
		return Parallel();
	}

	int region = default_id;
	if (syntax.region != "") {
		region = atoi(syntax.region.c_str());
	}

	Parallel result;
	for (int i = 0; i < (int)syntax.literals.size(); i++) {
		result &= import_action(syntax.literals[i], nets, region);
	}

	for (int i = 0; i < (int)syntax.guards.size(); i++) {
		result &= Action(import_expression(syntax.guards[i], nets, region));
	}

	for (int i = 0; i < (int)syntax.compositions.size(); i++) {
		result &= import_parallel(syntax.compositions[i], nets, region);
	}
	return result;
}

template <int group, typename number_t, typename instance_t>
Choice ExpressionInterpreter<group, number_t, instance_t>::import_choice(const parse_expression::composition_t<group, number_t, instance_t> &syntax, ucs::Netlist nets, int default_id) {
	int region = default_id;
	if (syntax.region != "") {
		region = atoi(syntax.region.c_str());
	}

	Choice result(syntax.level != 0);

	for (int i = 0; i < (int)syntax.literals.size(); i++) {
		Action sub = import_action(syntax.literals[i], nets, region);
		if (syntax.level == 0) {
			result |= sub;
		} else {
			result &= sub;
		}
	}

	for (int i = 0; i < (int)syntax.guards.size(); i++) {
		Action sub(import_expression(syntax.guards[i], nets, region));
		if (syntax.level == 0) {
			result |= sub;
		} else {
			result &= sub;
		}
	}

	for (int i = 0; i < (int)syntax.compositions.size(); i++) {
		Choice sub = import_choice(syntax.compositions[i], nets, region);
		if (syntax.level == 0) {
			result |= sub;
		} else {
			result &= sub;
		}
	}

	return result;
}

template <int group, typename number_t, typename instance_t>
State ExpressionInterpreter<group, number_t, instance_t>::import_state(const parse_expression::assignment_t<group, number_t, instance_t> &syntax, ucs::Netlist nets, int default_id) {
	int region = default_id;
	if (syntax.region != "") {
		region = atoi(syntax.region.c_str());
	}

	if (syntax.lvalue.empty() or not syntax.lvalue[0].valid) {
		internal("", "malformed assignment for state '" + syntax.to_string() + "'", __FILE__, __LINE__);
	}

	// TODO(edward.bingham) figure out net types
	if (syntax.operation == "+") {
		return State(import_net(syntax.lvalue[0], nets, region), Value::vdd());
	} else if (syntax.operation == "-") {
		return State(import_net(syntax.lvalue[0], nets, region), Value::gnd());
	} else if (syntax.operation == "~") {
		return State(import_net(syntax.lvalue[0], nets, region), Value::X());
	} else if (syntax.operation == "=") {
		State result;
		int v = import_net(syntax.lvalue[0], nets, region);
		if (syntax.rvalue.operators.empty() and syntax.rvalue.arguments.size() == 1 and syntax.rvalue.arguments[0].constant != "") {
			if (syntax.rvalue.arguments[0].constant == "false") {
				result.set(v, false);
			} else if (syntax.rvalue.arguments[0].constant == "true") {
				result.set(v, true);
			} else if (syntax.rvalue.arguments[0].constant == "gnd") {
				result.set(v, Value::gnd());
			} else if (syntax.rvalue.arguments[0].constant == "vdd") {
				result.set(v, Value::vdd());
			} else if (syntax.rvalue.arguments[0].constant != "") {
				result.set(v, atoi(syntax.rvalue.arguments[0].constant.c_str()));
			} else {
				if (tokens != NULL) {
					tokens->load(&syntax.rvalue);
					tokens->error("unsupported expression", __FILE__, __LINE__);
				} else {
					error(syntax.rvalue.to_string(), "unsupported expression", __FILE__, __LINE__);
				}
				return State();
			}
		}
		return result;
	} else {
		if (tokens != NULL) {
			tokens->load(&syntax);
			tokens->error("unsupported operation", __FILE__, __LINE__);
		} else {
			error(syntax.to_string(), "unsupported operation", __FILE__, __LINE__);
		}

		return State();
	}
}

template <int group, typename number_t, typename instance_t>
State ExpressionInterpreter<group, number_t, instance_t>::import_state(const parse_expression::composition_t<group, number_t, instance_t> &syntax, ucs::Netlist nets, int default_id) {
	int region = default_id;
	if (syntax.region != "") {
		region = atoi(syntax.region.c_str());
	}

	if (syntax.level >= (int)syntax.precedence.size()) {
		if (tokens != NULL) {
			tokens->load(&syntax);
			tokens->error("unrecognized operation", __FILE__, __LINE__);
		} else {
			error(syntax.to_string(), "unrecognized operation", __FILE__, __LINE__);
		}
		return State();
	} else if (syntax.literals.size() == 0 && syntax.compositions.size() == 0) {
		return State();
	} else if (syntax.precedence[syntax.level] == ":" && syntax.literals.size() + syntax.compositions.size() > 1) {
		if (tokens != NULL) {
			tokens->load(&syntax);
			tokens->error("illegal disjunction", __FILE__, __LINE__);
		} else {
			error(syntax.to_string(), "illegal disjunction", __FILE__, __LINE__);
		}
		return State();
	}

	State result;

	for (int i = 0; i < (int)syntax.literals.size(); i++) {
		if (syntax.literals[i].valid) {
			result &= import_state(syntax.literals[i], nets, region);
		}
	}

	for (int i = 0; i < (int)syntax.compositions.size(); i++) {
		if (syntax.compositions[i].valid) {
			result &= import_state(syntax.compositions[i], nets, region);
		}
	}

	return result;
}


}
