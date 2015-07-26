/*
 * export.h
 *
 *  Created on: Feb 6, 2015
 *      Author: nbingham
 */

#include <common/standard.h>

#include <ucs/variable.h>

#include <parse_expression/expression.h>
#include <parse_expression/assignment.h>
#include <parse_expression/composition.h>

#include <arithmetic/expression.h>
#include <arithmetic/assignment.h>

#ifndef interpret_arithmetic_export_h
#define interpret_arithmetic_export_h

parse_expression::expression export_expression(const arithmetic::expression &expr, ucs::variable_set &variables);
parse_expression::assignment export_assignment(const arithmetic::assignment &expr, ucs::variable_set &variables);
parse_expression::composition export_composition(const vector<arithmetic::assignment> &expr, ucs::variable_set &variables);
parse_expression::composition export_composition(const vector<vector<arithmetic::assignment> > &expr, ucs::variable_set &variables);

#endif
