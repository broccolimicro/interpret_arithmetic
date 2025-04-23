/*
 * export.h
 *
 *  Created on: Feb 6, 2015
 *      Author: nbingham
 */

#pragma once

#include <common/standard.h>

#include <parse_expression/expression.h>
#include <parse_expression/assignment.h>
#include <parse_expression/composition.h>

#include <arithmetic/expression.h>
#include <arithmetic/action.h>

#include "interface.h"

namespace arithmetic {

parse_ucs::variable_name export_net(int uid, ConstNetlist nets);
parse_expression::expression export_expression(const Value &v, ConstNetlist nets);
parse_expression::expression export_expression(const State &s, ConstNetlist nets);
parse_expression::composition export_composition(const State &s, ConstNetlist nets);
parse_expression::composition export_composition(const Region &r, ConstNetlist nets);
parse_expression::expression export_expression(const Expression &expr, ConstNetlist nets);
parse_expression::assignment export_assignment(const Action &expr, ConstNetlist nets);
parse_expression::composition export_composition(const Parallel &expr, ConstNetlist nets);
parse_expression::composition export_composition(const Choice &expr, ConstNetlist nets);

}
