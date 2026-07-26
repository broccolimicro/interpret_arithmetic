#include <gtest/gtest.h>
#include <parse/default/line_comment.h>
#include <parse/default/block_comment.h>
#include <string>

#include <common/mock_netlist.h>

#include "expression.h"
#include "import_expr.h"
#include "export_expr.h"

using namespace std;

TEST(CompositionParser, BasicParallelComposition) {
	// Test parallel test::composition (,)
	string test_code = "a+, b+, c-";
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	test::composition::register_syntax(tokens);
	tokens.insert("parallel_test", test_code);

	MockNetlist v;
	
	test::composition in(tokens);
	arithmetic::Choice expr = test::import_composition(in, v, &tokens);
	auto out = test::export_composition(expr, v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
	EXPECT_EQ(out.to_string(), "a+,b+,c-");
}

TEST(CompositionParser, ComplexParallelComposition) {
	// Test complex parallel composition
	string test_code = "a+, b = c & d, e-";
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	test::composition::register_syntax(tokens);
	tokens.insert("complex_parallel", test_code);

	MockNetlist v;
	
	test::composition in(tokens);
	arithmetic::Choice expr = test::import_composition(in, v, &tokens);
	auto out = test::export_composition(expr, v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
	EXPECT_TRUE(out.to_string().find("a+") != string::npos);
	EXPECT_TRUE(out.to_string().find("b=") != string::npos);
	EXPECT_TRUE(out.to_string().find("e-") != string::npos);
}

TEST(CompositionParser, BasicChoice) {
	// Test internal choice (:)
	string test_code = "(a+) : (b-)";
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	test::composition::register_syntax(tokens);
	tokens.insert("choice_test", test_code);

	MockNetlist v;
	
	test::composition in(tokens);
	arithmetic::Choice expr = test::import_composition(in, v, &tokens);
	auto out = test::export_composition(expr, v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
	EXPECT_EQ(out.to_string(), "a+:b-");
}

TEST(CompositionParser, ComplexChoice) {
	// Test more complex choice
	string test_code = "(a = x & y) : (b-, c+)";
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	test::composition::register_syntax(tokens);
	tokens.insert("complex_choice", test_code);

	MockNetlist v;
	
	test::composition in(tokens);
	arithmetic::Choice expr = test::import_composition(in, v, &tokens);
	auto out = test::export_composition(expr, v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
	EXPECT_EQ(out.to_string(), "a=x&y:b-,c+");
}

TEST(CompositionParser, NestedCompositions) {
	// Test nested compositions
	string test_code = "(a+, b+) : (c-, (d+ : e+))";
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	test::composition::register_syntax(tokens);
	tokens.insert("nested_test", test_code);

	MockNetlist v;
	
	test::composition in(tokens);
	// This might need to be adapted based on how nested compositions are handled
	arithmetic::Choice expr = test::import_composition(in, v, &tokens);
	auto out = test::export_composition(expr, v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
	EXPECT_EQ(out.to_string(), "a+,b+:c-,d+:c-,e+");
}

TEST(CompositionParser, GuardedCompositions) {
	// Test guarded compositions
	string test_code = "(c+, d+) : A.send(e) : (f-)";
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	test::composition::register_syntax(tokens);
	tokens.insert("guarded_test", test_code);

	MockNetlist v;
	
	test::composition in(tokens);
	// This might need adaptation based on how guarded compositions are handled
	arithmetic::Choice expr = test::import_composition(in, v, &tokens);
	auto out = test::export_composition(expr, v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
	EXPECT_EQ(out.to_string(), "c+,d+:A.send(e):f-");
}

TEST(CompositionParser, RoundTripConversion) {
	// Test round-trip conversion
	string test_code = "a+, b-, c = d & e";
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	test::composition::register_syntax(tokens);
	tokens.insert("round_trip", test_code);

	MockNetlist v;
	
	test::composition in(tokens);
	arithmetic::Choice expr = test::import_composition(in, v, &tokens);
	auto out = test::export_composition(expr, v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
	EXPECT_EQ(out.to_string(), "a+,b-,c=d&e");
}

TEST(CompositionParser, NestedCalls) {
	string test_code = "add(a,mul(b, c))";
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	test::composition::register_syntax(tokens);
	tokens.insert("nested_calls", test_code);

	MockNetlist v;
	
	test::composition in(tokens);
	arithmetic::Choice expr = test::import_composition(in, v, &tokens);
	auto out = test::export_composition(expr, v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
	EXPECT_EQ(out.to_string(), "add(a,mul(b,c))");
}

TEST(CompositionParser, NestedMemberCalls) {
	string test_code = "A.send(B.peek())";
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	test::composition::register_syntax(tokens);
	tokens.insert("nested_member_calls", test_code);

	MockNetlist v;
	
	test::composition in(tokens);
	arithmetic::Choice expr = test::import_composition(in, v, &tokens);
	auto out = test::export_composition(expr, v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
	EXPECT_EQ(out.to_string(), "A.send(B.peek())");
}
