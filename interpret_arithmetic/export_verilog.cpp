#include "export_verilog.h"

#include <arithmetic/algorithm.h>
#include <common/message.h>

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

}
