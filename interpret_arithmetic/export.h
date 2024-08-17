/*
 * export.h
 *
 *  Created on: Feb 6, 2015
 *      Author: nbingham
 */

#pragma once

#include <common/standard.h>

#include <ucs/variable.h>

#include <parse_expression/expression.h>
#include <parse_expression/assignment.h>
#include <parse_expression/composition.h>

#include <arithmetic/expression.h>
#include <arithmetic/action.h>

parse_expression::expression export_expression(const arithmetic::value &v, const ucs::variable_set &variables);
parse_expression::expression export_expression(const arithmetic::state &s, const ucs::variable_set &variables);
parse_expression::composition export_composition(const arithmetic::state &s, const ucs::variable_set &variables);
parse_expression::expression export_expression(const arithmetic::expression &expr, const ucs::variable_set &variables);
parse_expression::assignment export_assignment(const arithmetic::action &expr, const ucs::variable_set &variables);
parse_expression::composition export_composition(const arithmetic::parallel &expr, const ucs::variable_set &variables);
parse_expression::composition export_composition(const arithmetic::choice &expr, const ucs::variable_set &variables);
