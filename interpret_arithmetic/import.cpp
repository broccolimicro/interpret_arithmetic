#include "import.h"
#include "support.h"

namespace arithmetic {

string import_constant(const parse_expression::expression &syntax, tokenizer *tokens) {
	if (not syntax.valid or syntax.level < 0 or syntax.arguments.empty()) {
		if (tokens != nullptr) {
			tokens->load(&syntax);
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
			tokens->load(&syntax);
			tokens->internal("sub expressions in constants not supported", __FILE__, __LINE__);
		} else {
			internal("", "sub expressions in constants not supported", __FILE__, __LINE__);
		}
		return "0";
	}

	return result;
}

string import_constant(const parse_expression::argument &syntax, tokenizer *tokens) {
	if (syntax.sub.valid) {
		return import_constant(syntax.sub, tokens);
	} else if (not syntax.literal.empty()) {
		internal(syntax.literal, "expected constant-valued expression", __FILE__, __LINE__);
		return "0";
	}
	return syntax.constant;
}

string import_net_name(const parse_expression::argument &syntax, tokenizer *tokens) {
	if (syntax.sub.valid) {
		return import_net_name(syntax.sub, tokens);
	} else if (not syntax.literal.empty()) {
		return syntax.literal;
	}
	internal(syntax.constant, "expected instance", __FILE__, __LINE__);
	return "_";
}

string import_net_name(const parse_expression::expression &syntax, tokenizer *tokens) {
	if (not syntax.valid or syntax.level < 0 or syntax.arguments.empty()) {
		if (tokens != nullptr) {
			tokens->load(&syntax);
			tokens->internal("invalid expression", __FILE__, __LINE__);
		} else {
			internal("", "invaid expression", __FILE__, __LINE__);
		}
		return "_";
	}

	string result = "";
	if (syntax.operators.empty()) {
		result += import_net_name(syntax.arguments[0], tokens);
	} else if (syntax.symbol(syntax.operators[0]).is("", "[", ":", "]")) {
		result += import_net_name(syntax.arguments[0], tokens) + syntax.symbol(syntax.operators[0]).trigger;
		for (int i = 1; i < (int)syntax.arguments.size(); i++) {
			if (i != 1) {
				result += syntax.symbol(syntax.operators[0]).infix;
			}
			result += import_constant(syntax.arguments[i], tokens);
		}
		result += syntax.symbol(syntax.operators[0]).postfix;
	} else if (syntax.symbol(syntax.operators[0]).is("", ".", "", "")
			or syntax.symbol(syntax.operators[0]).is("", "::", "", "")) {
		result = import_net_name(syntax.arguments[0], tokens);
		result += syntax.symbol(syntax.operators[0]).trigger;
		result += import_net_name(syntax.arguments[1], tokens);
	} else {
		if (tokens != nullptr) {
			tokens->load(&syntax);
			tokens->internal("sub expressions in variable names not supported " + syntax.symbol(syntax.operators[0]).to_string(), __FILE__, __LINE__);
		} else {
			internal("", "sub expressions in variable names not supported " + syntax.symbol(syntax.operators[0]).to_string(), __FILE__, __LINE__);
		}
		return "_";
	}

	return result;
}

int import_net(string syntax, ucs::Netlist nets, tokenizer *tokens, bool auto_define) {
	int uid = nets.netIndex(syntax, auto_define);
	if (uid < 0) {
		if (tokens != nullptr) {
			tokens->error("undefined net '" + syntax + "'", __FILE__, __LINE__);
		} else {
			error("", "undefined net '" + syntax + "'", __FILE__, __LINE__);
		}
	}

	return uid;
}

int import_net(const parse_expression::expression &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define) {
	string name = import_net_name(syntax, tokens);
	if (default_id != 0) {
		name += "'" + ::to_string(default_id);
	}

	return import_net(name, nets, tokens, auto_define);
}

int import_operator(parse_expression::operation op) {
	for (int i = 0; i < (int)Operation::operators.size(); i++) {
		if (areSame(Operation::operators[i], op)) {
			return i;
		}
	}
	return -1;
}

State import_state(const parse_expression::assignment &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define)
{
	int region = default_id;
	if (syntax.region != "") {
		region = atoi(syntax.region.c_str());
	}

	if (syntax.operation == "+") {
		return State(import_net(syntax.lvalue[0], nets, region, tokens, auto_define), true);
	} else if (syntax.operation == "-") {
		return State(import_net(syntax.lvalue[0], nets, region, tokens, auto_define), false);
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

State import_state(const parse_expression::composition &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define)
{
	int region = default_id;
	if (syntax.region != "") {
		region = atoi(syntax.region.c_str());
	}

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
			result &= import_state(syntax.literals[i], nets, region, tokens, auto_define);

	for (int i = 0; i < (int)syntax.compositions.size(); i++)
		if (syntax.compositions[i].valid)
			result &= import_state(syntax.compositions[i], nets, region, tokens, auto_define);

	return result;
}

Expression import_argument(const parse_expression::argument &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define) {
	if (syntax.sub.valid) {
		return import_expression(syntax.sub, nets, default_id, tokens, auto_define);
	} else if (syntax.literal != "") {
		string name = syntax.literal;
		if (default_id != 0) {
			name += "'" + ::to_string(default_id);
		}
		return Operand::varOf(import_net(name, nets, tokens, auto_define));
	} else if (syntax.constant == "false") {
		return Operand::boolOf(false);
	} else if (syntax.constant == "true") {
		return Operand::boolOf(true);
	} else if (syntax.constant != "") {
		return Operand::intOf(atoi(syntax.constant.c_str()));
	}
	return Operand::X();
}

Expression import_expression(const parse_expression::expression &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define) {
	int region = default_id;

	if (syntax.level >= (int)syntax.precedence.size()) {
		if (tokens != NULL) {
			tokens->load(&syntax);
			tokens->error("unrecognized operation", __FILE__, __LINE__);
		} else {
			error(syntax.to_string(), "unrecognized operation", __FILE__, __LINE__);
		}
		return Expression(false);
	}

	bool err = false;
	Expression result;
	if (not syntax.operators.empty()
		and parse_expression::expression::precedence[syntax.level].type == parse_expression::operation_set::MODIFIER
		and syntax.symbol(syntax.operators.back()).is("", "'", "", "")) {
		string cnst = import_constant(syntax.arguments.back(), tokens);
		result = import_argument(syntax.arguments[0], nets, atoi(cnst.c_str()), tokens, auto_define);
	} else if (not syntax.operators.empty()
		and parse_expression::expression::precedence[syntax.level].type == parse_expression::operation_set::MODIFIER
		and (syntax.symbol(syntax.operators.back()).is("", ".", "", "")
			or syntax.symbol(syntax.operators.back()).is("", "::", "", ""))) {
		result = Operand::varOf(import_net(syntax, nets, default_id, tokens, auto_define));
	} else if (not syntax.operators.empty()
    and parse_expression::expression::precedence[syntax.level].type == parse_expression::operation_set::MODIFIER) {
		vector<Expression> sub;
		int op = import_operator(syntax.symbol(syntax.operators[0]));
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
				int op = import_operator(syntax.symbol(syntax.operators[i-1]));
				if (op < 0) {
					err = true;
				} else {
					result.push(op, sub);
				}
			}
		}

		if (syntax.arguments.size() == 1) {
			if (parse_expression::expression::precedence[syntax.level].type == parse_expression::operation_set::UNARY) {
				for (int i = (int)syntax.operators.size()-1; i >= 0 and not err; i--) {
					int op = import_operator(syntax.symbol(syntax.operators[i]));
					if (op < 0) {
						err = true;
					} else {
						result.push(op);
					}
				}
			} else {
				for (int i = 0; i < (int)syntax.operators.size() and not err; i++) {
					int op = import_operator(syntax.symbol(syntax.operators[i]));
					
					if (op < 0) {
						err = true;
					} else {
						result.push(op);
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

Action import_action(const parse_expression::assignment &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define) {
	int region = default_id;
	if (syntax.region != "") {
		region = atoi(syntax.region.c_str());
	}

	Action result;
	if (syntax.operation.empty()) {
		result.variable = -1;
		if (syntax.lvalue[0].valid) {
			result.expr = import_expression(syntax.lvalue[0], nets, region, tokens, auto_define);
		}
	} else if (syntax.operation == "+") {
		if (syntax.lvalue.size() > 0) {
			result.variable = import_net(syntax.lvalue[0], nets, region, tokens, auto_define);
		}
		result.expr = Operand::boolOf(true);
	} else if (syntax.operation == "-") {
		if (syntax.lvalue.size() > 0) {
			result.variable = import_net(syntax.lvalue[0], nets, region, tokens, auto_define);
		}
		result.expr = Operand::boolOf(false);
	} else if (syntax.operation == "=") {
		if (syntax.lvalue.size() > 0) {
			result.variable = import_net(syntax.lvalue[0], nets, region, tokens, auto_define);
		}
		if (syntax.rvalue.valid) {
			result.expr = import_expression(syntax.rvalue, nets, region, tokens, auto_define);
		}
	}

	return result;
}

Parallel import_parallel(const parse_expression::composition &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define) {
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


Choice import_choice(const parse_expression::composition &syntax, ucs::Netlist nets, int default_id, tokenizer *tokens, bool auto_define) {
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
