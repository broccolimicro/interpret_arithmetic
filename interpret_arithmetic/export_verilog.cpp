#include "export_verilog.h"
#include <common/message.h>
#include <arithmetic/algorithm.h>

namespace parse_verilog {

string export_value(const arithmetic::Value &v) {
	if (v.type == arithmetic::Value::BOOL) {
		if (v.isNeutral()) {
			return "0";
		} else if (v.isValid()) {
			return "1";
		} else if (v.isUnstable()) {
			return "X";
		} else {
			return "U";
		}
	} else if (v.type == arithmetic::Value::INT) {
		return ::to_string(v.ival);
	} else if (v.type == arithmetic::Value::REAL) {
		return ::to_string(v.rval);
	}
	internal("", "unrecognized value in export_value()", __FILE__, __LINE__);
	return "";
}

expression export_expression(const arithmetic::Value &v) {
	expression result;
	result.valid = true;
	result.level = expression::get_level("");
	result.arguments.push_back(argument(export_value(v)));
	return result;	
}

expression export_expression(const arithmetic::State &s, ucs::ConstNetlist nets)
{
	vector<expression> result;

	for (int i = 0; i < (int)s.values.size(); i++)
	{
		if (not s.values[i].isUnknown()) {
			expression add;
			add.valid = true;
			if (s.values[i].isNeutral()) {
				add.operations.push_back("~");
				add.level = expression::get_level(add.operations[0]);
				add.arguments.resize(1);
				add.arguments[0].literal = ucs::Net(nets.netAt(i));
			} else if (s.values[i].isValid()) {
				add.operations.push_back("");
				add.level = expression::get_level(add.operations[0]);
				add.arguments.resize(1);
				add.arguments[0].literal = ucs::Net(nets.netAt(i));
			} else {
				add.operations.push_back("==");
				add.level = expression::get_level(add.operations[0]);
				add.arguments.resize(2);
				add.arguments[0].literal = ucs::Net(nets.netAt(i));
				add.arguments[1].constant = export_value(s.values[i]);
			}

			result.push_back(add);
		}
	}

	if (result.size() == 1) {
		return result.back();
	}

	expression add;
	add.valid = true;

	if (result.size() > 1) {
		add.operations.push_back("&");
		add.level = expression::get_level(add.operations[0]);
		add.arguments.resize(result.size());
		for (int i = 0; i < (int)result.size(); i++) {
			add.arguments[i].sub = result[i];
		}
	} else {
		add.arguments.push_back(argument("1"));
	}

	return add;
}

string export_operation(int op) {
	if (op == arithmetic::Operation::BITWISE_NOT) {
		return "~";
	} else if (op == arithmetic::Operation::IDENTITY) {
		return "+";
	} else if (op == arithmetic::Operation::NEGATION) {
		return "-";
	} else if (op == arithmetic::Operation::VALIDITY) {
		return "valid";
	} else if (op == arithmetic::Operation::BOOLEAN_NOT) {
		return "!";
	} else if (op == arithmetic::Operation::INVERSE) {
		return "1/";
	} else if (op == arithmetic::Operation::BITWISE_OR) {
		return "|";
	} else if (op == arithmetic::Operation::BITWISE_AND) {
		return "&";
	} else if (op == arithmetic::Operation::BITWISE_XOR) {
		return "^";
	} else if (op == arithmetic::Operation::EQUAL) {
		return "==";
	} else if (op == arithmetic::Operation::NOT_EQUAL) {
		return "!=";
	} else if (op == arithmetic::Operation::LESS) {
		return "<";
	} else if (op == arithmetic::Operation::GREATER) {
		return ">";
	} else if (op == arithmetic::Operation::LESS_EQUAL) {
		return "<=";
	} else if (op == arithmetic::Operation::GREATER_EQUAL) {
		return ">=";
	} else if (op == arithmetic::Operation::SHIFT_LEFT) {
		return "<<";
	} else if (op == arithmetic::Operation::SHIFT_RIGHT) {
		return ">>";
	} else if (op == arithmetic::Operation::ADD) {
		return "+";
	} else if (op == arithmetic::Operation::SUBTRACT) {
		return "-";
	} else if (op == arithmetic::Operation::MULTIPLY) {
		return "*";
	} else if (op == arithmetic::Operation::DIVIDE) {
		return "/";
	} else if (op == arithmetic::Operation::MOD) {
		return "%";
	} else if (op == arithmetic::Operation::BOOLEAN_OR) {
		return "||";
	} else if (op == arithmetic::Operation::BOOLEAN_AND) {
		return "&&";
	} else if (op == arithmetic::Operation::ARRAY) {
		return ",";
	}
	return "";
}

argument export_argument(const vector<expression> &sub, arithmetic::Operand op, ucs::ConstNetlist nets) {
	argument result;
	if (op.isConst()) {
		result.constant = export_value(op.cnst);
	} else if (op.isVar()) {
		if (op.index < nets.netCount()) {
			result.literal = ucs::Net(nets.netAt(op.index));
		} else {
			internal("", "no net at index " + ::to_string(op.index), __FILE__, __LINE__);
		}
	} else if (op.isExpr()) {
		result.sub = sub[op.index];
	} else if (op.isType()) {
		internal("", "unable to export type expression", __FILE__, __LINE__);
	} else {
		internal("", "unable to export undefined expression", __FILE__, __LINE__);
	}
	return result;
}

expression export_expression(const arithmetic::Expression &expr, ucs::ConstNetlist nets)
{
	vector<expression> result;

	for (arithmetic::ConstUpIterator i(expr, {expr.top}); not i.done(); ++i) {
		expression add;
		add.valid = true;
		add.operations.push_back(export_operation(i->func));
		add.level = expression::get_level(add.operations[0]);
		add.arguments.resize(i->operands.size());
		for (int j = 0; j < (int)i->operands.size(); j++) {
			add.arguments[j] = export_argument(result, i->operands[j], nets);
			if (i->isCommutative() and j >= 2) {
				add.operations.push_back(add.operations.back());
			}
		}

		if (i->exprIndex >= result.size()) {
			result.resize(i->exprIndex+1);
		}
		result[i->exprIndex] = add;
	}

	if (expr.top.isExpr()) {
		if (expr.top.index < result.size()) {
			return result[expr.top.index];
		} else {
			expression add;
			add.valid = true;
			add.arguments.push_back(argument("0"));
			return add;
		}
	}

	expression add;
	add.valid = true;
	add.operations.push_back("");
	add.level = expression::get_level(add.operations[0]);
	add.arguments.push_back(export_argument(result, expr.top, nets));
	return add;
}

}
