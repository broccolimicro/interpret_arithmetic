#pragma once

#include <common/standard.h>
#include <common/net.h>

#include <parse_expression/expression.h>
#include <parse_expression/assignment.h>
#include <parse_expression/composition.h>

#include <arithmetic/expression.h>
#include <arithmetic/action.h>

namespace arithmetic {

parse_expression::expression export_field(string str);
parse_expression::expression export_member(string str);
parse_expression::expression export_net(string str);
parse_expression::expression export_net(int uid, ucs::ConstNetlist nets);

parse_expression::expression export_expression(const Value &v, ucs::ConstNetlist nets);
parse_expression::expression export_expression(const State &s, ucs::ConstNetlist nets);
parse_expression::composition export_composition(const State &s, ucs::ConstNetlist nets);
parse_expression::composition export_composition(const Region &r, ucs::ConstNetlist nets);
parse_expression::expression export_expression(const Expression &expr, ucs::ConstNetlist nets);
parse_expression::assignment export_assignment(const Action &expr, ucs::ConstNetlist nets);
parse_expression::composition export_composition(const Parallel &expr, ucs::ConstNetlist nets);
parse_expression::composition export_composition(const Choice &expr, ucs::ConstNetlist nets);

}
