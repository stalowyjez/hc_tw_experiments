#include <vector>
#include <algorithm>
#include <set>
#include <functional>

#include "basic.h"

typedef std::function<bool(std::set<std::pair<int,int>>,std::set<std::pair<int,int>>)> HCCheckFunc;

namespace edge_selection {
	EdgeSet simpleSelection(Graph,EdgeSet,HCCheckFunc);
	EdgeSet syphilisSelection(Graph,EdgeSet,HCCheckFunc);
}
