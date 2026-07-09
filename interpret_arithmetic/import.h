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

_CONST_INTERFACE_ARG(ExpressionImporter,
	(arithmetic::Expression, import_unary, (parse_expression::operation op, arithmetic::Expression expr) const, (op, expr)),
	(arithmetic::Expression, import_binary, (parse_expression::operation op, arithmetic::Expression left, arithmetic::Expression right) const, (op, left, right)),
	(arithmetic::Expression, import_group, (parse_expression::operation op, vector<arithmetic::Expression> args) const, (op, args)),
	(arithmetic::Expression, import_modifier, (parse_expression::operation op, vector<arithmetic::Expression> args) const, (op, args))
);

Expression import_constant(string value) {
	if (value == "false") {
		return Expression::boolOf(false);
	} else if (value == "true") {
		return Expression::boolOf(true);
	} else if (value == "gnd") {
		return Expression::gnd();
	} else if (value == "vdd") {
		return Expression::vdd();
	} else if (not value.empty()) {
		size_t n = value.find_first_of("0123456789");
		size_t m = value.find_first_of(".-+");

		if (n != 0) {
			if ((value[0] == '\"' and value.back() == '\"')
				or (value[0] == '\'' and value.back() == '\'')) {
				return Expression::stringOf(value.substr(1, value.size()-2));
			}
			return Expression::stringOf(value);
		} else if (m != string::npos) {
			return Expression::realOf(atof(value.c_str()));
		}
		return Expression::intOf(atoi(value.c_str()));
	}
	return Expression::X();
}

template <int group, typename number_t, typename instance_t>
Expression import_argument(ExpressionImporter imp, const parse_expression::argument_t<group, number_t, instance_t> &syntax, parse_expression::operation::ArgType argType, ucs::Netlist symbols, int region, tokenizer *tokens) {
	if (syntax.sub.valid) {
		return import_expression(imp, syntax.sub, symbols, region, tokens);
	} else if (syntax.literal != "") {
			}
		if (argType == parse_expression::operation::LABEL) {
			return import_constant(syntax.literal);
		}
	
		int uid = symbols.netIndex(syntax.literal+"'"+::to_string(region));
		if (uid >= 0) {
			return Expression::varOf(uid);
		}



		}

		error("", "symbol not found '" + syntax.literal + "'", __FILE__, __LINE__);
		return Expression::undef();
	}
	if (argType != parse_expression::operation::LITERAL and argType != parse_expression::operation::LABEL) {
		error("", "expected typename, found '" + syntax.constant + "'", __FILE__, __LINE__);
	}
	return import_constant(syntax.constant);
}

template <int group, typename number_t, typename instance_t>
vector<Expression> import_arguments(ExpressionImporter imp, const vector<parse_expression::argument_t<group, number_t, instance_t> > &syntax, parse_expression::operation::ArgType leftType, parse_expression::operation::ArgType rightType, ucs::Netlist symbols, int region, tokenizer *tokens) {
	vector<Expression> result;
	for (size_t i = 0; i < syntax.size(); i++) {
		result.push_back(import_argument(syntax[i], (i == 0 ? leftType : rightType), symbols, region, tokens));
	}
	return result;
}

template <int group, typename number_t, typename instance_t>
Expression import_expression(ExpressionImporter imp, const parse_expression::expression_t<group, number_t, instance_t> &syntax, ucs::Netlist symbols, int region, tokenizer *tokens) {
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
			return import_argument(imp, syntax.arguments[0], parse_expression::operation::LITERAL, symbols, region, tokens);
		} else {
			internal("", "malformed expression", __FILE__, __LINE__);
		}
	}

	parse_expression::operation op = syntax.precedence.at(syntax.level, syntax.operators.back());
	if (syntax.precedence.isGroup(syntax.level)) {
		return imp.import_group(op, import_arguments(imp, syntax.arguments, op.leftType, op.rightType, symbols, region, tokens));
	} else if (syntax.precedence.isModifier(syntax.level)) {
		return imp.import_modifier(op, import_arguments(imp, syntax.arguments, op.leftType, op.rightType, symbols, region, tokens));
	} else if (syntax.precedence.isBinary(syntax.level) or syntax.precedence.isUnary(syntax.level)) {
		Expression result;
		if (not syntax.arguments.empty()) {
			result = import_argument(imp, syntax.arguments[0], parse_expression::operation::LITERAL, symbols, region, tokens);
		}

		if (syntax.arguments.size() == 1u) {
			if (syntax.precedence.isUnary(syntax.level)) {
				for (int i = (int)syntax.operators.size()-1; i >= 0; i--) {
					parse_expression::operation op = syntax.precedence.at(syntax.level, syntax.operators[i]);

					result = imp.import_unary(op, result);
				}
			}
		} else {
			for (size_t i = 1; i < syntax.arguments.size(); i++) {
				parse_expression::operation op = syntax.precedence.at(syntax.level, syntax.operators[i-1]);
				
				Expression sub = import_argument(imp, syntax.arguments[i], op.rightType, symbols, region, tokens);

				result = imp.import_binary(op, result, sub);
			}
		}

		return result;
	}
	return Expression();
}

template <int group, typename number_t, typename instance_t>
Action import_action(ExpressionImporter imp, const parse_expression::assignment_t<group, number_t, instance_t> &syntax, ucs::Netlist symbols, int region, tokenizer *tokens) {
	if (syntax.region != "") {
		region = atoi(syntax.region.c_str());
	}

	//cout << "import action " << syntax.to_string("") << endl;

	Action result;
	if (syntax.operation.empty()) {
		result.lvalue = Expression::undef();
		if (syntax.lvalue[0].valid) {
			result.rvalue = import_expression(imp, syntax.lvalue[0], symbols, region, tokens);
		}
	} else if (syntax.operation == "+") {
		if (syntax.lvalue.size() > 0) {
			result.lvalue = import_expression(imp, syntax.lvalue[0], symbols, region, tokens);
		}
		result.rvalue = Expression::vdd();
	} else if (syntax.operation == "-") {
		if (syntax.lvalue.size() > 0) {
			result.lvalue = import_expression(imp, syntax.lvalue[0], symbols, region, tokens);
		}
		result.rvalue = Expression::gnd();
	} else if (syntax.operation == "=") {
		if (syntax.lvalue.size() > 0) {
			result.lvalue = import_expression(imp, syntax.lvalue[0], symbols, region, tokens);
		}
		if (syntax.rvalue.valid) {
			result.rvalue = import_expression(imp, syntax.rvalue, symbols, region, tokens);
		}
	}
	//cout << result.lvalue << " = " << result.rvalue << endl;

	return result;
}

template <int group, typename number_t, typename instance_t>
Parallel import_parallel(ExpressionImporter imp, const parse_expression::composition_t<group, number_t, instance_t> &syntax, ucs::Netlist symbols, ucs::TypeTable types, int currMod, int region, tokenizer *tokens) {
	if (syntax.level == 0 and (syntax.literals.size() + syntax.guards.size() + syntax.compositions.size()) > 1u) {
		error(__FILE__, __LINE__, tokens, &syntax, "expected parallel composition");
		return Parallel();
	}

	if (syntax.region != "") {
		region = atoi(syntax.region.c_str());
	}

	Parallel result;
	for (int i = 0; i < (int)syntax.literals.size(); i++) {
		result &= import_action(imp, syntax.literals[i], symbols, types, currMod, region, tokens);
	}

	for (int i = 0; i < (int)syntax.guards.size(); i++) {
		result &= Action(import_expression(imp, syntax.guards[i], symbols, types, currMod, region, tokens));
	}

	for (int i = 0; i < (int)syntax.compositions.size(); i++) {
		result &= import_parallel(imp, syntax.compositions[i], symbols, types, currMod, region, tokens);
	}
	return result;
}

template <int group, typename number_t, typename instance_t>
Choice import_choice(ExpressionImporter imp, const parse_expression::composition_t<group, number_t, instance_t> &syntax, ucs::Netlist symbols, ucs::TypeTable types, int currMod, int region, tokenizer *tokens) {
	if (syntax.region != "") {
		region = atoi(syntax.region.c_str());
	}

	Choice result(syntax.level != 0);

	for (int i = 0; i < (int)syntax.literals.size(); i++) {
		Action sub = import_action(imp, syntax.literals[i], symbols, region, tokens);
		if (syntax.level == 0) {
			result |= sub;
		} else {
			result &= sub;
		}
	}

	for (int i = 0; i < (int)syntax.guards.size(); i++) {
		Action sub(import_expression(imp, syntax.guards[i], symbols, region, tokens));
		if (syntax.level == 0) {
			result |= sub;
		} else {
			result &= sub;
		}
	}

	for (int i = 0; i < (int)syntax.compositions.size(); i++) {
		Choice sub = import_choice(imp, syntax.compositions[i], symbols, region, tokens);
		if (syntax.level == 0) {
			result |= sub;
		} else {
			result &= sub;
		}
	}

	return result;
}







int import_net(string syntax, ucs::Netlist nets, tokenizer *tokens, bool auto_define);

template <int group, typename number_t, typename instance_t>
string import_constant(const parse_expression::argument_t<group, number_t, instance_t> &syntax, tokenizer *tokens);
template <int group, typename number_t, typename instance_t>
string import_constant(const parse_expression::expression_t<group, number_t, instance_t> &syntax, tokenizer *tokens);
template <int group, typename number_t, typename instance_t>
string import_net_name(const parse_expression::argument_t<group, number_t, instance_t> &syntax, tokenizer *tokens);
template <int group, typename number_t, typename instance_t>
string import_net_name(const parse_expression::expression_t<group, number_t, instance_t> &syntax, tokenizer *tokens);
template <int group, typename number_t, typename instance_t>
string import_literal(const parse_expression::argument_t<group, number_t, instance_t> &syntax, tokenizer *tokens);
template <int group, typename number_t, typename instance_t>
string import_literal(const parse_expression::expression_t<group, number_t, instance_t> &syntax, tokenizer *tokens);

template <int group, typename number_t, typename instance_t>
string import_constant(const parse_expression::argument_t<group, number_t, instance_t> &syntax, tokenizer *tokens) {
	if (syntax.sub.valid) {
		return import_constant(syntax.sub, tokens);
	} else if (not syntax.literal.empty()) {
		internal(__FILE__, __LINE__, tokens, nullptr, "expected constant-valued expression, found '{}'", syntax.literal);
		return "0";
	}
	return syntax.constant;
}

template <int group, typename number_t, typename instance_t>
string import_constant(const parse_expression::expression_t<group, number_t, instance_t> &syntax, tokenizer *tokens) {
	if (not syntax.valid or syntax.level < 0 or syntax.arguments.empty()) {
		internal(__FILE__, __LINE__, tokens, &syntax, "invalid expression");
		return "0";
	}

	string result = "";
	if (syntax.operators.empty()) {
		result += import_constant(syntax.arguments[0], tokens);
	} else {
		internal(__FILE__, __LINE__, tokens, &syntax, "sub expressions in constants not supported");
		return "0";
	}

	return result;
}

template <int group, typename number_t, typename instance_t>
string import_net_name(const parse_expression::argument_t<group, number_t, instance_t> &syntax, tokenizer *tokens) {
	if (syntax.sub.valid) {
		return import_net_name(syntax.sub, tokens);
	} else if (not syntax.literal.empty()) {
		return syntax.literal;
	}
	internal(__FILE__, __LINE__, tokens, nullptr, "expected instance, found '{}'", syntax.constant);
	return "_";
}

template <int group, typename number_t, typename instance_t>
string import_net_name(const parse_expression::expression_t<group, number_t, instance_t> &syntax, tokenizer *tokens) {
	if (tokens != nullptr) {
		tokens->load(&syntax);
	}

	if (not syntax.valid or syntax.level < 0 or syntax.arguments.empty()) {
		internal(__FILE__, __LINE__, tokens, &syntax, "invalid expression");
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
		internal(__FILE__, __LINE__, tokens, &syntax, "sub expressions in variable names not supported for '{}'", syntax.precedence.at(syntax.level, syntax.operators[0]).to_string());
		return "_";
	}

	return result;
}

template <int group, typename number_t, typename instance_t>
string import_literal(const parse_expression::argument_t<group, number_t, instance_t> &syntax, tokenizer *tokens) {
	if (syntax.sub.valid) {
		return import_literal(syntax.sub, tokens);
	} else if (syntax.literal != "") {
		return syntax.literal;
	}
	return "";
}

template <int group, typename number_t, typename instance_t>
string import_literal(const parse_expression::expression_t<group, number_t, instance_t> &syntax, tokenizer *tokens) {
	if (not syntax.operators.empty() or syntax.arguments.size() != 1u) {
		return "";
	}
	return import_literal(syntax.arguments[0], tokens);	
}

template <int group, typename number_t, typename instance_t>
int import_net(const parse_expression::expression_t<group, number_t, instance_t> &syntax, ucs::Netlist nets, tokenizer *tokens, int default_id) {
	string name = import_net_name(syntax, tokens);
	if (default_id != 0) {
		name += "'" + ::to_string(default_id);
	}

	return arithmetic::import_net(name, nets, tokens, false);
}

template <int group, typename number_t, typename instance_t>
State import_state(ExpressionImporter imp, const parse_expression::assignment_t<group, number_t, instance_t> &syntax, ucs::Netlist nets, tokenizer *tokens, int default_id) {
	int region = default_id;
	if (syntax.region != "") {
		region = atoi(syntax.region.c_str());
	}

	if (syntax.lvalue.empty() or not syntax.lvalue[0].valid) {
		internal("", "malformed assignment for state '" + syntax.to_string() + "'", __FILE__, __LINE__);
	}

	// TODO(edward.bingham) figure out net types
	if (syntax.operation == "+") {
		return State(import_net(syntax.lvalue[0], nets, tokens, region), Value::vdd());
	} else if (syntax.operation == "-") {
		return State(import_net(syntax.lvalue[0], nets, tokens, region), Value::gnd());
	} else if (syntax.operation == "~") {
		return State(import_net(syntax.lvalue[0], nets, tokens, region), Value::X());
	} else if (syntax.operation == "=") {
		State result;
		int v = import_net(syntax.lvalue[0], nets, tokens, region);
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
State import_state(ExpressionImporter imp, const parse_expression::composition_t<group, number_t, instance_t> &syntax, ucs::Netlist nets, tokenizer *tokens, int default_id) {
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
			result &= import_state(imp, syntax.literals[i], nets, tokens, region);
		}
	}

	for (int i = 0; i < (int)syntax.compositions.size(); i++) {
		if (syntax.compositions[i].valid) {
			result &= import_state(imp, syntax.compositions[i], nets, tokens, region);
		}
	}

	return result;
}


}
