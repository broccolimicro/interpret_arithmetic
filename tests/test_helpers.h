#pragma once

#include <vector>
#include <string>
#include <utility>

using namespace std;

// Helper class to implement the Netlist interface for testing
struct VariableSet {
	vector<ucs::Net> vars;

	int netIndex(ucs::Net name) const {
		for (int i = 0; i < (int)vars.size(); i++) {
			if (vars[i] == name) {
				return i;
			}
		}
		return -1;
	}

	int netIndex(ucs::Net name, bool define) {
		bool found = false;
		for (int i = 0; i < (int)vars.size(); i++) {
			if (vars[i].fields == name.fields) {
				found = true;
				if (vars[i].region == name.region) {
					return i;
				}
			}
		}

		if (found or define) {
			vars.push_back(name);
			return (int)vars.size()-1;
		}
		return -1;
	}

	ucs::Net netAt(int uid) const {
		return vars[uid];
	}

	int netCount() const {
		return (int)vars.size();
	}
};

