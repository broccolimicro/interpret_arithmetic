#pragma once

#include <vector>
#include <arithmetic/expression.h>
#include <arithmetic/action.h>
#include <parse_expression/import.h>
#include <parse_expression/expression.h>
#include <parse_expression/precedence.h>
#include <common/net.h>

namespace test {

struct ExpressionImporter : parse_expression::Importer<arithmetic::Expression> {
	ucs::Netlist symbols;
	vector<int> region;

	ExpressionImporter(ucs::Netlist symbols, int region = 0);
	~ExpressionImporter();

	arithmetic::Expression import_term(const parse_expression::expression::argument &syntax, tokenizer *tokens) const override;
	void push_properties(parse_expression::operation op, const vector<parse_expression::expression::argument> &args, tokenizer *tokens) override;
	void pop_properties(parse_expression::operation op) override;
	arithmetic::Expression import_unary(parse_expression::operation op, arithmetic::Expression expr, tokenizer *tokens) const override;
	arithmetic::Expression import_binary(parse_expression::operation op, arithmetic::Expression left, arithmetic::Expression right, tokenizer *tokens) const override;
	arithmetic::Expression import_group(parse_expression::operation op, vector<arithmetic::Expression> args, tokenizer *tokens) const override;
	arithmetic::Expression import_modifier(parse_expression::operation op, vector<arithmetic::Expression> args, tokenizer *tokens) const override;
};

arithmetic::Expression import_expression(const parse_expression::expression &syntax, ucs::Netlist nets, tokenizer *tokens, int region = 0);

struct CompositionImporter : parse_expression::Importer<arithmetic::Choice> {
	ucs::Netlist symbols;
	vector<int> region;

	CompositionImporter(ucs::Netlist symbols, int region = 0);
	~CompositionImporter();

	arithmetic::Choice import_term(const parse_expression::expression::argument &syntax, tokenizer *tokens) const override;
	void push_properties(parse_expression::operation op, const vector<parse_expression::expression::argument> &args, tokenizer *tokens) override;
	void pop_properties(parse_expression::operation op) override;
	arithmetic::Choice import_binary(parse_expression::operation op, arithmetic::Choice left, arithmetic::Choice right, tokenizer *tokens) const override;
	arithmetic::Choice import_modifier(parse_expression::operation op, vector<arithmetic::Choice> args, tokenizer *tokens) const override;
};

arithmetic::Choice import_composition(const parse_expression::expression &syntax, ucs::Netlist nets, tokenizer *tokens, int region = 0);

}
