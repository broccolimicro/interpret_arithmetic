#include "support.h"

namespace arithmetic {

bool areSame(arithmetic::Operator o0, parse_expression::operation o1) {
	return o0.prefix == o1.prefix
		and o0.trigger == o1.trigger
		and o0.infix == o1.infix
		and o0.postfix == o1.postfix;
}

}
