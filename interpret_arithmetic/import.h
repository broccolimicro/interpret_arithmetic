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
arithmetic::Operation::OpType import_operator(parse_expression::operation op);

template <int group, typename number_t, typename instance_t>
string import_constant(const parse_expression::argument_t<group, number_t, instance_t> &syntax, tokenizer *tokens);

template <int group, typename number_t, typename instance_t>
Expression import_expression(const parse_expression::expression_t<group, number_t, instance_t> &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define);

template <int group, typename number_t, typename instance_t>
string import_constant(const parse_expression::expression_t<group, number_t, instance_t> &syntax, tokenizer *tokens) {
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
		result += import_constant(syntax.arguments[0], tokens);
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
string import_constant(const parse_expression::argument_t<group, number_t, instance_t> &syntax, tokenizer *tokens) {
	if (syntax.sub.valid) {
		return import_constant(syntax.sub, tokens);
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
string import_net_name(const parse_expression::expression_t<group, number_t, instance_t> &syntax, tokenizer *tokens);

template <int group, typename number_t, typename instance_t>
string import_net_name(const parse_expression::argument_t<group, number_t, instance_t> &syntax, tokenizer *tokens) {
	if (syntax.sub.valid) {
		return import_net_name(syntax.sub, tokens);
	} else if (not syntax.literal.empty()) {
		return syntax.literal;
	}
	internal(syntax.constant, "expected instance", __FILE__, __LINE__);
	return "_";
}

template <int group, typename number_t, typename instance_t>
string import_net_name(const parse_expression::expression_t<group, number_t, instance_t> &syntax, tokenizer *tokens) {
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
		result += import_net_name(syntax.arguments[0], tokens);
	} else if (syntax.precedence.at(syntax.level, syntax.operators[0]).is("", "[", ":", "]")) {
		result += import_net_name(syntax.arguments[0], tokens) + syntax.precedence.at(syntax.level, syntax.operators[0]).trigger;
		for (int i = 1; i < (int)syntax.arguments.size(); i++) {
			if (i != 1) {
				result += syntax.precedence.at(syntax.level, syntax.operators[0]).infix;
			}
			result += import_constant(syntax.arguments[i], tokens);
		}
		result += syntax.precedence.at(syntax.level, syntax.operators[0]).postfix;
	} else if (syntax.precedence.at(syntax.level, syntax.operators[0]).is("", ".", "", "")
			or syntax.precedence.at(syntax.level, syntax.operators[0]).is("", "::", "", "")) {
		result = import_net_name(syntax.arguments[0], tokens);
		result += syntax.precedence.at(syntax.level, syntax.operators[0]).trigger;
		result += import_net_name(syntax.arguments[1], tokens);
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
int import_net(const parse_expression::expression_t<group, number_t, instance_t> &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define) {
	string name = import_net_name(syntax, tokens);
	if (default_id != 0) {
		name += "'" + ::to_string(default_id);
	}

	return import_net(name, nets, tokens, auto_define);
}

template <int group, typename number_t, typename instance_t>
State import_state(const parse_expression::assignment_t<group, number_t, instance_t> &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define)
{
	int region = default_id;
	if (syntax.region != "") {
		region = atoi(syntax.region.c_str());
	}

	if (syntax.lvalue.empty() or not syntax.lvalue[0].valid) {
		internal("", "malformed assignment for state '" + syntax.to_string() + "'", __FILE__, __LINE__);
	}

	// TODO(edward.bingham) figure out net types
	if (syntax.operation == "+") {
		return State(import_net(syntax.lvalue[0], nets, region, tokens, auto_define), Value::vdd());
	} else if (syntax.operation == "-") {
		return State(import_net(syntax.lvalue[0], nets, region, tokens, auto_define), Value::gnd());
	} else if (syntax.operation == "~") {
		return State(import_net(syntax.lvalue[0], nets, region, tokens, auto_define), Value::X());
	} else if (syntax.operation == "=") {
		State result;
		int v = import_net(syntax.lvalue[0], nets, region, tokens, auto_define);
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
State import_state(const parse_expression::composition_t<group, number_t, instance_t> &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define)
{
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
			result &= import_state(syntax.literals[i], nets, region, tokens, auto_define);
		}
	}

	for (int i = 0; i < (int)syntax.compositions.size(); i++) {
		if (syntax.compositions[i].valid) {
			result &= import_state(syntax.compositions[i], nets, region, tokens, auto_define);
		}
	}

	return result;
}

template <int group, typename number_t, typename instance_t>
string argument_literal(const parse_expression::argument_t<group, number_t, instance_t> &syntax);

template <int group, typename number_t, typename instance_t>
string argument_literal(const parse_expression::expression_t<group, number_t, instance_t> &syntax) {
	if (not syntax.operators.empty() or syntax.arguments.size() != 1u) {
		return "";
	}
	return argument_literal<group, number_t, instance_t>(syntax.arguments[0]);	
}

template <int group, typename number_t, typename instance_t>
string argument_literal(const parse_expression::argument_t<group, number_t, instance_t> &syntax) {
	if (syntax.sub.valid) {
		return argument_literal<group, number_t, instance_t>(syntax.sub);
	} else if (syntax.literal != "") {
		return syntax.literal;
	}
	return "";
}

template <int group, typename number_t, typename instance_t>
Expression import_argument(const parse_expression::argument_t<group, number_t, instance_t> &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define) {
	if (syntax.sub.valid) {
		return import_expression(syntax.sub, nets, default_id, tokens, auto_define);
	} else if (syntax.literal != "") {
		string name = syntax.literal;
		if (default_id != 0) {
			name += "'" + ::to_string(default_id);
		}
		return Expression::varOf(import_net(name, nets, tokens, auto_define));
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
Expression import_expression(const parse_expression::expression_t<group, number_t, instance_t> &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define) {
	int region = default_id;

	if (tokens != NULL) {
		tokens->load(&syntax);
	}

	if (not syntax.precedence.isValidLevel(syntax.level)) {
		if (tokens != NULL) {
			tokens->error("unrecognized operation", __FILE__, __LINE__);
		} else {
			error(syntax.to_string(), "unrecognized operation", __FILE__, __LINE__);
		}
		return Expression::boolOf(false);
	}

	bool err = false;
	Expression result;
	if (not syntax.operators.empty()
		and syntax.precedence.isModifier(syntax.level)
		and syntax.precedence.at(syntax.level, syntax.operators.back()).is("", "!", "", "")) {
		vector<Expression> sub;
		arithmetic::Operation::OpType memb = import_operator(parse_expression::operation("", ".", "", ""));
		arithmetic::Operation::OpType call = import_operator(parse_expression::operation("", "(", ",", ")"));
		if (call < 0 or memb < 0) {
			err = true;
		}
		if (not err) {
			sub.push_back(arithmetic::Expression(memb, {import_argument(syntax.arguments[0], nets, region, tokens, auto_define), Expression::stringOf("send")}));
			for (int i = 1; i < (int)syntax.arguments.size() and not err; i++) {
				sub.push_back(import_argument(syntax.arguments[i], nets, region, tokens, auto_define));
			}
			result = arithmetic::Expression(call, sub);
		}
	} else if (not syntax.operators.empty()
		and syntax.precedence.isUnary(syntax.level)
		and syntax.precedence.at(syntax.level, syntax.operators.back()).is("", "", "", "?")) {
		vector<Expression> sub;
		arithmetic::Operation::OpType memb = import_operator(parse_expression::operation("", ".", "", ""));
		arithmetic::Operation::OpType call = import_operator(parse_expression::operation("", "(", ",", ")"));
		if (call < 0 or memb < 0) {
			err = true;
		}
		if (not err) {
			sub.push_back(arithmetic::Expression(memb, {import_argument(syntax.arguments[0], nets, region, tokens, auto_define), Expression::stringOf("recv")}));
			for (int i = 1; i < (int)syntax.arguments.size() and not err; i++) {
				sub.push_back(import_argument(syntax.arguments[i], nets, region, tokens, auto_define));
			}
			result = arithmetic::Expression(call, sub);
		}
	} else if (not syntax.operators.empty()
		and syntax.precedence.isUnary(syntax.level)
		and syntax.precedence.at(syntax.level, syntax.operators.back()).is("#", "", "", "")) {
		vector<Expression> sub;
		arithmetic::Operation::OpType memb = import_operator(parse_expression::operation("", ".", "", ""));
		arithmetic::Operation::OpType call = import_operator(parse_expression::operation("", "(", ",", ")"));
		if (call < 0 or memb < 0) {
			err = true;
		}
		if (not err) {
			sub.push_back(arithmetic::Expression(memb, {import_argument(syntax.arguments[0], nets, region, tokens, auto_define), Expression::stringOf("peek")}));
			for (int i = 1; i < (int)syntax.arguments.size() and not err; i++) {
				sub.push_back(import_argument(syntax.arguments[i], nets, region, tokens, auto_define));
			}
			result = arithmetic::Expression(call, sub);
		}
	} else if (not syntax.operators.empty()
		and syntax.precedence.isModifier(syntax.level)
		and syntax.precedence.at(syntax.level, syntax.operators.back()).is("", "'", "", "")) {
		string cnst = import_constant(syntax.arguments.back(), tokens);
		result = import_argument(syntax.arguments[0], nets, atoi(cnst.c_str()), tokens, auto_define);
	} else if (not syntax.operators.empty()
		and syntax.precedence.isModifier(syntax.level)
		and syntax.precedence.at(syntax.level, syntax.operators.back()).is("", ".", "", "")) {
		arithmetic::Operation::OpType op = import_operator(syntax.precedence.at(syntax.level, syntax.operators[0]));
		if (op < 0) {
			err = true;
		} else {
			string cnst = syntax.arguments.back().to_string();
			result = arithmetic::Expression(op, {import_argument(syntax.arguments[0], nets, default_id, tokens, auto_define), Expression::stringOf(cnst)});
		}
	// DESIGN(edward.bingham) This moves "this" into the first
	// argument of the function. So "a.b.c(d, e) becomes c(a.b,
	// d, e). This seems like a reasonable way to simplify
	// things, and follows the early style of c++ function names.
	// But I can see how it could create ambiguities for
	// overloaded functions.
	} else if (not syntax.operators.empty()
		and syntax.precedence.isModifier(syntax.level)
		and syntax.precedence.at(syntax.level, syntax.operators.back()).is("", "(", ",", ")")) {
		vector<Expression> sub;
		arithmetic::Operation::OpType op = import_operator(syntax.precedence.at(syntax.level, syntax.operators[0]));
		arithmetic::Operation::OpType memb = import_operator(parse_expression::operation("", ".", "", ""));
		if (op < 0) {
			err = true;
		}

		// Detect function call string
		//TODO: detect (((((myfunc)))))(ab), where func_name is recrusively buried
		string first = "";
		if (not syntax.arguments.empty()) {
			first = argument_literal(syntax.arguments[0]);
		}

		if (not first.empty()) {
			sub.push_back(Expression::stringOf(first));
		} else {
			sub.push_back(import_argument(syntax.arguments[0], nets, region, tokens, auto_define));
		}
		for (int i = 1; i < (int)syntax.arguments.size() and not err; i++) {
			sub.push_back(import_argument(syntax.arguments[i], nets, region, tokens, auto_define));
		}

		if (not sub.empty() and sub[0].top.isConst() and sub[0].top.cnst.type == Value::STRING) {
			if (sub[0].top.cnst.sval == "valid") {
				if (sub.size() > 2u) {
					error("", "valid() function expects 1 argument, found " + ::to_string(sub.size()-1), __FILE__, __LINE__);
				}
				op = arithmetic::Operation::VALIDITY;
				sub.erase(sub.begin());
			} else if (sub[0].top.cnst.sval == "true") {
				if (sub.size() > 2u) {
					error("", "true() function expects 1 argument, found " + ::to_string(sub.size()-1), __FILE__, __LINE__);
				}
				op = arithmetic::Operation::TRUTHINESS;
				sub.erase(sub.begin());
			}
		} else if (not sub.empty() and sub[0].top.isExpr() and sub[0].getExpr(sub[0].top.index)->func == memb) {
			Operation op = *sub[0].getExpr(sub[0].top.index);
			arithmetic::Operand name = op.operands.back();
			op.func = arithmetic::Operation::OpType::IDENTITY;
			op.operands.pop_back();
			sub[0].setExpr(op);
			sub.insert(sub.begin(), name);
		}
		result = arithmetic::Expression(op, sub);
	// END DESIGN
	} else if (not syntax.operators.empty()
		and (syntax.precedence.isModifier(syntax.level)
			or syntax.precedence.isGroup(syntax.level))) {
		vector<Expression> sub;
		arithmetic::Operation::OpType op = import_operator(syntax.precedence.at(syntax.level, syntax.operators[0]));
		if (op < 0) {
			err = true;
		}
		for (int i = 0; i < (int)syntax.arguments.size() and not err; i++) {
			sub.push_back(import_argument(syntax.arguments[i], nets, region, tokens, auto_define));
		}
		result = arithmetic::Expression(op, sub);
	} else {
		for (int i = 0; i < (int)syntax.arguments.size() and not err; i++) {
			Expression sub = import_argument(syntax.arguments[i], nets, region, tokens, auto_define);
			if (i == 0) {
				result = sub;
			} else {
				arithmetic::Operation::OpType op = import_operator(syntax.precedence.at(syntax.level, syntax.operators[i-1]));
				if (op < 0) {
					err = true;
				} else {
					result = Expression(op, {result, sub});
				}
			}
		}

		if (syntax.arguments.size() == 1) {
			if (syntax.precedence.isUnary(syntax.level)) {
				for (int i = (int)syntax.operators.size()-1; i >= 0 and not err; i--) {
					arithmetic::Operation::OpType op = import_operator(syntax.precedence.at(syntax.level, syntax.operators[i]));
					if (op < 0) {
						err = true;
					} else {
						result.push(op, {result.top});
					}
				}
			} else {
				for (int i = 0; i < (int)syntax.operators.size() and not err; i++) {
					arithmetic::Operation::OpType op = import_operator(syntax.precedence.at(syntax.level, syntax.operators[i]));
					
					if (op < 0) {
						err = true;
					} else {
						result.push(op, {result.top});
					}
				}
			}
		}
	}

	if (err) {
		if (tokens != NULL) {
			tokens->load(&syntax);
			tokens->error("unrecognized operator", __FILE__, __LINE__);
		} else {
			error(syntax.to_string(), "unrecognized operator", __FILE__, __LINE__);
		}
		return Expression(false);
	}

	return result;
}

template <int group, typename number_t, typename instance_t>
Action import_action(const parse_expression::assignment_t<group, number_t, instance_t> &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define) {
	int region = default_id;
	if (syntax.region != "") {
		region = atoi(syntax.region.c_str());
	}

	//cout << "import action " << syntax.to_string("") << endl;

	Action result;
	if (syntax.operation.empty()) {
		result.lvalue = Expression::undef();
		if (syntax.lvalue[0].valid) {
			result.rvalue = import_expression(syntax.lvalue[0], nets, region, tokens, auto_define);
		}
	} else if (syntax.operation == "+") {
		if (syntax.lvalue.size() > 0) {
			result.lvalue = import_expression(syntax.lvalue[0], nets, region, tokens, auto_define);
		}
		result.rvalue = Expression::vdd();
	} else if (syntax.operation == "-") {
		if (syntax.lvalue.size() > 0) {
			result.lvalue = import_expression(syntax.lvalue[0], nets, region, tokens, auto_define);
		}
		result.rvalue = Expression::gnd();
	} else if (syntax.operation == "=") {
		if (syntax.lvalue.size() > 0) {
			result.lvalue = import_expression(syntax.lvalue[0], nets, region, tokens, auto_define);
		}
		if (syntax.rvalue.valid) {
			result.rvalue = import_expression(syntax.rvalue, nets, region, tokens, auto_define);
		}
	}
	//cout << result.lvalue << " = " << result.rvalue << endl;

	return result;
}

template <int group, typename number_t, typename instance_t>
Parallel import_parallel(const parse_expression::composition_t<group, number_t, instance_t> &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define) {
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
		result &= import_action(syntax.literals[i], nets, region, tokens, auto_define);
	}

	for (int i = 0; i < (int)syntax.guards.size(); i++) {
		result &= Action(import_expression(syntax.guards[i], nets, region, tokens, auto_define));
	}

	for (int i = 0; i < (int)syntax.compositions.size(); i++) {
		result &= import_parallel(syntax.compositions[i], nets, region, tokens, auto_define);
	}
	return result;
}

template <int group, typename number_t, typename instance_t>
Choice import_choice(const parse_expression::composition_t<group, number_t, instance_t> &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define) {
	int region = default_id;
	if (syntax.region != "") {
		region = atoi(syntax.region.c_str());
	}

	Choice result(syntax.level != 0);

	for (int i = 0; i < (int)syntax.literals.size(); i++) {
		Action sub = import_action(syntax.literals[i], nets, region, tokens, auto_define);
		if (syntax.level == 0) {
			result |= sub;
		} else {
			result &= sub;
		}
	}

	for (int i = 0; i < (int)syntax.guards.size(); i++) {
		Action sub(import_expression(syntax.guards[i], nets, region, tokens, auto_define));
		if (syntax.level == 0) {
			result |= sub;
		} else {
			result &= sub;
		}
	}

	for (int i = 0; i < (int)syntax.compositions.size(); i++) {
		Choice sub = import_choice(syntax.compositions[i], nets, region, tokens, auto_define);
		if (syntax.level == 0) {
			result |= sub;
		} else {
			result &= sub;
		}
	}

	return result;
}

}
