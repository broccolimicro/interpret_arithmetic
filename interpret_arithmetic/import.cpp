/*
 * import.cpp
 *
 *  Created on: Feb 6, 2015
 *      Author: nbingham
 */

#include "import.h"
#include <interpret_ucs/import.h>

int import_operator(string op, size_t args) {
	if (args == 1u and op == "!") {
		return arithmetic::Operation::BITWISE_NOT;
	} else if (args == 1u and op == "+") {
		return arithmetic::Operation::IDENTITY;
	} else if (args == 1u and op == "-") {
		return arithmetic::Operation::NEGATION;
	} else if (args == 1u and op == "(bool)") {
		return arithmetic::Operation::VALIDITY;
	} else if (args == 1u and op == "~") {
		return arithmetic::Operation::BOOLEAN_NOT;
	} else if (args == 1u and op == "$") {
		return arithmetic::Operation::INVERSE;
	} else if (args >= 2u and op == "||") {
		return arithmetic::Operation::BITWISE_OR;
	} else if (args >= 2u and op == "&&") {
		return arithmetic::Operation::BITWISE_AND;
	} else if (args >= 2u and op == "^^") {
		return arithmetic::Operation::BITWISE_XOR;
	} else if (args >= 2u and op == "==") {
		return arithmetic::Operation::EQUAL;
	} else if (args >= 2u and op == "!=") {
		return arithmetic::Operation::NOT_EQUAL;
	} else if (args >= 2u and op == "<") {
		return arithmetic::Operation::LESS;
	} else if (args >= 2u and op == ">") {
		return arithmetic::Operation::GREATER;
	} else if (args >= 2u and op == "<=") {
		return arithmetic::Operation::LESS_EQUAL;
	} else if (args >= 2u and op == ">=") {
		return arithmetic::Operation::GREATER_EQUAL;
	} else if (args >= 2u and op == "<<") {
		return arithmetic::Operation::SHIFT_LEFT;
	} else if (args >= 2u and op == ">>") {
		return arithmetic::Operation::SHIFT_RIGHT;
	} else if (args >= 2u and op == "+") {
		return arithmetic::Operation::ADD;
	} else if (args >= 2u and op == "-") {
		return arithmetic::Operation::SUBTRACT;
	} else if (args >= 2u and op == "*") {
		return arithmetic::Operation::MULTIPLY;
	} else if (args >= 2u and op == "/") {
		return arithmetic::Operation::DIVIDE;
	} else if (args >= 2u and op == "%") {
		return arithmetic::Operation::MOD;
	} else if (args >= 2u and op == "|") {
		return arithmetic::Operation::BOOLEAN_OR;
	} else if (args >= 2u and op == "&") {
		return arithmetic::Operation::BOOLEAN_AND;
	} else if (args >= 2u and op == "^") {
		return arithmetic::Operation::BOOLEAN_XOR;
	} else if (args >= 2u and op == ",") {
		return arithmetic::Operation::ARRAY;
	}
	return -1;
}

arithmetic::State import_state(const parse_expression::assignment &syntax, ucs::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define)
{
	if (syntax.operation == "+") {
		vector<int> v = define_variables(syntax.names[0], variables, default_id, tokens, auto_define, auto_define);
		return arithmetic::State(v[0], true);
	} else if (syntax.operation == "-") {
		vector<int> v = define_variables(syntax.names[0], variables, default_id, tokens, auto_define, auto_define);
		return arithmetic::State(v[0], false);
	} else if (syntax.operation == "~") {
		vector<int> v = define_variables(syntax.names[0], variables, default_id, tokens, auto_define, auto_define);
		return arithmetic::State(v[0], arithmetic::Value::X());
	} else if (syntax.operation == "=") {
		arithmetic::State result;
		int m = min((int)syntax.names.size(), (int)syntax.expressions.size());
		for (int i = 0; i < m; i++) {
			vector<int> v = define_variables(syntax.names[i], variables, default_id, tokens, auto_define, auto_define);
			if (syntax.expressions[i].operations.empty() and syntax.expressions[i].arguments.size() == 1 and syntax.expressions[i].arguments[0].constant != "") {
				if (syntax.expressions[i].arguments[0].constant == "gnd") {
					result.set(v[0], false);
				} else if (syntax.expressions[i].arguments[0].constant == "vdd") {
					result.set(v[0], true);
				} else if (syntax.expressions[i].arguments[0].constant != "") {
					result.set(v[0], atoi(syntax.expressions[i].arguments[0].constant.c_str()));
				} else {
					if (tokens != NULL) {
						tokens->load(&syntax.expressions[i]);
						tokens->error("unsupported expression", __FILE__, __LINE__);
					} else {
						error(syntax.expressions[i].to_string(), "unsupported expression", __FILE__, __LINE__);
					}
					return arithmetic::State();
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

		return arithmetic::State();
	}
}

arithmetic::State import_state(const parse_expression::composition &syntax, ucs::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define)
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
		return arithmetic::State();
	}
	else if (syntax.literals.size() == 0 && syntax.compositions.size() == 0)
		return arithmetic::State();
	else if (syntax.precedence[syntax.level] == ":" && syntax.literals.size() + syntax.compositions.size() > 1)
	{
		if (tokens != NULL)
		{
			tokens->load(&syntax);
			tokens->error("illegal disjunction", __FILE__, __LINE__);
		}
		else
			error(syntax.to_string(), "illegal disjunction", __FILE__, __LINE__);
		return arithmetic::State();
	}

	arithmetic::State result;

	for (int i = 0; i < (int)syntax.literals.size(); i++)
		if (syntax.literals[i].valid)
			result &= import_state(syntax.literals[i], variables, default_id, tokens, auto_define);

	for (int i = 0; i < (int)syntax.compositions.size(); i++)
		if (syntax.compositions[i].valid)
			result &= import_state(syntax.compositions[i], variables, default_id, tokens, auto_define);

	return result;
}


arithmetic::Expression import_expression(const parse_expression::expression &syntax, ucs::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define)
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
		return arithmetic::Expression(false);
	}

	arithmetic::Expression result;

	for (int i = 0; i < (int)syntax.arguments.size(); i++)
	{
		if (syntax.arguments[i].sub.valid)
		{
			arithmetic::Expression sub = import_expression(syntax.arguments[i].sub, variables, default_id, tokens, auto_define);
			if (i == 0) {
				result = sub;
			} else {
				result.push(import_operator(syntax.operations[i-1], 2u), sub);
			}
		}
		else {
			arithmetic::Operand sub;
			if (syntax.arguments[i].literal.valid) {
				sub = arithmetic::Operand::varOf(define_variables(syntax.arguments[i].literal, variables, default_id, tokens, auto_define, auto_define)[0]);
			} else if (syntax.arguments[i].constant == "gnd") {
				sub = arithmetic::Operand::boolOf(false);
			} else if (syntax.arguments[i].constant == "vdd") {
				sub = arithmetic::Operand::boolOf(true);
			} else if (syntax.arguments[i].constant != "") {
				sub = arithmetic::Operand::intOf(atoi(syntax.arguments[i].constant.c_str()));
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

arithmetic::Action import_action(const parse_expression::assignment &syntax, ucs::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define)
{
	arithmetic::Action result;
	if (syntax.operation == "+")
	{
		result.behavior = arithmetic::Action::ASSIGN;
		if (syntax.names.size() > 0)
			result.variable = define_variables(syntax.names[0], variables, default_id, tokens, auto_define, auto_define)[0];
		result.expr = arithmetic::Operand::boolOf(true);
	}
	else if (syntax.operation == "-")
	{
		result.behavior = arithmetic::Action::ASSIGN;
		if (syntax.names.size() > 0)
			result.variable = define_variables(syntax.names[0], variables, default_id, tokens, auto_define, auto_define)[0];
		result.expr = arithmetic::Operand::boolOf(false);
	}
	else if (syntax.operation == "=")
	{
		result.behavior = arithmetic::Action::ASSIGN;
		if (syntax.names.size() > 0)
			result.variable = define_variables(syntax.names[0], variables, default_id, tokens, auto_define, auto_define)[0];
		if (syntax.expressions.size() > 0)
			result.expr = import_expression(syntax.expressions[0], variables, default_id, tokens, auto_define);
	}
	else if (syntax.operation == "?")
	{
		result.behavior = arithmetic::Action::RECEIVE;
		result.expr = arithmetic::Expression(true);
		if (syntax.names.size() > 0)
			result.channel = define_variables(syntax.names[0], variables, default_id, tokens, auto_define, auto_define)[0];
		if (syntax.names.size() > 1)
			result.variable = define_variables(syntax.names[1], variables, default_id, tokens, auto_define, auto_define)[0];
	}
	else if (syntax.operation == "!")
	{
		result.behavior = arithmetic::Action::SEND;
		if (syntax.names.size() > 0)
			result.channel = define_variables(syntax.names[0], variables, default_id, tokens, auto_define, auto_define)[0];
		if (syntax.expressions.size() > 0) {
			result.expr = import_expression(syntax.expressions[0], variables, default_id, tokens, auto_define);
		} else {
			result.expr = arithmetic::Expression(true);
		}
	}
	else if (syntax.operation == "?!")
	{
		result.behavior = arithmetic::Action::RECEIVE;
		if (syntax.names.size() > 0)
			result.channel = define_variables(syntax.names[0], variables, default_id, tokens, auto_define, auto_define)[0];
		if (syntax.names.size() > 1)
			result.variable = define_variables(syntax.names[1], variables, default_id, tokens, auto_define, auto_define)[0];
		if (syntax.expressions.size() > 0) {
			result.expr = import_expression(syntax.expressions[0], variables, default_id, tokens, auto_define);
		} else {
			result.expr = arithmetic::Expression(true);
		}
	}
	else if (syntax.operation == "!?")
	{
		result.behavior = arithmetic::Action::SEND;
		result.channel = define_variables(syntax.names[0], variables, default_id, tokens, auto_define, auto_define)[0];
		if (syntax.names.size() > 1)
			result.variable = define_variables(syntax.names[1], variables, default_id, tokens, auto_define, auto_define)[0];
		if (syntax.expressions.size() > 0) {
			result.expr = import_expression(syntax.expressions[0], variables, default_id, tokens, auto_define);
		} else {
			result.expr = arithmetic::Expression(true);
		}
	}

	return result;
}

arithmetic::Parallel import_parallel(const parse_expression::composition &syntax, ucs::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define)
{
	arithmetic::Parallel result;

	if (syntax.region != "")
		default_id = ::atoi(syntax.region.c_str());

	if (syntax.level == 0) {
		if (not syntax.literals.empty()) {
			result.actions.push_back(import_action(syntax.literals[0], variables, default_id, tokens, auto_define));
		} else if (not syntax.guards.empty()) {
			result = arithmetic::Parallel(import_expression(syntax.guards[0], variables, default_id, tokens, auto_define));
		} else if (not syntax.compositions.empty()) {
			arithmetic::Parallel temp = import_parallel(syntax.compositions[0], variables, default_id, tokens, auto_define);
			result.actions.insert(result.actions.end(), temp.actions.begin(), temp.actions.end());
		}
	} else {
		for (int i = 0; i < (int)syntax.literals.size(); i++)
			result.actions.push_back(import_action(syntax.literals[i], variables, default_id, tokens, auto_define));

		for (int i = 0; i < (int)syntax.guards.size(); i++)
			result.actions.push_back(arithmetic::Action(import_expression(syntax.guards[i], variables, default_id, tokens, auto_define)));

		for (int i = 0; i < (int)syntax.compositions.size(); i++)
		{
			arithmetic::Parallel temp = import_parallel(syntax.compositions[i], variables, default_id, tokens, auto_define);
			result.actions.insert(result.actions.end(), temp.actions.begin(), temp.actions.end());
		}
	}

	return result;
}


arithmetic::Choice import_choice(const parse_expression::composition &syntax, ucs::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define)
{
	arithmetic::Choice result;

	if (syntax.region != "")
		default_id = ::atoi(syntax.region.c_str());

	if (syntax.level == 0)
	{
		for (int i = 0; i < (int)syntax.literals.size(); i++)
		{
			result.terms.push_back(arithmetic::Parallel());
			result.terms.back().actions.push_back(import_action(syntax.literals[i], variables, default_id, tokens, auto_define));
		}

		for (int i = 0; i < (int)syntax.guards.size(); i++)
			result.terms.push_back(arithmetic::Parallel(import_expression(syntax.guards[i], variables, default_id, tokens, auto_define)));

		for (int i = 0; i < (int)syntax.compositions.size(); i++)
		{
			arithmetic::Choice temp = import_choice(syntax.compositions[i], variables, default_id, tokens, auto_define);
			result.terms.insert(result.terms.end(), temp.terms.begin(), temp.terms.end());
		}
	}
	else
	{
		result.terms.push_back(arithmetic::Parallel());
		for (int i = 0; i < (int)syntax.literals.size(); i++)
			result.terms.back().actions.push_back(import_action(syntax.literals[i], variables, default_id, tokens, auto_define));

		for (int i = 0; i < (int)syntax.guards.size(); i++)
			result.terms.back().actions.push_back(arithmetic::Action(import_expression(syntax.guards[i], variables, default_id, tokens, auto_define)));

		for (int i = 0; i < (int)syntax.compositions.size(); i++)
		{
			arithmetic::Choice temp = import_choice(syntax.compositions[i], variables, default_id, tokens, auto_define);
			for (int j = 0; j < (int)result.terms.size(); j++)
				for (int k = 0; k < (int)temp.terms.size(); k++)
					result.terms[j].actions.insert(result.terms[j].actions.end(), temp.terms[k].actions.begin(), temp.terms[k].actions.end());
		}
	}

	return result;
}

