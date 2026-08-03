#pragma once

#include <parse_expression/expression.h>
#include <parse_expression/assignment.h>
#include <parse_expression/wrapper.h>
#include <parse_expression/literal.h>
#include <parse/wrapper.h>

namespace test {

parse_expression::config makeExprConfig();
parse_expression::config makeCompConfig();

struct expression_config {
	static std::shared_ptr<parse_expression::config> cfg;

	string debug_name;

	expression_config();
	~expression_config();
};

struct composition_config {
	static std::shared_ptr<parse_expression::config> cfg;

	string debug_name;

	composition_config();
	~composition_config();
};

using constant = parse_expression::default_constant;
using literal = parse_expression::default_literal;
using label = parse::wrapper<parse::instance>;
using ident = parse::wrapper<parse::number>;

using expression = parse_expression::rvalue<expression_config>;
using guard = parse_expression::rvalue<expression_config, parse_expression::guard>;
using assignment = parse_expression::rvalue<expression_config, parse_expression::assignment>;
using composition = parse_expression::rvalue<composition_config>;

}
