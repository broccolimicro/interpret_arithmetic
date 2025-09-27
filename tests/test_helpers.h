#pragma once

#include <parse_expression/expression.h>
#include <parse_expression/composition.h>
#include <parse_expression/assignment.h>
#include <common/mock_netlist.h>

using expression = parse_expression::expression_t<>;
using composition = parse_expression::composition_t<>;
using assignment = parse_expression::assignment_t<>;

void setup_expressions();

