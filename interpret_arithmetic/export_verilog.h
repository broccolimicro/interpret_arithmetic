#pragma once

#include <common/standard.h>
#include <common/net.h>

#include <parse_verilog/expression.h>

#include <arithmetic/expression.h>
#include <arithmetic/state.h>
#include <arithmetic/action.h>

namespace parse_verilog {

string export_value(const arithmetic::Value &v);

expression export_expression(const arithmetic::Value &v);
expression export_expression(const arithmetic::State &s, ucs::ConstNetlist nets);
composition export_composition(const arithmetic::State &s, ucs::ConstNetlist nets);
composition export_composition(const arithmetic::Region &r, ucs::ConstNetlist nets);
expression export_expression(const arithmetic::Expression &expr, ucs::ConstNetlist nets);
assignment export_assignment(const arithmetic::Action &expr, ucs::ConstNetlist nets);
composition export_composition(const arithmetic::Parallel &expr, ucs::ConstNetlist nets);
composition export_composition(const arithmetic::Choice &expr, ucs::ConstNetlist nets);

}
