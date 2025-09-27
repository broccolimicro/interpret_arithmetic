#include "export_verilog.h"

#include <arithmetic/algorithm.h>
#include <common/message.h>
#include "export.h"

namespace parse_verilog {

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

expression export_expression(const arithmetic::Value &v) {
	return arithmetic::export_expression<parse_verilog::expression>(v, export_value);
}

expression export_expression(const arithmetic::State &s, ucs::ConstNetlist nets) {
	return arithmetic::export_expression<parse_verilog::expression>(s, nets, export_value);
}

composition export_composition(const arithmetic::State &s, ucs::ConstNetlist nets) {
	return arithmetic::export_composition<parse_verilog::composition>(s, nets, export_value);
}

composition export_composition(const arithmetic::Region &r, ucs::ConstNetlist nets) {
	return arithmetic::export_composition<parse_verilog::composition>(r, nets, export_value);
}

expression export_expression(const arithmetic::Expression &expr, ucs::ConstNetlist nets) {
	return arithmetic::export_expression<parse_verilog::expression>(expr, nets, export_value);
}

assignment export_assignment(const arithmetic::Action &expr, ucs::ConstNetlist nets) {
	return arithmetic::export_assignment<parse_verilog::assignment>(expr, nets, export_value);
}

composition export_composition(const arithmetic::Parallel &expr, ucs::ConstNetlist nets) {
	return arithmetic::export_composition<parse_verilog::composition>(expr, nets, export_value);
}

composition export_composition(const arithmetic::Choice &expr, ucs::ConstNetlist nets) {
	return arithmetic::export_composition<parse_verilog::composition>(expr, nets, export_value);
}

}
