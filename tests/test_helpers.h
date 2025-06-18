#pragma once

#include <vector>
#include <string>
#include <utility>
#include <parse_expression/precedence.h>

using namespace std;
using namespace parse_expression;

struct VariableSet {
	vector<pair<string, int> > vars;

	int netIndex(string name) const;
	int netIndex(string name, bool define);
	string netAt(int uid) const;
	int netCount() const;
};

precedence_set createPrecedence();

