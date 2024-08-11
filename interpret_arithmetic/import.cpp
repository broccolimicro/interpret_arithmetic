/*
 * import.cpp
 *
 *  Created on: Feb 6, 2015
 *      Author: nbingham
 */

#include "import.h"
#include <interpret_ucs/import.h>


arithmetic::state import_state(const parse_expression::assignment &syntax, ucs::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define)
{
	if (syntax.operation == "+") {
		vector<int> v = define_variables(syntax.names[0], variables, default_id, tokens, auto_define, auto_define);
		return arithmetic::state(v[0], arithmetic::value::valid);
	} else if (syntax.operation == "-") {
		vector<int> v = define_variables(syntax.names[0], variables, default_id, tokens, auto_define, auto_define);
		return arithmetic::state(v[0], arithmetic::value::neutral);
	} else if (syntax.operation == "~") {
		vector<int> v = define_variables(syntax.names[0], variables, default_id, tokens, auto_define, auto_define);
		return arithmetic::state(v[0], arithmetic::value::unstable);
	} else if (syntax.operation == ":=") {
		arithmetic::state result;
		int m = min((int)syntax.names.size(), (int)syntax.expressions.size());
		for (int i = 0; i < m; i++) {
			vector<int> v = define_variables(syntax.names[i], variables, default_id, tokens, auto_define, auto_define);
			if (syntax.expressions[i].operations.empty() and syntax.expressions[i].arguments.size() == 1 and syntax.expressions[i].arguments[0].constant != "") {
				if (syntax.expressions[i].arguments[0].constant == "null") {
					result.set(v[0], arithmetic::value::neutral);
				} else if (syntax.expressions[i].arguments[0].constant != "") {
					result.set(v[0], atoi(syntax.expressions[i].arguments[0].constant.c_str()));
				} else {
					if (tokens != NULL) {
						tokens->load(&syntax.expressions[i]);
						tokens->error("unsupported expression", __FILE__, __LINE__);
					} else {
						error(syntax.expressions[i].to_string(), "unsupported expression", __FILE__, __LINE__);
					}
					return arithmetic::state();
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

		return arithmetic::state();
	}
}

arithmetic::state import_state(const parse_expression::composition &syntax, ucs::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define)
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
		return arithmetic::state();
	}
	else if (syntax.literals.size() == 0 && syntax.compositions.size() == 0)
		return arithmetic::state();
	else if (syntax.precedence[syntax.level] == ":" && syntax.literals.size() + syntax.compositions.size() > 1)
	{
		if (tokens != NULL)
		{
			tokens->load(&syntax);
			tokens->error("illegal disjunction", __FILE__, __LINE__);
		}
		else
			error(syntax.to_string(), "illegal disjunction", __FILE__, __LINE__);
		return arithmetic::state();
	}

	arithmetic::state result;

	for (int i = 0; i < (int)syntax.literals.size(); i++)
		if (syntax.literals[i].valid)
			result &= import_state(syntax.literals[i], variables, default_id, tokens, auto_define);

	for (int i = 0; i < (int)syntax.compositions.size(); i++)
		if (syntax.compositions[i].valid)
			result &= import_state(syntax.compositions[i], variables, default_id, tokens, auto_define);

	return result;
}


arithmetic::expression import_expression(const parse_expression::expression &syntax, ucs::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define)
{
	arithmetic::expression result;
	vector<arithmetic::operand> operands;

	for (int i = 0; i < (int)syntax.arguments.size(); i++)
	{
		int offset = (int)result.operations.size();

		if (syntax.arguments[i].sub.valid)
		{
			arithmetic::expression sub = import_expression(syntax.arguments[i].sub, variables, default_id, tokens, auto_define);

			for (int j = 0; j < (int)sub.operations.size(); j++)
			{
				arithmetic::operation tmp = sub.operations[j];
				for (int k = 0; k < (int)tmp.operands.size(); k++)
					if (tmp.operands[k].type == arithmetic::operand::expression)
						tmp.operands[k].index += offset;

				result.operations.push_back(tmp);
			}

			operands.push_back(arithmetic::operand((int)result.operations.size()-1, arithmetic::operand::expression));
		}
		else if (syntax.arguments[i].literal.valid)
			operands.push_back(arithmetic::operand(define_variables(syntax.arguments[i].literal, variables, default_id, tokens, auto_define, auto_define)[0], arithmetic::operand::variable));
		else if (syntax.arguments[i].constant == "null")
			operands.push_back(arithmetic::operand(0, arithmetic::operand::neutral));
		else if (syntax.arguments[i].constant != "")
			operands.push_back(arithmetic::operand(atoi(syntax.arguments[i].constant.c_str()), arithmetic::operand::constant));

		if (operands.size() == 2)
		{
			result.operations.push_back(arithmetic::operation(syntax.operations[i-1], operands));
			operands.clear();
			operands.push_back(arithmetic::operand((int)result.operations.size()-1, arithmetic::operand::expression));
		}
	}

	if (result.operations.size() == 0)
		result.operations.push_back(arithmetic::operation("", operands));

	return result;
}

arithmetic::action import_action(const parse_expression::assignment &syntax, ucs::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define)
{
	arithmetic::action result;
	if (syntax.operation == "+")
	{
		result.behavior = arithmetic::action::assign;
		if (syntax.names.size() > 0)
			result.variable = define_variables(syntax.names[0], variables, default_id, tokens, auto_define, auto_define)[0];
		result.expr = arithmetic::operand(0, arithmetic::operand::constant);
	}
	else if (syntax.operation == "-")
	{
		result.behavior = arithmetic::action::assign;
		if (syntax.names.size() > 0)
			result.variable = define_variables(syntax.names[0], variables, default_id, tokens, auto_define, auto_define)[0];
		result.expr = arithmetic::operand(0, arithmetic::operand::neutral);
	}
	else if (syntax.operation == ":=")
	{
		result.behavior = arithmetic::action::assign;
		if (syntax.names.size() > 0)
			result.variable = define_variables(syntax.names[0], variables, default_id, tokens, auto_define, auto_define)[0];
		if (syntax.expressions.size() > 0)
			result.expr = import_expression(syntax.expressions[0], variables, default_id, tokens, auto_define);
	}
	else if (syntax.operation == "?")
	{
		result.behavior = arithmetic::action::receive;
		if (syntax.names.size() > 0)
			result.channel = define_variables(syntax.names[0], variables, default_id, tokens, auto_define, auto_define)[0];
		if (syntax.names.size() > 1)
			result.variable = define_variables(syntax.names[1], variables, default_id, tokens, auto_define, auto_define)[0];
	}
	else if (syntax.operation == "!")
	{
		result.behavior = arithmetic::action::send;
		if (syntax.names.size() > 0)
			result.channel = define_variables(syntax.names[0], variables, default_id, tokens, auto_define, auto_define)[0];
		if (syntax.expressions.size() > 0)
			result.expr = import_expression(syntax.expressions[0], variables, default_id, tokens, auto_define);
	}
	else if (syntax.operation == "?!")
	{
		result.behavior = arithmetic::action::receive;
		if (syntax.names.size() > 0)
			result.channel = define_variables(syntax.names[0], variables, default_id, tokens, auto_define, auto_define)[0];
		if (syntax.names.size() > 1)
			result.variable = define_variables(syntax.names[1], variables, default_id, tokens, auto_define, auto_define)[0];
		if (syntax.expressions.size() > 0)
			result.expr = import_expression(syntax.expressions[0], variables, default_id, tokens, auto_define);
	}
	else if (syntax.operation == "!?")
	{
		result.behavior = arithmetic::action::send;
		result.channel = define_variables(syntax.names[0], variables, default_id, tokens, auto_define, auto_define)[0];
		if (syntax.names.size() > 1)
			result.variable = define_variables(syntax.names[1], variables, default_id, tokens, auto_define, auto_define)[0];
		if (syntax.expressions.size() > 0)
			result.expr = import_expression(syntax.expressions[0], variables, default_id, tokens, auto_define);
	}

	return result;
}

arithmetic::choice import_choice(const parse_expression::composition &syntax, ucs::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define)
{
	arithmetic::choice result;

	if (syntax.region != "")
		default_id = ::atoi(syntax.region.c_str());

	if (syntax.level == 0)
	{
		for (int i = 0; i < (int)syntax.literals.size(); i++)
		{
			result.terms.push_back(arithmetic::parallel());
			result.terms.back().actions.push_back(import_action(syntax.literals[i], variables, default_id, tokens, auto_define));
		}

		for (int i = 0; i < (int)syntax.guards.size(); i++)
			result.terms.push_back(arithmetic::parallel(import_expression(syntax.guards[i], variables, default_id, tokens, auto_define)));

		for (int i = 0; i < (int)syntax.compositions.size(); i++)
		{
			arithmetic::choice temp = import_choice(syntax.compositions[i], variables, default_id, tokens, auto_define);
			result.terms.insert(result.terms.end(), temp.terms.begin(), temp.terms.end());
		}
	}
	else
	{
		result.terms.push_back(arithmetic::parallel());
		for (int i = 0; i < (int)syntax.literals.size(); i++)
			result.terms.back().actions.push_back(import_action(syntax.literals[i], variables, default_id, tokens, auto_define));

		for (int i = 0; i < (int)syntax.guards.size(); i++)
			result.terms.back().actions.push_back(arithmetic::action(import_expression(syntax.guards[i], variables, default_id, tokens, auto_define)));

		for (int i = 0; i < (int)syntax.compositions.size(); i++)
		{
			arithmetic::choice temp = import_choice(syntax.compositions[i], variables, default_id, tokens, auto_define);
			for (int j = 0; j < (int)result.terms.size(); j++)
				for (int k = 0; k < (int)temp.terms.size(); k++)
					result.terms[j].actions.insert(result.terms[j].actions.end(), temp.terms[k].actions.begin(), temp.terms[k].actions.end());
		}
	}

	return result;
}

