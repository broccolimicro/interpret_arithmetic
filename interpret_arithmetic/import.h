#pragma once

#include <common/standard.h>

#include <parse/tokenizer.h>

#include <parse_expression/expression.h>
#include <parse_expression/assignment.h>

namespace arithmetic {

template <typename T>
struct Importer {
	virtual T import_term(const parse_expression::expression::argument &syntax, tokenizer *tokens) const = 0;
	virtual void push_properties(parse_expression::operation op, const vector<parse_expression::expression::argument> &args, tokenizer *tokens) {
		// default behavior is NOP
	}

	virtual void pop_properties(parse_expression::operation op) {
		// default behavior is NOP
	}

	virtual T import_unary(parse_expression::operation op, T expr, tokenizer *tokens) const {
		if (tokens != nullptr) {
			tokens->internal("unary operations not supported by interpreter", __FILE__, __LINE__);
		} else {
			internal("", "unary operations not supported by interpreter", __FILE__, __LINE__);
		}
		return expr;
	}

	virtual T import_binary(parse_expression::operation op, T left, T right, tokenizer *tokens) const {
		if (tokens != nullptr) {
			tokens->internal("binary operations not supported by interpreter", __FILE__, __LINE__);
		} else {
			internal("", "binary operations not supported by interpreter", __FILE__, __LINE__);
		}
		return left;
	}

	virtual T import_ternary(parse_expression::operation op, vector<T> args, tokenizer *tokens) const {
		if (tokens != nullptr) {
			tokens->internal("binary operations not supported by interpreter", __FILE__, __LINE__);
		} else {
			internal("", "binary operations not supported by interpreter", __FILE__, __LINE__);
		}
		if (args.empty()) {
			return T();
		}
		return args[0];
	}

	virtual T import_group(parse_expression::operation op, vector<T> args, tokenizer *tokens) const {
		if (tokens != nullptr) {
			tokens->internal("group operations not supported by interpreter", __FILE__, __LINE__);
		} else {
			internal("", "group operations not supported by interpreter", __FILE__, __LINE__);
		}
		if (not args.empty()) {
			return args[0];
		}
		return T();
	}

	virtual T import_modifier(parse_expression::operation op, vector<T> args, tokenizer *tokens) const {
		if (tokens != nullptr) {
			tokens->internal("modifier operations not supported by interpreter", __FILE__, __LINE__);
		} else {
			internal("", "modifier operations not supported by interpreter", __FILE__, __LINE__);
		}
		if (not args.empty()) {
			return args[0];
		}
		return T();
	}

	// These probably don't need an overide
	virtual T import_argument(const parse_expression::expression::argument &syntax, tokenizer *tokens) {
		if (tokens != NULL) {
			tokens->load(syntax.ptr.get());
		}

		if (syntax.type >= 0) {
			return import_term(syntax, tokens);
		} else {
			return import_expression(syntax.ptr->get<parse_expression::expression>(), tokens);
		}
	}

	virtual vector<T> import_arguments(parse_expression::operation op, const vector<parse_expression::expression::argument> &syntax, tokenizer *tokens) {
		vector<T> result;
		for (size_t i = 0; i < syntax.size(); i++) {
			result.push_back(import_argument(syntax[i], tokens));
		}
		return result;
	}

	T import_expression(const parse_expression::expression &syntax, tokenizer *tokens) {
		if (tokens != NULL) {
			tokens->load(&syntax);
		}

		if (syntax.type < 0) {
			if (tokens != NULL) {
				tokens->internal("unrecognized operation", __FILE__, __LINE__);
			} else {
				internal("", "unrecognized operation", __FILE__, __LINE__);
			}
			return T();
		}

		if (syntax.operators.empty()) {
			if (syntax.arguments.size() == 1u) {
				return import_argument(syntax.arguments[0], tokens);
			} else {
				internal("", "malformed expression", __FILE__, __LINE__);
			}
		}

		T result;

		parse_expression::operation op = syntax.operators[0];
		push_properties(op, syntax.arguments, tokens);
		if (syntax.isGroup()) {
			result = import_group(op, import_arguments(op, syntax.arguments, tokens), tokens);
		} else if (syntax.isModifier()) {
			result = import_modifier(op, import_arguments(op, syntax.arguments, tokens), tokens);
		} else if (syntax.isTernary()) {
			result = import_ternary(op, import_arguments(op, syntax.arguments, tokens), tokens);
		} else if (syntax.isBinary() or syntax.isUnary()) {
			if (not syntax.arguments.empty()) {
				result = import_argument(syntax.arguments[0], tokens);
			}

			if (syntax.arguments.size() == 1u) {
				if (syntax.isUnary()) {
					for (int i = (int)syntax.operators.size()-1; i >= 0; i--) {
						result = import_unary(syntax.operators[i], result, tokens);
					}
				}
			} else {
				for (size_t i = 1; i < syntax.arguments.size(); i++) {
					T sub = import_argument(syntax.arguments[i], tokens);
					result = import_binary(syntax.operators[i-1], result, sub, tokens);
				}
			}
		}
		pop_properties(op);
		return result;
	}
};

}
