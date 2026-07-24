#include "expression.h"
#include <parse_expression/precedence.h>
#include <parse_expression/literal.h>
#include <parse/wrapper.h>

namespace test {

std::shared_ptr<parse_expression::config> expression_config::cfg = 
	std::make_shared<parse_expression::config>(parse_test::makeExprConfig());

expression_config::expression_config() {
	debug_name = "test";
}

expression_config::~expression_config() {
}

std::shared_ptr<parse_expression::config> composition_config::cfg = 
	std::make_shared<parse_expression::config>(parse_test::makeCompConfig());

composition_config::composition_config() {
	debug_name = "test";
}

composition_config::~composition_config() {
}

parse_expression::config makeExprConfig() {
	parse_expression::config cfg;
	int CONSTANT = cfg.push<parse_expression::default_constant>("constant");
	int LITERAL = cfg.push<parse_expression::default_literal>("literal");
	int TYPE = cfg.push<parse::wrapper<parse::instance> >("type");
	int TERM = cfg.push<parse::wrapper<parse::instance> >("term");
	int LABEL = cfg.push<parse::wrapper<parse::number> >("label");

	cfg.base = {LITERAL, CONSTANT};

	using operation_set=parse_expression::operation_set;

	cfg.order.push(operation_set::TERNARY);
	cfg.order.push_back("", "?", ":", "");

	cfg.order.push(operation_set::BINARY);
	cfg.order.push_back("", "", "|", "");

	cfg.order.push(operation_set::BINARY);
	cfg.order.push_back("", "", "&", "");

	cfg.order.push(operation_set::BINARY);
	cfg.order.push_back("", "", "^", "");

	cfg.order.push(operation_set::BINARY);
	cfg.order.push_back("", "", "||", "");

	cfg.order.push(operation_set::BINARY);
	cfg.order.push_back("", "", "&&", "");

	cfg.order.push(operation_set::BINARY);
	cfg.order.push_back("", "", "^^", "");

	cfg.order.push(operation_set::BINARY);
	cfg.order.push_back("", "", "==", "");
	cfg.order.push_back("", "", "~=", "");
	cfg.order.push_back("", "", "<", "");
	cfg.order.push_back("", "", ">", "");
	cfg.order.push_back("", "", "<=", "");
	cfg.order.push_back("", "", ">=", "");

	cfg.order.push(operation_set::BINARY);
	cfg.order.push_back("", "", "<<", "");
	cfg.order.push_back("", "", ">>", "");

	cfg.order.push(operation_set::BINARY);
	cfg.order.push_back("", "", "+", "");
	cfg.order.push_back("", "", "-", "");

	cfg.order.push(operation_set::BINARY);
	cfg.order.push_back("", "", "*", "");
	cfg.order.push_back("", "", "/", "");
	cfg.order.push_back("", "", "%", "");

	cfg.order.push(operation_set::UNARY);
	cfg.order.push_back("!", "", "", "");
	cfg.order.push_back("~", "", "", "");
	cfg.order.push_back("+", "", "", "");
	cfg.order.push_back("-", "", "", "");
	cfg.order.push_back("?", "", "", "");

	cfg.order.push(operation_set::MODIFIER);
	cfg.order.push_back("", "'", "", "", {LITERAL}, {LABEL});

	cfg.order.push(operation_set::MODIFIER);
	//cfg.order.push_back("", "{", ",", "}");
	cfg.order.push_back("", "(", ",", ")", {TERM});
	cfg.order.push_back("", ".", "", "", {LITERAL}, {LABEL});
	cfg.order.push_back("", "[", ":", "]");

	cfg.order.push(operation_set::MODIFIER);
	cfg.order.push_back("", "::", "", "", {TYPE}, {LABEL});

	cfg.order.push(operation_set::GROUP);
	cfg.order.push_back("[", "", ",", "]");

	cfg.lvalueLevel = cfg.order.size()-4;

	return cfg;
}

parse_expression::config makeCompConfig() {
	parse_expression::config cfg;

	int GUARD = cfg.push<guard>("guard");
	int ASSIGN = cfg.push<assignment>("assign");

	cfg.base = {GUARD, ASSIGN};

	using operation_set=parse_expression::operation_set;

	cfg.order.push(operation_set::BINARY);
	cfg.order.push_back("", "", ":", "");

	cfg.order.push(operation_set::BINARY);
	cfg.order.push_back("", "", ",", "");

	cfg.lvalueLevel = 2;

	return cfg;
}

}
