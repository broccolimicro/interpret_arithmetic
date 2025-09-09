#include "export_verilog.h"

#include <arithmetic/algorithm.h>
#include <common/message.h>

namespace parse_verilog {

string export_value(const arithmetic::Value &v) {
	if (v.isNeutral()) {
		return "0";

	} else if (v.isValid()) {
		switch (v.type) {
			case arithmetic::Value::BOOL:
			case arithmetic::Value::WIRE:
				return "1";

			case arithmetic::Value::INT:
				return ::to_string(v.ival);

			case arithmetic::Value::REAL:
				return ::to_string(v.rval);

			case arithmetic::Value::STRING:
				return v.sval;

			default:
				internal("", "unrecognized valid value: " + std::to_string(v), __FILE__, __LINE__);
				return "";
		}
	} else if (v.isUnstable()) {
		return "X";

	} else {
		return "U";
	}
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
	switch (op) {
		case arithmetic::Operation::WIRE_NOT:
			return "~";
		case arithmetic::Operation::IDENTITY:
			return "+";
		case arithmetic::Operation::NEGATION:
			return "-";
		case arithmetic::Operation::VALIDITY:
			return "valid";
		case arithmetic::Operation::BOOLEAN_NOT:
			return "!";
		case arithmetic::Operation::INVERSE:
			return "1/";
		case arithmetic::Operation::WIRE_OR:
			return "|";
		case arithmetic::Operation::WIRE_AND:
			return "&";
		case arithmetic::Operation::WIRE_XOR:
			return "^";
		case arithmetic::Operation::EQUAL:
			return "==";
		case arithmetic::Operation::NOT_EQUAL:
			return "!=";
		case arithmetic::Operation::LESS:
			return "<";
		case arithmetic::Operation::GREATER:
			return ">";
		case arithmetic::Operation::LESS_EQUAL:
			return "<=";
		case arithmetic::Operation::GREATER_EQUAL:
			return ">=";
		case arithmetic::Operation::SHIFT_LEFT:
			return "<<";
		case arithmetic::Operation::SHIFT_RIGHT:
			return ">>";
		case arithmetic::Operation::ADD:
			return "+";
		case arithmetic::Operation::SUBTRACT:
			return "-";
		case arithmetic::Operation::MULTIPLY:
			return "*";
		case arithmetic::Operation::DIVIDE:
			return "/";
		case arithmetic::Operation::MOD:
			return "%";
		case arithmetic::Operation::BOOLEAN_OR:
			return "||";
		case arithmetic::Operation::BOOLEAN_AND:
			return "&&";
		case arithmetic::Operation::ARRAY:
			return ",";
		default:
			internal("", "unrecognized operation " + std::to_string(op), __FILE__, __LINE__);
			return "";
	}
}

argument export_argument(const vector<expression> &sub, arithmetic::Operand op, ucs::ConstNetlist nets) {
	argument result;
	if (op.isConst()) {
		result.constant = export_value(op.cnst);
	} else if (op.isVar()) {
		if (op.index < nets.netCount()) {
			result.literal = ucs::Net(nets.netAt(op.index));
		} else {
			internal("", "no net at index " + std::to_string(op.index), __FILE__, __LINE__);
		}
	} else if (op.isExpr()) {
		result.sub = sub[op.index];
	} else if (op.isType()) {
		internal("", "unable to export type expression: sub[0] -> " + sub[0].to_string(), __FILE__, __LINE__);
	} else {
		internal("", "unable to export undefined expression: sub[0] -> " + sub[0].to_string(), __FILE__, __LINE__);
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
