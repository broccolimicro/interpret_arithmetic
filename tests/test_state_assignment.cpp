#include <gtest/gtest.h>
#include <parse/default/line_comment.h>
#include <parse/default/block_comment.h>
#include <parse_expression/expression.h>
#include <parse_expression/composition.h>
#include <sstream>
#include <string>

#include <interpret_arithmetic/import.h>
#include <interpret_arithmetic/export.h>
#include "test_helpers.h"

using namespace std;
using namespace parse_expression;

TEST(StateAssignmentParser, BasicAssignmentPlus) {
	string test_code = "a+";
	
	expression::register_precedence(createPrecedence());
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	composition::register_syntax(tokens);
	tokens.insert("composition_plus", test_code);

	VariableSet v;
	
	composition in(tokens);
	arithmetic::State state = arithmetic::import_state(in, v, 0, &tokens, true);
	composition out = export_composition(state, v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
	EXPECT_EQ(out.to_string(), "a+");
}

TEST(StateAssignmentParser, BasicAssignmentMinus) {
	// Test basic composition with minus operation (removal)
	string test_code = "b-";
	
	expression::register_precedence(createPrecedence());
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	composition::register_syntax(tokens);
	tokens.insert("composition_minus", test_code);

	VariableSet v;
	
	composition in(tokens);
	arithmetic::State state = arithmetic::import_state(in, v, 0, &tokens, true);
	composition out = export_composition(state, v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
	EXPECT_EQ(out.to_string(), "b-");
}

/*TEST(StateAssignmentParser, AssignmentWithValue) {
	// Test composition with equality
	string test_code = "c = 1";
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	composition::register_syntax(tokens);
	tokens.insert("composition_value", test_code);

	VariableSet v;
	
	composition in(tokens);
	arithmetic::State state = arithmetic::import_state(in, v, 0, &tokens, true);
	composition out = export_composition(state, v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
	EXPECT_EQ(out.to_string(), "c=1");
}

TEST(StateAssignmentParser, AssignmentWithGndVdd) {
	// Test composition with gnd and vdd
	string test_code = "d = true";
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	composition::register_syntax(tokens);
	tokens.insert("composition_vdd", test_code);

	VariableSet v;
	
	composition in(tokens);
	arithmetic::State state = arithmetic::import_state(in, v, 0, &tokens, true);
	composition out = export_composition(state, v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
	EXPECT_EQ(out.to_string(), "d+");
	
	// Test with gnd
	test_code = "e = false";
	
	tokenizer tokens2;
	tokens2.register_token<parse::block_comment>(false);
	tokens2.register_token<parse::line_comment>(false);
	composition::register_syntax(tokens2);
	tokens2.insert("composition_gnd", test_code);

	composition in2(tokens2);
	arithmetic::State state2 = arithmetic::import_state(in2, v, 0, &tokens2, true);
	composition out2 = export_composition(state2, v);

	EXPECT_TRUE(tokens2.is_clean());
	EXPECT_TRUE(out2.valid);
	EXPECT_EQ(out2.to_string(), "e-");
}*/

