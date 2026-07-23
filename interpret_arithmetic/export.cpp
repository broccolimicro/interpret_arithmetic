#include "export.h"
#include <arithmetic/value.h>

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

parse_expression::expression::argument ExpressionExporter::export_operand(Operand arg) const {
	internal("", "operand export not defined", __FILE__, __LINE__);
	return {-1, nullptr};
}

parse_expression::expression::argument ExpressionExporter::export_action(const Action &act) const {
	internal("", "action export not defined", __FILE__, __LINE__);
	return {-1, nullptr};
}

parse_expression::expression ExpressionExporter::export_expression(int type, const vector<Value> &arr) const {
	vector<Operand> args;
	for (const Value &v : arr) {
		args.push_back(Operand(v));
	}

	if (type == Value::ARRAY) {
		return export_expression(Operation::ARRAY, args);
	} else if (type == Value::STRUCT) {
		return export_expression(Operation::STRUCT, args);
	}
	return export_expression(Operation::IDENTITY, {Operand::undef()});
}

parse_expression::expression::argument ExpressionExporter::export_argument(Operand op, const vector<parse_expression::expression> *sub) const {
	if (op.isExpr() and sub != nullptr) {
		parse_expression::expression::argument result;
		result.type = -1;
		result.ptr = std::shared_ptr<parse::syntax>((*sub)[op.index].clone());
		return result;
	}
	return export_operand(op);
}

vector<parse_expression::expression::argument> ExpressionExporter::export_arguments(int func, const vector<Operand> &args, const vector<parse_expression::expression> *sub) const {
	vector<parse_expression::expression::argument> result;
	if (func == Operation::VALIDITY) {
		// valid(arg)

		result.push_back(export_operand(Operand::termOf("valid")));
		result.push_back(export_argument(args[0], sub));
	} else if (func == Operation::TRUTHINESS) {
		// true(arg)

		result.push_back(export_operand(Operand::termOf("true")));
		result.push_back(export_argument(args[0], sub));
	} else if (func == Operation::NEGATIVE) {
		// arg < 0

		result.push_back(export_argument(args[0], sub));
		result.push_back(export_operand(Operand::intOf(0)));
	} else if (func == Operation::IDENTITY) {
		// arg

		result.push_back(export_argument(args[0], sub));
	} else if (func == Operation::INVERSE) {
		// 1.0 / arg

		result.push_back(export_operand(Operand::realOf(1.0)));
		result.push_back(export_argument(args[0], sub));
	} else {
		for (const Operand &arg : args) {
			result.push_back(export_argument(arg, sub));
		}
	}
	return result;
}

parse_expression::expression ExpressionExporter::export_expression(int func, const vector<Operand> &args, const vector<parse_expression::expression> *sub) const {
	const parse_expression::precedence_set &order = precedence();
	parse_expression::expression result;
	result.valid = true;

	auto op = export_operator(func);
	result.level = op.first;
	result.type = order.type(op.first);
	if (op.second >= 0) {
		result.operators.push_back(order.at(op.first, op.second));
	}

	result.arguments = export_arguments(func, args, sub);
	return result;
}

parse_expression::expression ExpressionExporter::export_expression(const Expression &expr) const {
	if (not expr.top.isExpr()) {
		return export_expression(Operation::IDENTITY, {expr.top});
	}

	vector<parse_expression::expression> sub;
	for (arithmetic::ConstUpIterator i(expr, {expr.top}); not i.done(); ++i) {
		if (i->exprIndex >= sub.size()) {
			sub.resize(i->exprIndex+1);
		}
		sub[i->exprIndex] = export_expression(i->func, i->operands, &sub);
	}

	if (expr.top.index < sub.size()) {
		return sub[expr.top.index];
	}
	return export_expression(Operation::IDENTITY, {Operand::undef()});
}

parse_expression::assignment ExpressionExporter::export_assignment(const Action &expr) const {
	parse_expression::assignment result;
	result.valid = true;

	if (not expr.lvalue.isUndef()) {
		result.lvalue.push_back(export_expression(expr.lvalue));
	}

	// TODO(edward.bingham) we need type information about the lvalue here
	Operand top = expr.rvalue.top;
	if (top.isConst() and top.cnst.isNeutral()) {
		result.operation = "-";
	} else if (top.isConst() and top.cnst.isUnstable()) {
		result.operation = "~";
	} else if (top.isConst() and top.cnst.type == Value::WIRE and top.cnst.isValid()) {
		result.operation = "+";
	} else {
		result.rvalue = export_expression(expr.rvalue);
		result.operation = "=";
	}

	return result;
}

parse_expression::expression ExpressionExporter::export_expression(const Parallel &expr) const {
	const parse_expression::precedence_set &order = precedence();
	parse_expression::expression result;
	result.valid = true;

	auto op = export_operator(Operation::WIRE_AND);
	result.level = op.first;
	result.type = order.type(op.first);
	if (op.second >= 0) {
		result.operators.push_back(order.at(op.first, op.second));
	}

	for (const Action &act : expr.actions) {
		result.arguments.push_back(export_action(act));
	}
	return result;
}

parse_expression::expression ExpressionExporter::export_composition(const Choice &expr) const {
	const parse_expression::precedence_set &order = precedence();
	parse_expression::expression result;
	result.valid = true;

	auto op = export_operator(Operation::WIRE_OR);
	result.level = op.first;
	result.type = order.type(op.first);
	if (op.second >= 0) {
		result.operators.push_back(order.at(op.first, op.second));
	}

	for (const Parallel &para : expr.terms) {
		result.arguments.push_back({-1, std::shared_ptr<parse::syntax>(export_expression(para).clone())});
	}
	return result;
}


}
