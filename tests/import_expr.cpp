#include "import_expr.h"
#include "expression.h"

arithmetic::Expression ExpressionInterpreter::import_unary(parse_expression::operation op, arithmetic::Expression expr) const {
	if (op.is("!", "", "", "")) {
		return !expr;
	} else if (op.is("~", "", "", "")) {
		return ~expr;
	} else if (op.is("+", "", "", "")) {
		return expr;
	} else if (op.is("-", "", "", "")) {
		return -expr;
	} else if (op.is("(bool)", "", "", "")) {
		return arithmetic::cast("bool", expr);
	} else if (op.is("#", "", "", "")) {
		return arithmetic::memberCall(expr, "peek", {});
	} else if (op.is("", "", "", "?")) {
		return arithmetic::memberCall(expr, "recv", {});
	}
	return expr;
}

arithmetic::Expression ExpressionInterpreter::import_binary(parse_expression::operation op, arithmetic::Expression left, arithmetic::Expression right) const {
	if (op.is("", "", "|", "")) {
		return left | right;
	} else if (op.is("", "", "&", "")) {
		return left & right;
	} else if (op.is("", "", "^", "")) {
		return left ^ right;
	} else if (op.is("", "", "==", "")) {
		return left == right;
	} else if (op.is("", "", "!=", "")) {
		return left != right;
	} else if (op.is("", "", "<", "")) {
		return left < right;
	} else if (op.is("", "", "<=", "")) {
		return left <= right;
	} else if (op.is("", "", ">", "")) {
		return left > right;
	} else if (op.is("", "", ">=", "")) {
		return left >= right;
	} else if (op.is("", "", "||", "")) {
		return left || right;
	} else if (op.is("", "", "&&", "")) {
		return left && right;
	} else if (op.is("", "", "<<", "")) {
		return left << right;
	} else if (op.is("", "", ">>", "")) {
		return left >> right;
	} else if (op.is("", "", "+", "")) {
		return left + right;
	} else if (op.is("", "", "-", "")) {
		return left - right;
	} else if (op.is("", "", "*", "")) {
		return left * right;
	} else if (op.is("", "", "/", "")) {
		return left / right;
	} else if (op.is("", "", "%", "")) {
		return left % right;
	}
	internal("", "unrecognized operation", __FILE__, __LINE__);
	return left;
}

arithmetic::Expression ExpressionInterpreter::import_group(parse_expression::operation op, vector<arithmetic::Expression> args) const {
	if (op.is("[", "", ",", "]")) {
		return arithmetic::array(args);
	}
	internal("", "unrecognized operation", __FILE__, __LINE__);
	return arithmetic::Expression();
}

arithmetic::Expression ExpressionInterpreter::import_modifier(parse_expression::operation op, vector<arithmetic::Expression> args) const {
	if (op.is("", "!", "", "")) {     // Channel Send
		if (not args.empty()) {
			return arithmetic::memberCall("send", args);
		} else {
			error(__FILE__, __LINE__, nullptr, nullptr, "operator '!' expects at least one operand");
			return arithmetic::memberCall("send", {arithmetic::Expression::undef()});
		}
	// TODO(edward.bingham) This operator is specific to QDI languages (CHP, HSE, PRS, COG)
	} /*else if (op.is("", "'", "", "")) { // Region
		if (args.size() != 2u) {
			error(__FILE__, __LINE__, nullptr, nullptr, "operator ''' expects 2 arguments, found '{}'", args.size());
		}
		string cnst = import_constant(args[1]);
		return import_argument(args[0], nets, atoi(cnst.c_str()));
	}*/ else if (op.is("", ".", "", "")) { // Member
		return arithmetic::Expression(arithmetic::Operation::MEMBER, args);
	// DESIGN(edward.bingham) Move "this" into the first argument of the
	// function. So "a.b.c(d, e) becomes c(a.b, d, e). This seems like a
	// reasonable way to simplify things, and follows the early style of c++
	// function names.
	} else if (op.is("", "(", ",", ")")) { // Call, Validity, Truthiness
		if (args.empty()) {
			error(__FILE__, __LINE__, nullptr, nullptr, "function call expects function name");
			return arithmetic::Expression();
		}

		// Replace member calls
		if (args[0].top.isExpr() and args[0].getExpr(args[0].top.index)->func == arithmetic::Operation::MEMBER) {
			arithmetic::Operation op = *args[0].getExpr(args[0].top.index);

			arithmetic::Operand name = op.operands.back();
			op.operands.pop_back();

			if (op.operands.size() == 1u) {
				op.func = arithmetic::Operation::IDENTITY;
			}
			args[0].setExpr(op);
			args.insert(args.begin(), name);

			return arithmetic::Expression(arithmetic::Operation::MEMBER_CALL, args);
		// Replace built-in functions
		} else if (args[0].top.isConst() and args[0].top.cnst.type == arithmetic::Value::STRING and args[0].top.cnst.sval == "valid") {
			if      (args.size() == 1u) { return arithmetic::Expression::vdd(); }
			else if (args.size() == 2u) { return arithmetic::isValid(args[1]); }
			else { error(__FILE__, __LINE__, nullptr, nullptr, "valid() function expects 1 argument, found {}", args.size()-1); }
		} else if (args[0].top.isConst() and args[0].top.cnst.type == arithmetic::Value::STRING and args[0].top.cnst.sval == "true") {
			if      (args.size() == 1u) { return arithmetic::Expression::boolOf(true); }
			else if (args.size() == 2u) { return arithmetic::isTrue(args[1]); }
			else { error(__FILE__, __LINE__, nullptr, nullptr, "true() function expects 1 argument, found {}", args.size()-1); }
		} else {
			return arithmetic::Expression(arithmetic::Operation::CALL, args);
		}
	// END DESIGN
	} else if (op.is("", "[", ":", "]")) {
		return arithmetic::Expression(arithmetic::Operation::INDEX, args);
	}
	internal("", "unrecognized operation", __FILE__, __LINE__);
	return arithmetic::Expression();
}

