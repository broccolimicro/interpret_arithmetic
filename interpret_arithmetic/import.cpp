/*
 * import.cpp
 *
 *  Created on: Feb 6, 2015
 *      Author: nbingham
 */

#include "import.h"

namespace arithmetic {

int import_net(const parse_ucs::variable_name &syntax, Netlist nets, int default_id, tokenizer *tokens, bool auto_define) {
	int region = default_id;
	if (syntax.region != "") {
		region = atoi(syntax.region.c_str());
	}

	string name;
	if (syntax.names.size() > 0) {
		name = syntax.names[0].to_string("");
	} for (int i = 1; i < (int)syntax.names.size(); i++) {
		name += "." + syntax.names[i].to_string("");
	}

	int uid = nets.netIndex(name, region, auto_define);
	if (uid < 0) {
		if (tokens != NULL) {
			tokens->load(&syntax);
			tokens->error("undefined net '" + name + "'", __FILE__, __LINE__);
		} else {
			error("", "undefined net '" + name + "'", __FILE__, __LINE__);
		}
	}

	return uid;
}

int import_operator(string op, size_t args) {
	if (args == 1u and op == "!") {
		return Operation::BITWISE_NOT;
	} else if (args == 1u and op == "+") {
		return Operation::IDENTITY;
	} else if (args == 1u and op == "-") {
		return Operation::NEGATION;
	} else if (args == 1u and op == "(bool)") {
		return Operation::VALIDITY;
	} else if (args == 1u and op == "~") {
		return Operation::BOOLEAN_NOT;
	} else if (args == 1u and op == "$") {
		return Operation::INVERSE;
	} else if (args >= 2u and op == "||") {
		return Operation::BITWISE_OR;
	} else if (args >= 2u and op == "&&") {
		return Operation::BITWISE_AND;
	} else if (args >= 2u and op == "^^") {
		return Operation::BITWISE_XOR;
	} else if (args >= 2u and op == "==") {
		return Operation::EQUAL;
	} else if (args >= 2u and op == "!=") {
		return Operation::NOT_EQUAL;
	} else if (args >= 2u and op == "<") {
		return Operation::LESS;
	} else if (args >= 2u and op == ">") {
		return Operation::GREATER;
	} else if (args >= 2u and op == "<=") {
		return Operation::LESS_EQUAL;
	} else if (args >= 2u and op == ">=") {
		return Operation::GREATER_EQUAL;
	} else if (args >= 2u and op == "<<") {
		return Operation::SHIFT_LEFT;
	} else if (args >= 2u and op == ">>") {
		return Operation::SHIFT_RIGHT;
	} else if (args >= 2u and op == "+") {
		return Operation::ADD;
	} else if (args >= 2u and op == "-") {
		return Operation::SUBTRACT;
	} else if (args >= 2u and op == "*") {
		return Operation::MULTIPLY;
	} else if (args >= 2u and op == "/") {
		return Operation::DIVIDE;
	} else if (args >= 2u and op == "%") {
		return Operation::MOD;
	} else if (args >= 2u and op == "|") {
		return Operation::BOOLEAN_OR;
	} else if (args >= 2u and op == "&") {
		return Operation::BOOLEAN_AND;
	} else if (args >= 2u and op == "^") {
		return Operation::BOOLEAN_XOR;
	} else if (args >= 2u and op == ",") {
		return Operation::ARRAY;
	}
	return -1;
}

State import_state(const parse_expression::assignment &syntax, Netlist nets, int default_id, tokenizer *tokens, bool auto_define)
{
	if (syntax.operation == "+") {
		return State(import_net(syntax.names[0], nets, default_id, tokens, auto_define), true);
	} else if (syntax.operation == "-") {
		return State(import_net(syntax.names[0], nets, default_id, tokens, auto_define), false);
	} else if (syntax.operation == "~") {
		return State(import_net(syntax.names[0], nets, default_id, tokens, auto_define), Value::X());
	} else if (syntax.operation == "=") {
		State result;
		int m = min((int)syntax.names.size(), (int)syntax.expressions.size());
		for (int i = 0; i < m; i++) {
			int v = import_net(syntax.names[i], nets, default_id, tokens, auto_define);
			if (syntax.expressions[i].operations.empty() and syntax.expressions[i].arguments.size() == 1 and syntax.expressions[i].arguments[0].constant != "") {
				if (syntax.expressions[i].arguments[0].constant == "gnd") {
					result.set(v, false);
				} else if (syntax.expressions[i].arguments[0].constant == "vdd") {
					result.set(v, true);
				} else if (syntax.expressions[i].arguments[0].constant != "") {
					result.set(v, atoi(syntax.expressions[i].arguments[0].constant.c_str()));
				} else {
					if (tokens != NULL) {
						tokens->load(&syntax.expressions[i]);
						tokens->error("unsupported expression", __FILE__, __LINE__);
					} else {
						error(syntax.expressions[i].to_string(), "unsupported expression", __FILE__, __LINE__);
					}
					return State();
				}
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

State import_state(const parse_expression::composition &syntax, Netlist nets, int default_id, tokenizer *tokens, bool auto_define)
{
	if (syntax.region != "")
		default_id = atoi(syntax.region.c_str());

	if (syntax.level >= (int)syntax.precedence.size())
	{
		if (tokens != NULL)
		{
			tokens->load(&syntax);
			tokens->error("unrecognized operation", __FILE__, __LINE__);
		}
		else
			error(syntax.to_string(), "unrecognized operation", __FILE__, __LINE__);
		return State();
	}
	else if (syntax.literals.size() == 0 && syntax.compositions.size() == 0)
		return State();
	else if (syntax.precedence[syntax.level] == ":" && syntax.literals.size() + syntax.compositions.size() > 1)
	{
		if (tokens != NULL)
		{
			tokens->load(&syntax);
			tokens->error("illegal disjunction", __FILE__, __LINE__);
		}
		else
			error(syntax.to_string(), "illegal disjunction", __FILE__, __LINE__);
		return State();
	}

	State result;

	for (int i = 0; i < (int)syntax.literals.size(); i++)
		if (syntax.literals[i].valid)
			result &= import_state(syntax.literals[i], nets, default_id, tokens, auto_define);

	for (int i = 0; i < (int)syntax.compositions.size(); i++)
		if (syntax.compositions[i].valid)
			result &= import_state(syntax.compositions[i], nets, default_id, tokens, auto_define);

	return result;
}


Expression import_expression(const parse_expression::expression &syntax, Netlist nets, int default_id, tokenizer *tokens, bool auto_define)
{
	if (syntax.region != "")
		default_id = atoi(syntax.region.c_str());

	if (syntax.level >= (int)syntax.precedence.size())
	{
		if (tokens != NULL)
		{
			tokens->load(&syntax);
			tokens->error("unrecognized operation", __FILE__, __LINE__);
		}
		else
			error(syntax.to_string(), "unrecognized operation", __FILE__, __LINE__);
		return Expression(false);
	}

	Expression result;

	for (int i = 0; i < (int)syntax.arguments.size(); i++)
	{
		if (syntax.arguments[i].sub.valid)
		{
			Expression sub = import_expression(syntax.arguments[i].sub, nets, default_id, tokens, auto_define);
			if (i == 0) {
				result = sub;
			} else {
				result.push(import_operator(syntax.operations[i-1], 2u), sub);
			}
		}
		else {
			Operand sub;
			if (syntax.arguments[i].literal.valid) {
				sub = Operand::varOf(import_net(syntax.arguments[i].literal, nets, default_id, tokens, auto_define));
			} else if (syntax.arguments[i].constant == "gnd") {
				sub = Operand::boolOf(false);
			} else if (syntax.arguments[i].constant == "vdd") {
				sub = Operand::boolOf(true);
			} else if (syntax.arguments[i].constant != "") {
				sub = Operand::intOf(atoi(syntax.arguments[i].constant.c_str()));
			}
			if (i == 0) {
				result = sub;
			} else {
				result.push(import_operator(syntax.operations[i-1], 2u), sub);
			}
		}
	}

	if (syntax.arguments.size() == 1) {
		if (parse_expression::expression::precedence[syntax.level].type == parse_expression::operation_set::left_unary) {
			for (int i = (int)syntax.operations.size()-1; i >= 0; i--) {
				result.push(import_operator(syntax.operations[i], 1u));
			}
		} else {
			for (int i = 0; i < (int)syntax.operations.size(); i++) {
				result.push(import_operator(syntax.operations[i], 1u));
			}
		}
	}

	return result;
}

Action import_action(const parse_expression::assignment &syntax, Netlist nets, int default_id, tokenizer *tokens, bool auto_define)
{
	Action result;
	if (syntax.operation == "+") {
		if (syntax.names.size() > 0) {
			result.variable = import_net(syntax.names[0], nets, default_id, tokens, auto_define);
		}
		result.expr = Operand::boolOf(true);
	} else if (syntax.operation == "-") {
		if (syntax.names.size() > 0) {
			result.variable = import_net(syntax.names[0], nets, default_id, tokens, auto_define);
		}
		result.expr = Operand::boolOf(false);
	} else if (syntax.operation == "=") {
		if (syntax.names.size() > 0) {
			result.variable = import_net(syntax.names[0], nets, default_id, tokens, auto_define);
		}
		if (syntax.expressions.size() > 0) {
			result.expr = import_expression(syntax.expressions[0], nets, default_id, tokens, auto_define);
		}
	}

	return result;
}

Parallel import_parallel(const parse_expression::composition &syntax, Netlist nets, int default_id, tokenizer *tokens, bool auto_define)
{
	Parallel result;

	if (syntax.region != "")
		default_id = ::atoi(syntax.region.c_str());

	if (syntax.level == 0) {
		if (not syntax.literals.empty()) {
			result.actions.push_back(import_action(syntax.literals[0], nets, default_id, tokens, auto_define));
		} else if (not syntax.guards.empty()) {
			result = Parallel(import_expression(syntax.guards[0], nets, default_id, tokens, auto_define));
		} else if (not syntax.compositions.empty()) {
			Parallel temp = import_parallel(syntax.compositions[0], nets, default_id, tokens, auto_define);
			result.actions.insert(result.actions.end(), temp.actions.begin(), temp.actions.end());
		}
	} else {
		for (int i = 0; i < (int)syntax.literals.size(); i++)
			result.actions.push_back(import_action(syntax.literals[i], nets, default_id, tokens, auto_define));

		for (int i = 0; i < (int)syntax.guards.size(); i++)
			result.actions.push_back(Action(import_expression(syntax.guards[i], nets, default_id, tokens, auto_define)));

		for (int i = 0; i < (int)syntax.compositions.size(); i++)
		{
			Parallel temp = import_parallel(syntax.compositions[i], nets, default_id, tokens, auto_define);
			result.actions.insert(result.actions.end(), temp.actions.begin(), temp.actions.end());
		}
	}

	return result;
}


Choice import_choice(const parse_expression::composition &syntax, Netlist nets, int default_id, tokenizer *tokens, bool auto_define)
{
	Choice result;

	if (syntax.region != "")
		default_id = ::atoi(syntax.region.c_str());

	if (syntax.level == 0)
	{
		for (int i = 0; i < (int)syntax.literals.size(); i++)
		{
			result.terms.push_back(Parallel());
			result.terms.back().actions.push_back(import_action(syntax.literals[i], nets, default_id, tokens, auto_define));
		}

		for (int i = 0; i < (int)syntax.guards.size(); i++)
			result.terms.push_back(Parallel(import_expression(syntax.guards[i], nets, default_id, tokens, auto_define)));

		for (int i = 0; i < (int)syntax.compositions.size(); i++)
		{
			Choice temp = import_choice(syntax.compositions[i], nets, default_id, tokens, auto_define);
			result.terms.insert(result.terms.end(), temp.terms.begin(), temp.terms.end());
		}
	}
	else
	{
		result.terms.push_back(Parallel());
		for (int i = 0; i < (int)syntax.literals.size(); i++)
			result.terms.back().actions.push_back(import_action(syntax.literals[i], nets, default_id, tokens, auto_define));

		for (int i = 0; i < (int)syntax.guards.size(); i++)
			result.terms.back().actions.push_back(Action(import_expression(syntax.guards[i], nets, default_id, tokens, auto_define)));

		for (int i = 0; i < (int)syntax.compositions.size(); i++)
		{
			Choice temp = import_choice(syntax.compositions[i], nets, default_id, tokens, auto_define);
			for (int j = 0; j < (int)result.terms.size(); j++)
				for (int k = 0; k < (int)temp.terms.size(); k++)
					result.terms[j].actions.insert(result.terms[j].actions.end(), temp.terms[k].actions.begin(), temp.terms[k].actions.end());
		}
	}

	return result;
}

}
