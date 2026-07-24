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

parse_expression::expression::argument ExpressionExporter::export_action(const Action &act) const {
	internal("", "action export not defined", __FILE__, __LINE__);
	return {-1, nullptr};
}

parse_expression::expression ExpressionExporter::export_special(int func, const vector<parse_expression::expression::argument> &args) const {
	if (func >= 0 and func < (int)arithmetic::Operation::operators.size()) {
		internal("", "no export for operator '" + arithmetic::Operation::operators[func].to_string() + "'", __FILE__, __LINE__);
	} else {
		internal("", "operator " + ::to_string(func) + " not defined", __FILE__, __LINE__);
	}
	return parse_expression::expression();
}





// Functions that don't need to be overridden

parse_expression::expression::argument ExpressionExporter::export_operand(Operand arg) const {
	if (arg.isUndef()) {
		return {-1, nullptr};
	} else if (arg.isConst()) {
		if (arg.cnst.type == arithmetic::Value::ARRAY
			or arg.cnst.type == arithmetic::Value::STRUCT) {
			return {-1, std::shared_ptr<parse::syntax>(export_expression(arg.cnst.type, arg.cnst.arr).clone())};
		}
		return export_constant(arg.cnst);
	} else if (arg.isVar()) {
		return export_literal(arg.index);
	}
	internal("", "operand export not defined", __FILE__, __LINE__);
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

vector<parse_expression::expression::argument> ExpressionExporter::export_arguments(const vector<Operand> &args, const vector<parse_expression::expression> *sub) const {
	vector<parse_expression::expression::argument> result;
	for (const Operand &arg : args) {
		result.push_back(export_argument(arg, sub));
	}
	return result;
}

parse_expression::expression ExpressionExporter::export_expression(int func, vector<Operand> args, const vector<parse_expression::expression> *sub) const {
	const parse_expression::precedence_set &order = precedence();

	if (func == Operation::VALIDITY) {
		args.insert(args.begin(), Operand::termOf("valid"));
		func = Operation::CALL;
	} else if (func == Operation::TRUTHINESS) {
		args.insert(args.begin(), Operand::termOf("true"));
		func = Operation::CALL;
	} else if (func == Operation::NEGATIVE) {
		args.push_back(Operand::intOf(0));
		func = Operation::LESS;
	} else if (func == Operation::IDENTITY) {
		parse_expression::expression result;
		result.valid = true;

		result.level = -1;
		result.type = -1;

		result.arguments = export_arguments(args, sub);
		return result;
	} else if (func == Operation::INVERSE) {
		args.insert(args.begin(), Operand::realOf(1.0));
		func = Operation::DIVIDE;
	}

	auto op = export_operator(func);
	if (op.empty()) {
		return export_special(func, export_arguments(args, sub));
	}
	auto idx = order.find(-1, op);

	parse_expression::expression result;
	result.valid = true;

	result.level = idx.level;
	result.type = order.type(result.level);
	result.operators.push_back(op);

	result.arguments = export_arguments(args, sub);
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

parse_expression::assignment ExpressionExporter::export_expression(const Action &expr) const {
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
	auto idx = order.find(-1, op);
	result.level = idx.level;
	result.type = order.type(result.level);
	result.operators.push_back(op);

	for (const Action &act : expr.actions) {
		result.arguments.push_back(export_action(act));
	}
	return result;
}

parse_expression::expression ExpressionExporter::export_expression(const Choice &expr) const {
	const parse_expression::precedence_set &order = precedence();
	parse_expression::expression result;
	result.valid = true;

	auto op = export_operator(Operation::WIRE_OR);
	auto idx = order.find(-1, op);
	result.level = idx.level;
	result.type = order.type(result.level);
	result.operators.push_back(op);

	for (const Parallel &para : expr.terms) {
		result.arguments.push_back({-1, std::shared_ptr<parse::syntax>(export_expression(para).clone())});
	}
	return result;
}


}
