#pragma once

#include <common/interface.h>
#include <string>

using namespace std;

namespace arithmetic {

using Net = pair<string, int>;

CONST_INTERFACE(ConstNetlist,
	(int, netIndex, (string name, int region) const),
	(Net, netAt, (int uid) const),
	(int, netCount, () const)
)

INTERFACE(Netlist,
	(int, netIndex, (string name, int region, bool define)),
	(Net, netAt, (int uid) const),
	(int, netCount, () const)
)

}
