/*
 * export.h
 *
 *  Created on: Feb 6, 2015
 *      Author: nbingham
 */

#pragma once

#include <common/standard.h>

#include <ucs/variable.h>

#include <parse_verilog/expression.h>

#include <arithmetic/expression.h>
#include <arithmetic/state.h>

namespace parse_verilog {

slice export_slice(int lb, int ub, const ucs::variable_set &variables);
member_name export_member_name(ucs::instance instance, const ucs::variable_set &variables);
variable_name export_variable_name(ucs::variable variable, const ucs::variable_set &variables);
variable_name export_variable_name(int variable, const ucs::variable_set &variables);
expression export_expression(const arithmetic::value &v);
expression export_expression(const arithmetic::state &s, const ucs::variable_set &variables);
expression export_expression(const arithmetic::expression &expr, const ucs::variable_set &variables);

}
