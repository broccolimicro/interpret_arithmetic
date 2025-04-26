#include <gtest/gtest.h>
#include <parse/default/line_comment.h>
#include <parse/default/block_comment.h>
#include <parse_expression/expression.h>
#include <parse_verilog/expression.h>
#include <sstream>
#include <string>

#include <interpret_arithmetic/import.h>
#include <interpret_arithmetic/export_verilog.h>
#include "test_helpers.h"

using namespace std;
using namespace parse_expression;

//==============================================================================
// Verilog Export Tests
//==============================================================================

TEST(VerilogExportParser, BasicBooleanOperations) {
	// Test exporting boolean operations to Verilog
	string test_code = "a & b | ~c";
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	expression::register_syntax(tokens);
	tokens.insert("basic_boolean", test_code);

	VariableSet v;
	
	expression in(tokens);
	arithmetic::Expression expr = arithmetic::import_expression(in, v, 0, &tokens, true);
	
	// Export to Verilog expression
	parse_verilog::expression verilog_expr = parse_verilog::export_expression(expr, v);
	
	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(verilog_expr.valid);
	
	// Verilog should use & and | for boolean operations
	string result = verilog_expr.to_string();
	EXPECT_TRUE(result.find("&") != string::npos);
	EXPECT_TRUE(result.find("|") != string::npos);
	EXPECT_TRUE(result.find("~") != string::npos || result.find("!") != string::npos);
}

TEST(VerilogExportParser, ArithmeticOperations) {
	// Test exporting arithmetic operations to Verilog
	string test_code = "a + b * c";
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	expression::register_syntax(tokens);
	tokens.insert("arithmetic_ops", test_code);

	VariableSet v;
	
	expression in(tokens);
	arithmetic::Expression expr = arithmetic::import_expression(in, v, 0, &tokens, true);
	
	// Export to Verilog expression
	parse_verilog::expression verilog_expr = parse_verilog::export_expression(expr, v);
	
	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(verilog_expr.valid);
	
	// Check for arithmetic operators
	string result = verilog_expr.to_string();
	EXPECT_TRUE(result.find("+") != string::npos);
	EXPECT_TRUE(result.find("*") != string::npos);
}

TEST(VerilogExportParser, ComparisonOperations) {
	// Test exporting comparison operations to Verilog
	string test_code = "a < b & c == d";
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	expression::register_syntax(tokens);
	tokens.insert("comparison_ops", test_code);

	VariableSet v;
	
	expression in(tokens);
	arithmetic::Expression expr = arithmetic::import_expression(in, v, 0, &tokens, true);
	
	// Export to Verilog expression
	parse_verilog::expression verilog_expr = parse_verilog::export_expression(expr, v);
	
	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(verilog_expr.valid);
	
	// Check for comparison operators
	string result = verilog_expr.to_string();
	EXPECT_TRUE(result.find("<") != string::npos);
	EXPECT_TRUE(result.find("==") != string::npos);
}

TEST(VerilogExportParser, ExportState) {
	// Test exporting state to Verilog
	string test_code = "a+";
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	composition::register_syntax(tokens);
	tokens.insert("state_test", test_code);

	VariableSet v;
	
	parse_expression::composition in(tokens);
	arithmetic::State state = arithmetic::import_state(in, v, 0, &tokens, true);
	
	// Export to Verilog expression
	parse_verilog::expression verilog_expr = parse_verilog::export_expression(state, v);
	
	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(verilog_expr.valid);
	
	// State should be converted to an expression in Verilog
	string result = verilog_expr.to_string();
	EXPECT_TRUE(result.find("a") != string::npos);
}

TEST(VerilogExportParser, ExportValue) {
	// Test exporting constant values to Verilog
	
	// Boolean true value
	string value_true = parse_verilog::export_value(arithmetic::Value(true));
	EXPECT_TRUE(value_true == "1" || value_true == "1'b1");
	
	// Boolean false value
	string value_false = parse_verilog::export_value(arithmetic::Value(false));
	EXPECT_TRUE(value_false == "0" || value_false == "1'b0");
	
	// Numeric value
	string value_num = parse_verilog::export_value(arithmetic::Value(42));
	EXPECT_TRUE(value_num == "42" || value_num.find("'d42") != string::npos);
}

TEST(VerilogExportParser, ComplexExpression) {
	// Test exporting complex expressions to Verilog
	string test_code = "(a & b) | (c & ~d) | (e < f)";
	
	tokenizer tokens;
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	expression::register_syntax(tokens);
	tokens.insert("complex_expr", test_code);

	VariableSet v;
	
	expression in(tokens);
	arithmetic::Expression expr = arithmetic::import_expression(in, v, 0, &tokens, true);
	
	// Export to Verilog expression
	parse_verilog::expression verilog_expr = parse_verilog::export_expression(expr, v);
	
	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(verilog_expr.valid);
	EXPECT_EQ(verilog_expr.to_string(), "a&&b||c&&!(d)||e<f");
}

TEST(VerilogExportParser, ExportVariableName) {
	// Test exporting variable names to Verilog
	VariableSet v;
	
	// Add some variables
	v.netIndex("simple", true);         // simple name
	v.netIndex("complex.name", true);   // hierarchical name
	v.netIndex("array[0]", true);       // array name
	
	// Export to Verilog variable names
	parse_verilog::variable_name var0 = v.netAt(0);
	parse_verilog::variable_name var1 = v.netAt(1);
	parse_verilog::variable_name var2 = v.netAt(2);
	
	EXPECT_TRUE(var0.valid);
	EXPECT_TRUE(var1.valid);
	EXPECT_TRUE(var2.valid);
	
	// Check for proper variable naming
	EXPECT_EQ(var0.to_string(), "simple");
	
	// These may be transformed in Verilog syntax
	string complex_result = var1.to_string();
	EXPECT_TRUE(complex_result.find("complex") != string::npos);
	
	string array_result = var2.to_string();
	EXPECT_TRUE(array_result.find("array") != string::npos);
} 
