#include <algorithm>
#include <vector>
#include <memory>
#include <set>
#include <map>
#include <functional>
#include <string>

#include "treewidth.h"

#ifndef HC_SOLVER_COMMON

#define HC_SOLVER_COMMON

struct HCEdge {
	std::pair<int,int> edge;
	std::shared_ptr<HCEdge> previous;

	HCEdge();
	HCEdge(std::pair<int,int> const & edge, std::shared_ptr<HCEdge> pv);
};

#define PS_SIZE 30

struct PartialSolution {
	char match[PS_SIZE];
	std::shared_ptr<HCEdge> edges;

	PartialSolution(std::shared_ptr<HCEdge> edges);
	PartialSolution();
};

#define TWO_EDGES 63

int getPSVal(const char * match, int pos);
void setPSVal(char * match, int pos, int val);

bool isReallyFinal(const char * match, int pos, int v, int w);

namespace std {
	template<>
	struct hash<PartialSolution> {
		size_t operator() (PartialSolution const & ps) const; 
	};
}

bool operator==(PartialSolution const & A, PartialSolution const & B);
bool operator!=(PartialSolution const & A, PartialSolution const & B);
bool operator<(PartialSolution const & A, PartialSolution const & B);

//
// Solver class
//

typedef std::vector<PartialSolution> PartialSolutions;

class HCSolver {
private:
	std::shared_ptr<HCEdge> tail_edge;
	std::function<PartialSolutions(PartialSolutions,int)> reduction;
	PartialSolutions process_node(int root, std::vector<TWNode> const & nodes);
public:
	HCSolver(std::function<PartialSolutions(PartialSolutions,int)>);
	std::pair<bool,std::vector<int>> getHC(std::vector<TWNode> & nodes, std::vector<std::vector<int>> & graph, std::set<std::pair<int,int>> & edges, int n);
};

//
// Getting partial solutions divided into buckets
//

size_t getBucket(char const * match, int k);
std::unordered_map<size_t,PartialSolutions> getBuckets(PartialSolutions solutions, int k);
PartialSolutions simple_reduction(PartialSolutions result, size_t k);

namespace rgaussian1 {
	PartialSolutions reduction(PartialSolutions solutions, int k);
}

namespace rgaussian2 {
	PartialSolutions reduction(PartialSolutions solutions, int k);
}

#endif

