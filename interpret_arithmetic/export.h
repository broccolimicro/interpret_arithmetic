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

parse_expression::expression export_expression(const arithmetic::Value &v, const ucs::variable_set &variables);
parse_expression::expression export_expression(const arithmetic::State &s, const ucs::variable_set &variables);
parse_expression::composition export_composition(const arithmetic::State &s, const ucs::variable_set &variables);
parse_expression::composition export_composition(const arithmetic::Region &r, const ucs::variable_set &variables);
parse_expression::expression export_expression(const arithmetic::Expression &expr, const ucs::variable_set &variables);
parse_expression::assignment export_assignment(const arithmetic::Action &expr, const ucs::variable_set &variables);
parse_expression::composition export_composition(const arithmetic::Parallel &expr, const ucs::variable_set &variables);
parse_expression::composition export_composition(const arithmetic::Choice &expr, const ucs::variable_set &variables);
