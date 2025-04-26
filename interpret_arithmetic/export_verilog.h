#pragma once

#include <common/standard.h>
#include <common/net.h>

#include <parse_verilog/expression.h>

#include <arithmetic/expression.h>
#include <arithmetic/state.h>

namespace parse_verilog {

string export_value(const arithmetic::Value &v);
expression export_expression(const arithmetic::Value &v);
expression export_expression(const arithmetic::State &s, ucs::ConstNetlist variables);
expression export_expression(const arithmetic::Expression &expr, ucs::ConstNetlist variables);

}
