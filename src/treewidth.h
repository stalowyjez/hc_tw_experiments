#include <vector>
#include <algorithm>
#include <set>

#ifndef TWNODE

#define TWNODE

struct OrdinaryDecomposition {
	std::vector<std::vector<int>> vertices;
	std::vector<std::vector<int>> graph;

	OrdinaryDecomposition();
	OrdinaryDecomposition(std::vector<std::vector<int>>&&,std::vector<std::vector<int>>&&);
};

enum nodeType {
	LEAF,
	ADD_VERTEX,
	ADD_EDGE,
	DEL_VERTEX,
	JOIN,
	NONE
};

enum edgeMode {
	CHOSEN,
	AVAILABLE,
	DELETED
};

struct TWNode {
	int parent;
	std::vector<int> vertices;
	std::vector<int> children;
	std::set<std::pair<int,int>> edges;
	nodeType type;
	edgeMode mode;
	int v;
	int w;
	bool is_final;

	bool marked = false;

	TWNode(int,std::vector<int>,std::vector<int>);
	TWNode(int,std::vector<int>,std::vector<int>,nodeType);
	TWNode(int,std::vector<int>,std::vector<int>,nodeType,int);
	TWNode(int,std::vector<int>,std::vector<int>,std::set<std::pair<int,int>>,nodeType,int,int);
};

bool operator== (TWNode const & A, TWNode const & B);

OrdinaryDecomposition decompose(int n, std::vector<std::vector<int>> _Graph, std::set<std::pair<int,int>> _edges);
std::vector<TWNode> decomposeNicely(int n, std::vector<std::vector<int>> _Graph, std::set<std::pair<int,int>> _edges);
std::vector<TWNode> decompositionWithoutEdges(std::vector<TWNode>, std::set<std::pair<int,int>>);

std::vector<TWNode> makeDecompositionNice(OrdinaryDecomposition);

void setEdgeModes(std::vector<TWNode>&,std::set<std::pair<int,int>>const&,std::set<std::pair<int,int>>const&);

void setGraphAndEdges(std::vector<std::vector<int>> _Graph, std::set<std::pair<int,int>> _Edges);

#endif

