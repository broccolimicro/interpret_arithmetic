#pragma once

#include <common/interface.h>
#include <string>

using namespace std;

namespace arithmetic {

using Net = pair<string, int>;

_CONST_INTERFACE_ARG(ConstNetlist,
	(int, netIndex, (string name, int region) const),
	(Net, netAt, (int uid) const),
	(int, netCount, () const)
)

_INTERFACE_ARG(Netlist,
	(int, netIndex, (string name, int region, bool define)),
	(Net, netAt, (int uid) const),
	(int, netCount, () const)
)

}
