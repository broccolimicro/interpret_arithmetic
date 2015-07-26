/*
 * import.h
 *
 *  Created on: Feb 6, 2015
 *      Author: nbingham
 */

#include <common/standard.h>

#include <parse/tokenizer.h>
#include <ucs/variable.h>

#include <parse_expression/expression.h>
#include <parse_expression/assignment.h>
#include <parse_expression/composition.h>

#include <arithmetic/expression.h>
#include <arithmetic/assignment.h>


#ifndef interpret_arithmetic_import_h
#define interpret_arithmetic_import_h

arithmetic::expression import_expression(const parse_expression::expression &syntax, ucs::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define);
arithmetic::assignment import_assignment(const parse_expression::assignment &syntax, ucs::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define);
vector<vector<arithmetic::assignment> > import_composition(const parse_expression::composition &syntax, ucs::variable_set &variables, int default_id, tokenizer *tokens, bool auto_define);


#endif
