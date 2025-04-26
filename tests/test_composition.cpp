#include <gtest/gtest.h>
#include <parse/default/line_comment.h>
#include <parse/default/block_comment.h>
#include <parse_expression/expression.h>
#include <parse_expression/assignment.h>
#include <parse_expression/composition.h>
#include <sstream>
#include <string>

#include <interpret_arithmetic/import.h>
#include <interpret_arithmetic/export.h>
#include "test_helpers.h"

using namespace std;
using namespace parse_expression;

//==============================================================================
// Composition Tests
//==============================================================================

TEST(CompositionParser, BasicParallelComposition) {
	// Test parallel composition (,)
	string test_code = "a+, b+, c-";
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	composition::register_syntax(tokens);
	tokens.insert("parallel_test", test_code);

	VariableSet v;
	
	composition in(tokens);
	arithmetic::Parallel parallel = arithmetic::import_parallel(in, v, 0, &tokens, true);
	composition out = export_composition(parallel, v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
	EXPECT_TRUE(out.to_string().find("a+") != string::npos);
	EXPECT_TRUE(out.to_string().find("b+") != string::npos);
	EXPECT_TRUE(out.to_string().find("c-") != string::npos);
}

TEST(CompositionParser, ComplexParallelComposition) {
	// Test complex parallel composition
	string test_code = "a+, b = c & d, e-";
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	composition::register_syntax(tokens);
	tokens.insert("complex_parallel", test_code);

	VariableSet v;
	
	composition in(tokens);
	arithmetic::Parallel parallel = arithmetic::import_parallel(in, v, 0, &tokens, true);
	composition out = export_composition(parallel, v);

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
	composition::register_syntax(tokens);
	tokens.insert("choice_test", test_code);

	VariableSet v;
	
	composition in(tokens);
	arithmetic::Choice choice = arithmetic::import_choice(in, v, 0, &tokens, true);
	composition out = export_composition(choice, v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
	EXPECT_TRUE(out.to_string().find("a+") != string::npos);
	EXPECT_TRUE(out.to_string().find("b-") != string::npos);
	EXPECT_TRUE(out.to_string().find(":") != string::npos);
}

TEST(CompositionParser, ComplexChoice) {
	// Test more complex choice
	string test_code = "(a = x & y) : (b-, c+)";
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	composition::register_syntax(tokens);
	tokens.insert("complex_choice", test_code);

	VariableSet v;
	
	composition in(tokens);
	arithmetic::Choice choice = arithmetic::import_choice(in, v, 0, &tokens, true);
	composition out = export_composition(choice, v);

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
	composition::register_syntax(tokens);
	tokens.insert("nested_test", test_code);

	VariableSet v;
	
	composition in(tokens);
	// This might need to be adapted based on how nested compositions are handled
	arithmetic::Choice choice = arithmetic::import_choice(in, v, 0, &tokens, true);
	composition out = export_composition(choice, v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
}

TEST(CompositionParser, GuardedCompositions) {
	// Test guarded compositions
	string test_code = "a & b ? (c+, d+) : e ? (f-)";
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	composition::register_syntax(tokens);
	tokens.insert("guarded_test", test_code);

	VariableSet v;
	
	composition in(tokens);
	// This might need adaptation based on how guarded compositions are handled
	arithmetic::Choice choice = arithmetic::import_choice(in, v, 0, &tokens, true);
	composition out = export_composition(choice, v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
}

TEST(CompositionParser, RoundTripConversion) {
	// Test round-trip conversion
	string test_code = "a+, b-, c = d & e";
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	composition::register_syntax(tokens);
	tokens.insert("round_trip", test_code);

	VariableSet v;
	
	composition in(tokens);
	arithmetic::Parallel parallel = arithmetic::import_parallel(in, v, 0, &tokens, true);
	composition out = export_composition(parallel, v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
	
	// Check the output contains all the original terms
	string result = out.to_string();
	EXPECT_TRUE(result.find("a+") != string::npos);
	EXPECT_TRUE(result.find("b-") != string::npos);
	EXPECT_TRUE(result.find("c=") != string::npos);
	EXPECT_TRUE(result.find("d") != string::npos);
	EXPECT_TRUE(result.find("e") != string::npos);
} 
