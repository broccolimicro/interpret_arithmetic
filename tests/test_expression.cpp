#include <gtest/gtest.h>
#include <parse/default/line_comment.h>
#include <parse/default/block_comment.h>
#include <parse_expression/expression.h>
#include <sstream>
#include <string>

#include <arithmetic/rewrite.h>

#include <interpret_arithmetic/import.h>
#include <interpret_arithmetic/export.h>
#include "test_helpers.h"

using namespace std;
using namespace parse_expression;

TEST(ExpressionParser, BasicBooleanOperations) {
	// Test simple AND, OR, NOT operations
	string test_code = "a & b | ~c";
	
	expression::register_precedence(createPrecedence());
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	expression::register_syntax(tokens);
	tokens.insert("basic_boolean", test_code);

	VariableSet v;
	
	expression in(tokens);
	arithmetic::Expression expr = arithmetic::import_expression(in, v, 0, &tokens, true);
	
	expr.minimize();
	expr.minimize(arithmetic::rewriteHuman());

	expression out = export_expression(expr, v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
	EXPECT_EQ(out.to_string(), "a&b|~c");
}

TEST(ExpressionParser, ComplexBooleanOperations) {
	// Test more complex boolean expressions
	string test_code = "(a & b) | (~c & d)";
	
	expression::register_precedence(createPrecedence());
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	expression::register_syntax(tokens);
	tokens.insert("complex_boolean", test_code);

	VariableSet v;
	
	expression in(tokens);
	arithmetic::Expression expr = arithmetic::import_expression(in, v, 0, &tokens, true);
	expr.minimize();
	expr.minimize(arithmetic::rewriteHuman());
	expression out = export_expression(expr, v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
	EXPECT_EQ(out.to_string(), "a&b|d&~c");
}

TEST(ExpressionParser, ArithmeticOperations) {
	// Test arithmetic operations
	string test_code = "a + b * c";
	
	expression::register_precedence(createPrecedence());
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	expression::register_syntax(tokens);
	tokens.insert("arithmetic_ops", test_code);

	VariableSet v;
	
	expression in(tokens);
	arithmetic::Expression expr = arithmetic::import_expression(in, v, 0, &tokens, true);
	expression out = export_expression(expr, v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
	EXPECT_EQ(out.to_string(), "a+b*c");
}

TEST(ExpressionParser, ComparisonOperations) {
	// Test comparison operations
	string test_code = "a < b & c == d";
	
	expression::register_precedence(createPrecedence());
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	expression::register_syntax(tokens);
	tokens.insert("comparison_ops", test_code);

	VariableSet v;
	
	expression in(tokens);
	arithmetic::Expression expr = arithmetic::import_expression(in, v, 0, &tokens, true);
	expr.minimize();
	expr.minimize(arithmetic::rewriteHuman());
	expression out = export_expression(expr, v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
	EXPECT_EQ(out.to_string(), "a<b&c==d");
}

TEST(ExpressionParser, MixedOperations) {
	// Test mixing boolean, arithmetic, and comparison operations
	string test_code = "(a + b > c) & (d * e < f)";
	
	expression::register_precedence(createPrecedence());
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	expression::register_syntax(tokens);
	tokens.insert("mixed_ops", test_code);

	VariableSet v;
	
	expression in(tokens);
	arithmetic::Expression expr = arithmetic::import_expression(in, v, 0, &tokens, true);
	expr.minimize();
	expr.minimize(arithmetic::rewriteHuman());
	expression out = export_expression(expr, v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
	EXPECT_EQ(out.to_string(), "c<a+b&d*e<f");
}

TEST(ExpressionParser, NegationAndIdentity) {
	// Test unary operations
	string test_code = "+a & -b";
	
	expression::register_precedence(createPrecedence());
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	expression::register_syntax(tokens);
	tokens.insert("unary_ops", test_code);

	VariableSet v;
	
	expression in(tokens);
	arithmetic::Expression expr = arithmetic::import_expression(in, v, 0, &tokens, true);
	expr.minimize();
	expr.minimize(arithmetic::rewriteHuman());
	expression out = export_expression(expr, v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
	EXPECT_EQ(out.to_string(), "a&b");
}

TEST(ExpressionParser, BitShifting) {
	// Test bit shifting operations
	string test_code = "a << 2 | b >> 3";
	
	expression::register_precedence(createPrecedence());
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	expression::register_syntax(tokens);
	tokens.insert("shift_ops", test_code);

	VariableSet v;
	
	expression in(tokens);
	arithmetic::Expression expr = arithmetic::import_expression(in, v, 0, &tokens, true);
	expr.minimize();
	expr.minimize(arithmetic::rewriteHuman());
	expression out = export_expression(expr, v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
	EXPECT_EQ(out.to_string(), "a<<2|b>>3");
}

TEST(ExpressionParser, Constants) {
	// Test numeric constants
	string test_code = "a & 42 | b & 0";
	
	expression::register_precedence(createPrecedence());
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	expression::register_syntax(tokens);
	tokens.insert("constants", test_code);

	VariableSet v;
	
	expression in(tokens);
	arithmetic::Expression expr = arithmetic::import_expression(in, v, 0, &tokens, true);
	expr.minimize();
	expr.minimize(arithmetic::rewriteHuman());
	expression out = export_expression(expr, v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
	EXPECT_EQ(out.to_string(), "a|b");
}

TEST(ExpressionParser, TrueFalse) {
	// Test gnd and vdd constants
	string test_code = "a & true | b & false";
	
	expression::register_precedence(createPrecedence());
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	expression::register_syntax(tokens);
	tokens.insert("true_false", test_code);

	VariableSet v;
	
	expression in(tokens);
	arithmetic::Expression expr = arithmetic::import_expression(in, v, 0, &tokens, true);
	expr.minimize();
	expr.minimize(arithmetic::rewriteHuman());
	expression out = export_expression(expr, v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
	EXPECT_EQ(out.to_string(), "(bool)a");
}

TEST(ExpressionParser, DifferentRegions) {
	// Test expressions with region specifications
	string test_code = "a'1 & b'2 | c'3";
	
	expression::register_precedence(createPrecedence());
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	expression::register_syntax(tokens);
	tokens.insert("regions", test_code);

	VariableSet v;

	
	expression in(tokens);
	arithmetic::Expression expr = arithmetic::import_expression(in, v, 0, &tokens, true);
	expression out = export_expression(expr, v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
	EXPECT_EQ(out.to_string(), "a'1&b'2|c'3");
}

TEST(ExpressionParser, Function) {
	string test_code = "x + y.myfunc(a, b, c)";
	
	expression::register_precedence(createPrecedence());
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	expression::register_syntax(tokens);
	tokens.insert("function", test_code);

	VariableSet v;
	
	expression in(tokens);
	arithmetic::Expression expr = arithmetic::import_expression(in, v, 0, &tokens, true);
	expr.minimize();
	expr.minimize(arithmetic::rewriteHuman());
	expression out = export_expression(expr, v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
	EXPECT_EQ(out.to_string(), "x+y.myfunc(a,b,c)");
}

TEST(ExpressionParser, EmptyFunction) {
	string test_code = "x + y.myfunc()";
	
	expression::register_precedence(createPrecedence());
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	expression::register_syntax(tokens);
	tokens.insert("function", test_code);

	VariableSet v;
	
	expression in(tokens);
	arithmetic::Expression expr = arithmetic::import_expression(in, v, 0, &tokens, true);
	expr.minimize();
	expr.minimize(arithmetic::rewriteHuman());
	expression out = export_expression(expr, v);

	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(out.valid);
	EXPECT_EQ(out.to_string(), "x+y.myfunc()");
}

