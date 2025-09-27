#include "export.h"
#include "support.h"
#include <arithmetic/algorithm.h>

namespace arithmetic {

string export_value(const Value &v) {
	if (v.isUndef()) {
		return "undef";
	} else if (v.isUnstable()) {
		return "unstable";
	} else if (v.isUnknown()) {
		return "unknown";
	} else if (v.type == Value::WIRE) {
		if (v.isNeutral()) {
			return "gnd";
		} else if (v.isValid()) {
			return "vdd";
		}
	} else if (v.type == Value::BOOL) {
		return ::to_string(v.bval);
	} else if (v.type == Value::INT) {
		return ::to_string(v.ival);
	} else if (v.type == Value::REAL) {
		return ::to_string(v.rval);
	} else if (v.type == Value::STRING) {
		return v.sval;
	}
	internal("", "unrecognized value in export_value()", __FILE__, __LINE__);
	return "";
}

}
