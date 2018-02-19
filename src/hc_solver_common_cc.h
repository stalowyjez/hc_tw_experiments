#include <algorithm>
#include <vector>
#include <memory>
#include <set>
#include <map>
#include <functional>

#include "treewidth.h"

#ifndef CC_SOLVER

#define CC_SOLVER

struct CCPartialSolution {
	unsigned long long types;
	unsigned long long poly;

	CCPartialSolution();
	CCPartialSolution(unsigned long long types, unsigned long long poly);
};


#define NN 0l
#define LF 1l
#define RT 3l
#define BT 2l

typedef unsigned long long Type;

Type getType(unsigned long long const & types, int pos);
void setType(unsigned long long & types, int pos, Type type);

bool operator<(CCPartialSolution const & A, CCPartialSolution const & B);
bool operator==(CCPartialSolution const & A, CCPartialSolution const & B);

//
// Solver class
//

typedef std::vector<CCPartialSolution> CCPartialSolutions;

class HCSolver {
private:
	std::function<CCPartialSolutions(CCPartialSolutions,int)> reduction;
	bool hasHC(std::vector<TWNode> const & nodes);
public:
	CCPartialSolutions process_node(int root, std::vector<TWNode> const & nodes);
	HCSolver(std::function<CCPartialSolutions(CCPartialSolutions,int)>);
	std::pair<bool,std::vector<int>> getHC(std::vector<TWNode> & nodes, std::vector<std::vector<int>> & graph, std::set<std::pair<int,int>> & edges, int n);
};

CCPartialSolutions simple_reduction(CCPartialSolutions && result, size_t k);

#endif

