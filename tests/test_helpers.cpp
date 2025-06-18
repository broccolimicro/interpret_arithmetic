#include "test_helpers.h"

int VariableSet::netIndex(string name) const {
	int region = 0;
	size_t tic = name.rfind('\'');
	if (tic != string::npos) {
		region = std::stoi(name.substr(tic+1));
		name = name.substr(0, tic);
	}

	for (int i = 0; i < (int)vars.size(); i++) {
		if (vars[i].first == name and vars[i].second == region) {
			return i;
		}
	}
	return -1;
}

int VariableSet::netIndex(string name, bool define) {
	int region = 0;
	size_t tic = name.rfind('\'');
	if (tic != string::npos) {
		region = std::stoi(name.substr(tic+1));
		name = name.substr(0, tic);
	}

	bool found = false;
	for (int i = 0; i < (int)vars.size(); i++) {
		if (vars[i].first == name) {
			found = true;
			if (vars[i].second == region) {
				return i;
			}
		}
	}

	if (found or define) {
		vars.push_back({name, region});
		return (int)vars.size()-1;
	}
	return -1;
}

string VariableSet::netAt(int uid) const {
	return vars[uid].first + (vars[uid].second != 0 ?
		"'"+::to_string(vars[uid].second) : "");
}

int VariableSet::netCount() const {
	return (int)vars.size();
}

precedence_set createPrecedence() {
	precedence_set result;
	result.push(operation_set::GROUP);
	result.push_back("[", "", ",", "]");

	result.push(operation_set::TERNARY);
	result.push_back("", "?", ":", "");

	result.push(operation_set::BINARY);
	result.push_back("", "", "|", "");

	result.push(operation_set::BINARY);
	result.push_back("", "", "&", "");

	result.push(operation_set::BINARY);
	result.push_back("", "", "^", "");

	result.push(operation_set::BINARY);
	result.push_back("", "", "==", "");
	result.push_back("", "", "~=", "");
	result.push_back("", "", "<", "");
	result.push_back("", "", ">", "");
	result.push_back("", "", "<=", "");
	result.push_back("", "", ">=", "");

	result.push(operation_set::BINARY);
	result.push_back("", "", "||", "");
	
	result.push(operation_set::BINARY);
	result.push_back("", "", "&&", "");

	result.push(operation_set::BINARY);
	result.push_back("", "", "^^", "");

	result.push(operation_set::BINARY);
	result.push_back("", "", "<<", "");
	result.push_back("", "", ">>", "");

	result.push(operation_set::BINARY);
	result.push_back("", "", "+", "");
	result.push_back("", "", "-", "");

	result.push(operation_set::BINARY);
	result.push_back("", "", "*", "");
	result.push_back("", "", "/", "");
	result.push_back("", "", "%", "");

	result.push(operation_set::UNARY);
	result.push_back("!", "", "", "");
	result.push_back("~", "", "", "");
	result.push_back("(bool)", "", "", "");
	result.push_back("+", "", "", "");
	result.push_back("-", "", "", "");
	result.push_back("?", "", "", "");

	result.push(operation_set::MODIFIER);
	result.push_back("", "'", "", "");

	result.push(operation_set::MODIFIER);
	result.push_back("", "(", ",", ")");
	result.push_back("", ".", "", "");
	result.push_back("", "[", ":", "]");

	result.push(operation_set::MODIFIER);
	result.push_back("", "::", "", "");
	return result;
}

