#pragma once

#include <vector>
#include <string>
#include <utility>

using namespace std;

// Helper class to implement the Netlist interface for testing
struct VariableSet {
	vector<pair<string, int> > vars;

	int netIndex(string name) const {
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

	int netIndex(string name, bool define) {
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

	string netAt(int uid) const {
		return vars[uid].first + (vars[uid].second != 0 ?
			"'"+::to_string(vars[uid].second) : "");
	}

	int netCount() const {
		return (int)vars.size();
	}
};

