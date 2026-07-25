#include "export_verilog.h"

#include <arithmetic/algorithm.h>
#include <common/message.h>
#include "export.h"

#include <parse_verilog/expression.h>
#include <parse/wrapper.h>

namespace parse_verilog {

string export_value(const arithmetic::Value &v) {
	if (v.type == arithmetic::Value::TYPE) {
		return v.sval;
	} else if (v.type == arithmetic::Value::TERM) {
		return v.sval;
	} else if (v.isUnstable()) {
		return "X";
	} else if (v.isUnknown()) {
		return "U";
	} else if (v.isNeutral()) {
		return "0";
	} else if (v.type == arithmetic::Value::WIRE) {
		return "1";
	} else if (v.type == arithmetic::Value::STRING) {
		return "\"" + v.sval + "\"";
	} else if (v.type == arithmetic::Value::BOOL) {
		return v.bval ? "1'b1" : "1'b0";
	} else if (v.type == arithmetic::Value::INT) {
		return ::to_string(v.ival);
	} else if (v.type == arithmetic::Value::REAL) {
		return ::to_string(v.rval);
	}
	internal("", "unrecognized value in export_value()", __FILE__, __LINE__);
	return "";
}

ExpressionExporter::ExpressionExporter(ucs::ConstNetlist nets) : nets(nets) {
}

ExpressionExporter::~ExpressionExporter() {
}

parse_expression::operation ExpressionExporter::export_operator(int func) const {
	using OpType = arithmetic::Operation::OpType;
	using operation = parse_expression::operation;

	switch (func) {
	// VALIDITY - converted to CALL
	case OpType::WIRE_NOT: return operation("~", "", "", "");
	case OpType::WIRE_OR:  return operation("", "", "|", "");
	case OpType::WIRE_AND: return operation("", "", "&", "");
	case OpType::WIRE_XOR: return operation("", "", "^", "");
	// TRUTHINESS - converted to CALL
	case OpType::BOOLEAN_NOT: return operation("!", "", "", "");
	case OpType::BOOLEAN_OR: return operation("", "", "||", "");
	case OpType::BOOLEAN_AND: return operation("", "", "&&", "");
	// BOOLEAN_XOR
	case OpType::EQUAL: return operation("", "", "==", "");
	case OpType::NOT_EQUAL: return operation("", "", "!=", "");
	case OpType::LESS: return operation("", "", "<", "");
	case OpType::GREATER: return operation("", "", ">", "");
	case OpType::LESS_EQUAL: return operation("", "", "<=", "");
	case OpType::GREATER_EQUAL: return operation("", "", ">=", "");
	// NEGATIVE - converted to LESS
	case OpType::TERNARY: return operation("", "?", ":", "");
	case OpType::IDENTITY: return operation("+", "", "", "");
	case OpType::NEGATION: return operation("-", "", "", "");
	// INVERSE - converted to DIVIDE
	// TODO(edward.bingham) we need type information here to determine if we are using arithmetic or logical shift
	case OpType::SHIFT_LEFT: return operation("", "", "<<", "");
	case OpType::SHIFT_RIGHT: return operation("", "", ">>", "");
	case OpType::ADD: return operation("", "", "+", "");
	case OpType::SUBTRACT: return operation("", "", "-", "");
	case OpType::MULTIPLY: return operation("", "", "*", "");
	case OpType::DIVIDE: return operation("", "", "/", "");
	case OpType::MOD: return operation("", "", "%", "");
	case OpType::CALL: return operation("$", "(", ",", ")");
	// MEMBER_CALL
	case OpType::CAST: return operation("", "'(", "", ")");
	case OpType::ARRAY: return operation("'{", "", "", "}");
	case OpType::INDEX: return operation("", "[", ":", "]");
	case OpType::STRUCT: return operation("'{", "", "", "}");
	case OpType::MEMBER: return operation("", ".", "", "");
	}
	return operation();
}

const parse_expression::precedence_set &ExpressionExporter::precedence() const {
	return parse_verilog::config::cfg->order;
}

parse_expression::expression::argument ExpressionExporter::export_constant(arithmetic::Value value) const {
	parse::wrapper<number> result;
	result.value = parse_verilog::export_value(value);
	return {0, std::shared_ptr<parse::syntax>(result.clone())};
}

parse_expression::expression::argument ExpressionExporter::export_literal(size_t index) const {
	parse::wrapper<parse::instance> result;
	result.value = nets.netAt(index);
	return {1, std::shared_ptr<parse::syntax>(result.clone())};
}

parse_expression::expression ExpressionExporter::export_special(int func, const vector<parse_expression::expression::argument> &args) const {
	using OpType = arithmetic::Operation::OpType;
	if (func == OpType::BOOLEAN_XOR) {
		return export_boolean_xor(args);
	}

	return arithmetic::ExpressionExporter::export_special(func, args);
}

parse_expression::expression ExpressionExporter::export_boolean_xor(const vector<parse_expression::expression::argument> &args) const {
	using OpType = arithmetic::Operation::OpType;
	const parse_expression::precedence_set &order = precedence();

	if (args.size() < 2u) {
		parse_expression::expression result;
		result.valid = true;

		result.level = -1;
		result.type = -1;

		result.arguments = args;
		return result;
	}

	auto orOp = export_operator(OpType::BOOLEAN_OR);
	auto andOp = export_operator(OpType::BOOLEAN_AND);
	auto notOp = export_operator(OpType::BOOLEAN_NOT);
	if (orOp.empty() or andOp.empty() or notOp.empty()) {
		internal("", "boolean operators not defined for verilog", __FILE__, __LINE__);
		return parse_expression::expression();
	}

	auto orIdx = order.find(-1, orOp);
	auto andIdx = order.find(-1, andOp);
	auto notIdx = order.find(-1, notOp);

	parse_expression::expression notA;
	notA.valid = true;
	notA.level = notIdx.level;
	notA.type = order.type(notA.level);
	notA.operators.push_back(notOp);
	notA.arguments.push_back(args[0]);

	parse_expression::expression notB;
	notB.valid = true;
	notB.level = notIdx.level;
	notB.type = order.type(notB.level);
	notB.operators.push_back(notOp);
	notB.arguments.push_back(args[1]);

	parse_expression::expression left;
	left.valid = true;
	left.level = andIdx.level;
	left.type = order.type(left.level);
	left.operators.push_back(andOp);
	left.arguments.push_back(args[0]);
	left.arguments.push_back({-1, std::shared_ptr<parse::syntax>(notB.clone())});

	parse_expression::expression right;
	right.valid = true;
	right.level = andIdx.level;
	right.type = order.type(right.level);
	right.operators.push_back(andOp);
	right.arguments.push_back({-1, std::shared_ptr<parse::syntax>(notA.clone())});
	right.arguments.push_back(args[1]);

	parse_expression::expression top;
	top.valid = true;
	top.level = orIdx.level;
	top.type = order.type(top.level);
	top.operators.push_back(orOp);
	top.arguments.push_back({-1, std::shared_ptr<parse::syntax>(left.clone())});
	top.arguments.push_back({-1, std::shared_ptr<parse::syntax>(right.clone())});
	
	if (args.size() > 2u) {
		vector<parse_expression::expression::argument> nextArgs;
		nextArgs.push_back({-1, std::shared_ptr<parse::syntax>(top.clone())});
		nextArgs.insert(nextArgs.end(), args.begin() + 2, args.end());
		return export_boolean_xor(nextArgs);
	}

	return top;
}

/*parse_expression::expression ExpressionExporter::export_member_call(const vector<parse_expression::expression::argument> &args) const {
	using OpType = arithmetic::Operation::OpType;
	const parse_expression::precedence_set &order = precedence();

	if (args.size() < 2u) {
		parse_expression::expression result;
		result.valid = true;

		result.level = -1;
		result.type = -1;

		result.arguments = export_arguments(args, sub);
		return result;
	}

	auto callOp = export_operator(OpType::CALL);
	auto memberOp = export_operator(OpType::MEMBER);
	if (callOp.empty() or memberOp.empty()) {
		internal("", "call and member operators not defined for verilog", __FILE__, __LINE__);
		return parse_expression::expression();
	}

	auto callIdx = order.find(-1, callOp);
	auto memberIdx = order.find(-1, memberOp);

	parse_expression::expression member;
	member.valid = true;
	member.level = memberIdx.level;
	member.type = order.type(member.level);
	member.operators.push_back(memberOp);
	member.arguments.push_back(args[0]);
	member.arguments.push_back(args[1]);

	parse_expression::expression top;
	top.valid = true;
	top.level = callIdx.level;
	top.type = order.type(top.level);
	top.operators.push_back(callOp);
	top.arguments.push_back({-1, std::shared_ptr<parse::syntax>(member.clone())});
	top.arguments.insert(args.begin()+2, args.end());
	
	return top;
}*/

parse_expression::expression export_expression(const arithmetic::Expression &expr, ucs::ConstNetlist nets) {
	return ExpressionExporter(nets).export_expression(expr);
}

}
