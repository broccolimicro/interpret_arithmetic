#include "import.h"
#include "support.h"

namespace arithmetic {

int import_net(string syntax, ucs::Netlist nets, tokenizer *tokens, bool auto_define) {
	int uid = nets.netIndex(syntax, auto_define);
	if (uid < 0) {
		if (tokens != nullptr) {
			tokens->error("undefined net '" + syntax + "'", __FILE__, __LINE__);
		} else {
			error("", "undefined net '" + syntax + "'", __FILE__, __LINE__);
		}
	}

	return uid;
}

arithmetic::Operation::OpType import_operator(parse_expression::operation op) {
	for (int i = 0; i < (int)Operation::operators.size(); i++) {
		if (areSame(Operation::operators[i], op)) {
			if (Operation::operators.is_valid(i)) {
				return (Operation::OpType)i;
			} else {
				return Operation::OpType::UNDEF;
			}
		}
	}
	return arithmetic::Operation::OpType::UNDEF;
}

}
