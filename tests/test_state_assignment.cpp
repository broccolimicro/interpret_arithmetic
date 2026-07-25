#include <gtest/gtest.h>
#include <parse/default/line_comment.h>
#include <parse/default/block_comment.h>
#include <string>

#include "expression.h"
#include "import_expr.h"
#include "export_expr.h"
#include <interpret_arithmetic/export.h>

#include <common/mock_netlist.h>

using namespace std;
using namespace test;

TEST(StateAssignmentParser, BasicAssignmentPlus) {
	string test_code = "a+";
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	test::composition::register_syntax(tokens);
	tokens.insert("composition_plus", test_code);

	MockNetlist v;
	
	test::composition in(tokens);
	arithmetic::Region region = test::import_composition(in, v, &tokens).evaluate(arithmetic::State());
	auto out = test::export_composition(arithmetic::Choice(region), v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
	EXPECT_EQ(out.to_string(), "a+");
}

TEST(StateAssignmentParser, BasicAssignmentMinus) {
	// Test basic test::compositionwith minus operation (removal)
	string test_code = "b-";
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	test::composition::register_syntax(tokens);
	tokens.insert("composition_minus", test_code);

	MockNetlist v;
	
	test::composition in(tokens);
	arithmetic::Region region = test::import_composition(in, v, &tokens).evaluate(arithmetic::State());
	auto out = test::export_composition(arithmetic::Choice(region), v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
	EXPECT_EQ(out.to_string(), "b-");
}

TEST(StateAssignmentParser, AssignmentWithValue) {
	// Test test::compositionwith equality
	string test_code = "c = 1";
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	test::composition::register_syntax(tokens);
	tokens.insert("composition_value", test_code);

	MockNetlist v;
	
	test::composition in(tokens);
	arithmetic::Region region = test::import_composition(in, v, &tokens).evaluate(arithmetic::State());
	auto out = test::export_composition(arithmetic::Choice(region), v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
	EXPECT_EQ(out.to_string(), "c=1");
}

TEST(StateAssignmentParser, AssignmentWithGndVdd) {
	// Test test::compositionwith gnd and vdd
	string test_code = "d = true";
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	test::composition::register_syntax(tokens);
	tokens.insert("composition_vdd", test_code);

	MockNetlist v;
	
	test::composition in(tokens);
	arithmetic::Region region = test::import_composition(in, v, &tokens).evaluate(arithmetic::State());
	auto out = test::export_composition(arithmetic::Choice(region), v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
	EXPECT_EQ(out.to_string(), "d=true");
	
	// Test with gnd
	test_code = "e = false";
	
	tokenizer tokens2;
	tokens2.register_token<parse::block_comment>(false);
	tokens2.register_token<parse::line_comment>(false);
	test::composition::register_syntax(tokens2);
	tokens2.insert("composition_gnd", test_code);

	test::composition in2(tokens2);
	arithmetic::Region region2 = test::import_composition(in2, v, &tokens2).evaluate(arithmetic::State());
	auto out2 = test::export_composition(arithmetic::Choice(region2), v);

	EXPECT_TRUE(tokens2.is_clean());
	EXPECT_TRUE(out2.valid);
	EXPECT_EQ(out2.to_string(), "e=false");
}

