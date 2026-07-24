#pragma once

#include <parse_expression/expression.h>
#include <parse_expression/assignment.h>
#include <parse_expression/wrapper.h>

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

using expression = parse_expression::rvalue<expression_config>;
using guard = parse_expression::rvalue<expression_config, parse_expression::guard>;
using assignment = parse_expression::rvalue<expression_config, parse_expression::assignment>;
using simple_composition = parse_expression::rvalue<composition_config>;

}
