#include "import_default.h"

namespace arithmetic {

Expression import_literal(string name, ucs::Netlist symbols, tokenizer *tokens, bool auto_define) {
	int uid = symbols.netIndex(name, auto_define);
	if (uid >= 0) {
		return Expression::varOf(uid);
	}

	if (tokens != nullptr) {
		tokens->error("undefined symbol '" + name + "'", __FILE__, __LINE__);
	} else {
		error("", "undefined symbol '" + name + "'", __FILE__, __LINE__);
	}
	return Expression::undef();
}

Expression import_constant(string cnst, tokenizer *tokens) {
	if (cnst == "false") {
		return Expression::boolOf(false);
	} else if (cnst == "true") {
		return Expression::boolOf(true);
	} else if (cnst == "gnd") {
		return Expression::gnd();
	} else if (cnst == "vdd") {
		return Expression::vdd();
	} else if (cnst == "undef") {
		return Expression::undef();
	} else if (not cnst.empty()) {
		size_t n = cnst.find_first_of("0123456789");
		size_t m = cnst.find_first_of(".-+");

		if (n != 0) {
			if ((cnst[0] == '\"' and cnst.back() == '\"')
				or (cnst[0] == '\'' and cnst.back() == '\'')) {
				return Expression::stringOf(cnst.substr(1, cnst.size()-2));
			}
			return Expression::stringOf(cnst);
		} else if (m != string::npos) {
			return Expression::realOf(atof(cnst.c_str()));
		}
		return Expression::intOf(atoi(cnst.c_str()));
	}
	return Expression::X();
}

}
