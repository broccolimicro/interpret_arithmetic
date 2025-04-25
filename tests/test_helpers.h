#pragma once

#include <vector>
#include <string>
#include <utility>

using namespace std;

// Helper class to implement the Netlist interface for testing
struct VariableSet {
	vector<pair<string, int> > vars;

	int netIndex(string name, int region) const {
		for (int i = 0; i < (int)vars.size(); i++) {
			if (vars[i].first == name and vars[i].second == region) {
				return i;
			}
		}
		return -1;
	}

	int netIndex(string name, int region, bool define) {
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

	pair<string, int> netAt(int uid) const {
		return vars[uid];
	}

	int netCount() const {
		return (int)vars.size();
	}
};

// Non-defining variable set for testing undefined nets
struct NonDefiningVariableSet {
	vector<string> vars;

	int netIndex(string name, int region) const {
		for (int i = 0; i < (int)vars.size(); i++) {
			if (vars[i] == name) {
				return i;
			}
		}
		return -1;
	}

	int netIndex(string name, int region, bool define) {
		// Always return -1 for undefined variables, regardless of define flag
		for (int i = 0; i < (int)vars.size(); i++) {
			if (vars[i] == name) {
				return i;
			}
		}
		return -1;
	}

	pair<string, int> netAt(int uid) const {
		return {vars[uid], 0};
	}

	int netCount() const {
		return (int)vars.size();
	}
};
