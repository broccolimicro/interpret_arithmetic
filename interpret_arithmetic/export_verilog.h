#pragma once

#include <common/standard.h>
#include <common/net.h>

#include <parse_verilog/expression.h>

#include <arithmetic/expression.h>
#include <arithmetic/state.h>
#include <arithmetic/action.h>

#include <interpret_arithmetic/export.h>

namespace parse_verilog {

struct ExpressionExporter : arithmetic::ExpressionExporter {
	pair<int, int> export_operator(int func) const override;
	const parse_expression::precedence_set &precedence() const override;
};

string export_value(const arithmetic::Value &v);

parse_expression::expression export_expression(const arithmetic::Value &v);
parse_expression::expression export_expression(const arithmetic::State &s, ucs::ConstNetlist nets);
parse_expression::expression export_composition(const arithmetic::State &s, ucs::ConstNetlist nets);
parse_expression::expression export_composition(const arithmetic::Region &r, ucs::ConstNetlist nets);
parse_expression::expression export_expression(const arithmetic::Expression &expr, ucs::ConstNetlist nets);
parse_expression::assignment export_assignment(const arithmetic::Action &expr, ucs::ConstNetlist nets);
parse_expression::expression export_composition(const arithmetic::Parallel &expr, ucs::ConstNetlist nets);
parse_expression::expression export_composition(const arithmetic::Choice &expr, ucs::ConstNetlist nets);

}
