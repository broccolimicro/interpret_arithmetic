#pragma once

#include <vector>
#include <parse_expression/precedence.h>
#include <arithmetic/expression.h>

struct ExpressionInterpreter {
	arithmetic::Expression import_unary(parse_expression::operation op, arithmetic::Expression expr) const;
	arithmetic::Expression import_binary(parse_expression::operation op, arithmetic::Expression left, arithmetic::Expression right) const;
	arithmetic::Expression import_group(parse_expression::operation op, std::vector<arithmetic::Expression> args) const;
	arithmetic::Expression import_modifier(parse_expression::operation op, std::vector<arithmetic::Expression> args) const;
};

