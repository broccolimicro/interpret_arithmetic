#include <gtest/gtest.h>
#include <parse/default/line_comment.h>
#include <parse/default/block_comment.h>
#include <sstream>
#include <string>

#include <parse_verilog/expression.h>
#include <interpret_arithmetic/import.h>
#include <interpret_arithmetic/export_verilog.h>
#include <common/mock_netlist.h>

using namespace std;

using namespace parse_verilog;

using composition=parse_expression::composition_t<expr_group, parse_verilog::number>;

TEST(VerilogExportParser, BasicBooleanOperations) {
	// Test exporting boolean operations to Verilog
	string test_code = "a & b | ~c";
	
	tokenizer tokens;
	parse_verilog::setup_expressions();
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	expression::register_syntax(tokens);
	tokens.insert("basic_boolean", test_code);

	MockNetlist v;
	
	expression in(tokens);
	arithmetic::Expression expr = arithmetic::import_expression(in, v, 0, &tokens, true);
	
	// Export to Verilog expression
	expression verilog_expr = parse_verilog::export_expression(expr, v);
	
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
	parse_verilog::setup_expressions();
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	expression::register_syntax(tokens);
	tokens.insert("arithmetic_ops", test_code);

	MockNetlist v;
	
	expression in(tokens);
	arithmetic::Expression expr = arithmetic::import_expression(in, v, 0, &tokens, true);
	
	// Export to Verilog expression
	expression verilog_expr = parse_verilog::export_expression(expr, v);
	
	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(verilog_expr.valid);
	
	// Check for arithmetic operators
	string result = verilog_expr.to_string();
	EXPECT_TRUE(result.find("+") != string::npos);
	EXPECT_TRUE(result.find("*") != string::npos);
}

TEST(VerilogExportParser, ComparisonOperations) {
	// Test exporting comparison operations to Verilog
	string test_code = "a < b && c == d";
	
	tokenizer tokens;
	parse_verilog::setup_expressions();
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	expression::register_syntax(tokens);
	tokens.insert("comparison_ops", test_code);

	MockNetlist v;
	
	expression in(tokens);
	arithmetic::Expression expr = arithmetic::import_expression(in, v, 0, &tokens, true);
	
	// Export to Verilog expression
	expression verilog_expr = parse_verilog::export_expression(expr, v);
	
	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(verilog_expr.valid);
	EXPECT_EQ(verilog_expr.to_string(), "a<b&&c==d");
}

TEST(VerilogExportParser, ExportState) {
	// Test exporting state to Verilog
	string test_code = "a+";
	
	tokenizer tokens;
	parse_verilog::setup_expressions();
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	composition::register_syntax(tokens);
	tokens.insert("state_test", test_code);

	MockNetlist v;
	
	composition in(tokens);
	arithmetic::State state = arithmetic::import_state(in, v, 0, &tokens, true);
	
	// Export to Verilog expression
	expression verilog_expr = parse_verilog::export_expression(state, v);
	
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
	string test_code = "(a && b) || (c && !d) || (e < f)";
	
	tokenizer tokens;
	parse_verilog::setup_expressions();
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	expression::register_syntax(tokens);
	tokens.insert("complex_expr", test_code);

	MockNetlist v;
	
	expression in(tokens);
	arithmetic::Expression expr = arithmetic::import_expression(in, v, 0, &tokens, true);
	
	// Export to Verilog expression
	expression verilog_expr = parse_verilog::export_expression(expr, v);
	
	EXPECT_TRUE(tokens.is_clean());
	EXPECT_TRUE(verilog_expr.valid);
	EXPECT_EQ(verilog_expr.to_string(), "a&&b||c&&!d||e<f");
}
