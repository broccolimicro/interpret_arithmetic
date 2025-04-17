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
#include "interface.h"

namespace parse_verilog {

variable_name export_variable_name(int variable, arithmetic::ConstNetlist variables);
string export_value(const arithmetic::Value &v);
expression export_expression(const arithmetic::Value &v);
expression export_expression(const arithmetic::State &s, arithmetic::ConstNetlist variables);
expression export_expression(const arithmetic::Expression &expr, arithmetic::ConstNetlist variables);

}
