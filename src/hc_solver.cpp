#include <vector>
#include <algorithm>
#include <set>
#include <string>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <functional>

#include "hc_solver.h"

using namespace std;


pair<bool,vector<int>> solve(vector<TWNode> & nodes, vector<vector<int>> & graph, set<pair<int,int>> & edges, int n) {
#if REDUCTION == 1
	auto solver = HCSolver(rgaussian1::reduction);
#elif REDUCTION == 2
	auto solver = HCSolver(rgaussian2::reduction);
#else
	auto solver = HCSolver(simple_reduction);
#endif

	return solver.getHC(nodes,graph,edges,n);
}

