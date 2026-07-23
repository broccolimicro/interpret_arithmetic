#include "export_verilog.h"

#include <arithmetic/algorithm.h>
#include <common/message.h>
#include "export.h"

#include <parse_verilog/expression.h>

namespace parse_verilog {

pair<int, int> ExpressionExporter::export_operator(int func) const {
	switch (func) {
	case arithmetic::Operation::VALIDITY: return {-1, -1};
	case arithmetic::Operation::WIRE_NOT: return {13, 3};
	case arithmetic::Operation::WIRE_OR: return {13, 6};
	case arithmetic::Operation::WIRE_AND: return {13, 4};
	case arithmetic::Operation::WIRE_XOR: return {13, 3};
	}
	 
}

const parse_expression::precedence_set &ExpressionExporter::precedence() const {
	return parse_verilog::config::cfg->order;
}


string export_value(const arithmetic::Value &v) {
	if (v.isUnstable()) {
		return "X";
	} else if (v.isUnknown()) {
		return "U";
	} else if (v.isNeutral()) {
		return "0";
	} else if (v.type == arithmetic::Value::WIRE) {
		return "1";
	} else if (v.type == arithmetic::Value::BOOL) {
		return v.bval ? "1'b1" : "1'b0";
	} else if (v.type == arithmetic::Value::INT) {
		return ::to_string(v.ival);
	} else if (v.type == arithmetic::Value::REAL) {
		return ::to_string(v.rval);
	} else if (v.type == arithmetic::Value::STRING) {
		return v.sval;
	}
	internal("", "unrecognized value in export_value()", __FILE__, __LINE__);
	return "";
}

parse_expression::expression export_expression(const arithmetic::Value &v) {
	return arithmetic::export_expression<parse_verilog::expression>(v, export_value);
}

parse_expression::expression export_expression(const arithmetic::State &s, ucs::ConstNetlist nets) {
	return arithmetic::export_expression<parse_verilog::expression>(s, nets, export_value);
}

parse_expression::expression export_composition(const arithmetic::State &s, ucs::ConstNetlist nets) {
	return arithmetic::export_composition<parse_verilog::composition>(s, nets, export_value);
}

parse_expression::expression export_composition(const arithmetic::Region &r, ucs::ConstNetlist nets) {
	return arithmetic::export_composition<parse_verilog::composition>(r, nets, export_value);
}

parse_expression::expression export_expression(const arithmetic::Expression &expr, ucs::ConstNetlist nets) {
	return arithmetic::export_expression<parse_verilog::expression>(expr, nets, export_value);
}

parse_expression::assignment export_assignment(const arithmetic::Action &expr, ucs::ConstNetlist nets) {
	return arithmetic::export_assignment<parse_verilog::assignment>(expr, nets, export_value);
}

parse_expression::expression export_composition(const arithmetic::Parallel &expr, ucs::ConstNetlist nets) {
	return arithmetic::export_composition<parse_verilog::composition>(expr, nets, export_value);
}

parse_expression::expression export_composition(const arithmetic::Choice &expr, ucs::ConstNetlist nets) {
	return arithmetic::export_composition<parse_verilog::composition>(expr, nets, export_value);
}

}
