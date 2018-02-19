#include <cstdio>
#include <vector>
#include <set>
#include <algorithm>
#include <queue>
#include <map>
#include <cassert>
#include "treewidth.h"
#include "basic.h"
#include "strange_io.h"

//#define DEBUG

using namespace std;

static vector<vector<int>> G;
static vector<vector<int>> LN;

static bool deleted[V];

static set<pair<int,int>> edges;
static map<pair<int,int>,bool> isOriginal;

//
// Ordinary decomposition constructors
//

OrdinaryDecomposition::OrdinaryDecomposition() {}
OrdinaryDecomposition::OrdinaryDecomposition(vector<vector<int>> && _vertices, vector<vector<int>> && _graph) :
	vertices(_vertices),
	graph(_graph)
{}

//
// Constructors
//

TWNode::TWNode(int _parent, vector<int> _children, vector<int> _vertices) :
	parent(_parent),
	vertices(_vertices),
	children(_children),
	type(NONE),
	mode(AVAILABLE)
{}

TWNode::TWNode(int _parent, vector<int> _vertices, vector<int> _children, nodeType _type) :
	parent(_parent),
	vertices(_vertices),
	children(_children),
	type(_type),
	mode(AVAILABLE),
	is_final(false)
{}

TWNode::TWNode(int _parent, vector<int> _vertices, vector<int> _children, nodeType _type, int _v) :
	parent(_parent),
	vertices(_vertices),
	children(_children),
	type(_type),
	mode(AVAILABLE),
	v(_v),
	is_final(false)
{}

TWNode::TWNode(int _parent, vector<int> _vertices, vector<int> _children, set<pair<int,int>> _edges, nodeType _type, int _v, int _w) :
	parent(_parent),
	vertices(_vertices),
	children(_children),
	edges(_edges),
	type(_type),
	mode(AVAILABLE),
	v(_v),
	w(_w),
	is_final(false)
{}

//
// Getting number of element in vector
//

int getNum(vector<int> vec, int element) {
	for (int i = 0; i < ((int) vec.size()); i++)
		if (vec[i] == element)
			return i;
	return -1;
}

//
// Sort vertices
//

void sortVertices(vector<TWNode> & nodes) {
	for (auto & node : nodes) {
		sort(node.vertices.begin(), node.vertices.end());
		if (node.type == ADD_VERTEX)
			node.v = getNum(node.vertices, node.v);
		if (node.type == DEL_VERTEX)
			node.v = getNum(nodes[node.children[0]].vertices, node.v);
	}
}

//
// Adding leaves
//

void createLeaf(int parent, vector<TWNode> & nodes) {
	nodes[parent].children.push_back(nodes.size());
	nodes.push_back(TWNode(parent,vector<int>(),vector<int>(),LEAF));
}

void createLeaves(vector<TWNode> & nodes) {
	int initial_size = (int) nodes.size();
	for (int i = 0; i < initial_size; i++)
		if (nodes[i].children.size() == 0)
			createLeaf(i, nodes);
}

//
// Adding add/remove nodes
//

void createAddNode(int child, vector<TWNode> & nodes, int v) {
	int p = nodes[child].parent;

	for (int & chd : nodes[p].children)
		if (chd == child)
			chd = nodes.size();
	nodes[child].parent = nodes.size();

	vector<int> new_vertices(nodes[child].vertices);
	new_vertices.push_back(v);

	nodes.push_back(TWNode(p,new_vertices,vector<int>({child}),ADD_VERTEX,v));
}

void createDelNode(int child, vector<TWNode> & nodes, int v) {
	int p = nodes[child].parent;

	for (int & chd : nodes[p].children)
		if (chd == child)
			chd = nodes.size();
	nodes[child].parent = nodes.size();

	auto new_vertices = withoutElement(nodes[child].vertices, v);

	nodes.push_back(TWNode(p,new_vertices,vector<int>({child}),DEL_VERTEX,v));
}

void createAddAndRemoveNodes(vector<TWNode> & nodes) {
	for (int i = 1; i < ((int)nodes.size()); i++) 
		if (not isDiffEmpty(nodes[i].vertices, nodes[nodes[i].parent].vertices)) {
			int v = firstInDiff(nodes[i].vertices, nodes[nodes[i].parent].vertices);
			createDelNode(i, nodes, v);
		}

	for (int i = 1; i < ((int)nodes.size()); i++) 
		if (not isDiffEmpty(nodes[nodes[i].parent].vertices, nodes[i].vertices)) {
			int v = firstInDiff(nodes[nodes[i].parent].vertices, nodes[i].vertices);
			createAddNode(i, nodes, v);
		}
}

//
// Getting nice joins
//

void rebuildJoinNode(int node, vector<TWNode> & nodes) {
	nodes[node].type = JOIN;

	auto rest_of_children = nodes[node].children;
	auto vertices = nodes[node].vertices;

	int last = node;

	while (rest_of_children.size() > 2) {
		int current_child = rest_of_children.back();
		rest_of_children.pop_back();

		nodes[last].children = vector<int>({current_child,(int)nodes.size()});
		nodes[current_child].parent = last;

		nodes.push_back(TWNode(last,vertices,vector<int>(),JOIN));
		last = nodes.size()-1;
	}

	nodes[last].children = rest_of_children;
	for (int child : rest_of_children)
		nodes[child].parent = last;
}

void rebuildJoinNodes(vector<TWNode> & nodes) {
	int initial_size = nodes.size();
	for (int i = 1; i < initial_size; i++)
		if (nodes[i].children.size() > 1)
			rebuildJoinNode(i, nodes);
}

//
// Adding "addEdge" nodes
//

void createAddEdgeNode(vector<TWNode> & nodes, int child, pair<int,int> e) {
	int p = nodes[child].parent;

	for (int & chd : nodes[p].children)
		if (chd == child)
			chd = nodes.size();
	nodes[child].parent = nodes.size();

	//auto new_edges = nodes[child].edges;
	//new_edges.insert(e);

	int v = getNum(nodes[child].vertices,e.first);
	int w = getNum(nodes[child].vertices,e.second);

	nodes.push_back(TWNode(p,nodes[child].vertices,vector<int>({child}),set<pair<int,int>>(),ADD_EDGE,v,w));
}

void edgeClearing(vector<TWNode> & nodes, int root, set<pair<int,int>> const & edges_to_clear) {
	auto new_etc = edges_to_clear;
	auto old_edges = nodes[root].edges;

	for (auto e : old_edges)
		if (edges_to_clear.count(e) != 0)
			nodes[root].edges.erase(e);
		else
			new_etc.insert(e);

	for (auto child : nodes[root].children)
		edgeClearing(nodes, child, new_etc);
}

void createAddEdgeNodes(vector<TWNode> & nodes) {
	for (auto & node : nodes) {
		for (int v : node.vertices) 
			for (int w : node.vertices)
				if (v < w and edges.count(edge(v,w)) != 0 and isOriginal[edge(v,w)])
					node.edges.insert(edge(v,w));
	}

	edgeClearing(nodes,0,set<pair<int,int>>());
	
	for (int i = 1; i < ((int)nodes.size()); i++) {
		auto current_edges = set<pair<int,int>>();
		swap(nodes[i].edges,current_edges);

		for (auto edge : current_edges)
			createAddEdgeNode(nodes,i,edge);
	}
}

//
// Cleaning NONEs
//

void removeNoneNodes(vector<TWNode> & nodes) {

	// PHASE 1
	
	for (int i = 0; i < ((int)nodes.size()); i++) {
		while (nodes[i].parent != -1 and nodes[nodes[i].parent].type == NONE and nodes[nodes[i].parent].parent != nodes[i].parent)
			nodes[i].parent = nodes[nodes[i].parent].parent;
		for (int & child : nodes[i].children)
			while (nodes[child].type == NONE)
				child = nodes[child].children[0];
	}

#ifdef DEBUG
	printf("Not bad after first phase.\n");
#endif

	// PHASE 2
	
	vector<int> new_nums(nodes.size());
	vector<TWNode> result;

	int current_num = 0;

	for (int i = 0; i < ((int)nodes.size()); i++)
		if (nodes[i].type != NONE)
			new_nums[i] = current_num++;

#ifdef DEBUG
	for (int i = 0; i < new_nums.size(); i++)
		printf("new_num[%d] = %d\n", i, new_nums[i]);
#endif

	for (int i = 0; i < ((int)nodes.size()); i++) if (nodes[i].type != NONE) {
		if (nodes[i].parent != -1)
			nodes[i].parent = new_nums[nodes[i].parent];
		for (int & child : nodes[i].children)
			child = new_nums[child];
		result.push_back(nodes[i]);
	}

	nodes = result;
}

//
// Adding edges
//


void addEdge(int A, int B, bool is_original) {
	G[A].push_back(B);
	G[B].push_back(A);

	edges.insert(edge(A,B));
	isOriginal[edge(A,B)] = is_original;
}

//
// Getting order
//

vector<int> vertexNums;

int computeMissingedges(int v) {
	int result = 0;
	for (int w : G[v]) if (not deleted[w])
		for (int u : G[v]) if (u < w and not deleted[u])
			if (not edges.count(edge(u,w)))
				result++;
	return result;
}

int findMinVertex(int n) {
	int best = 0;
	int best_val = INF;

	for (int v = 1; v <= n; v++) if (not deleted[v]) {
		int v_val = vertexNums[v]; 

		if (v_val < best_val) {
			best = v;
			best_val = v_val;
		}
	}

	return best;
}

vector<bool> mark;

void deleteVertex(int v) {
  int counter = 0;
	deleted[v] = true;
	for (int w : G[v]) if (not deleted[w])
		for (int u : G[v]) if (not deleted[u] && u != w)
			if (not edges.count(edge(u,w))) {
				addEdge(u,w,false);
        counter ++;

				// find common vertices
				
				for (int s : G[u]) if (not deleted[s])
					mark[s] = true;
				for (int s : G[w]) if (mark[s])
					vertexNums[s]--;
				for (int s : G[u]) if (not deleted[s])
					mark[s] = false;

				// find uncommon vertices

				for (int s : G[u]) if (not deleted[s])
					mark[s] = true;
				for (int s : G[w]) if ((not deleted[s]) && (not mark[s]) && (u != s))
					vertexNums[w]++;
				for (int s : G[u]) if (not deleted[s])
					mark[s] = false;

				// find uncommon vertices

				for (int s : G[w]) if (not deleted[s])
					mark[s] = true;
				for (int s : G[u]) if ((not deleted[s]) && (not mark[s]) && (w != s))
					vertexNums[u]++;
				for (int s : G[w]) if (not deleted[s])
					mark[s] = false;
			}

	for (int w : G[v]) if (not deleted[w])
		mark[w] = true;

	for (int w : G[v]) if (not deleted[w])
		for (int u : G[w]) if ((not deleted[u]) && (not mark[u]))
			vertexNums[w]--;

	for (int w : G[v]) if (not deleted[w])
		mark[w] = false;

  assert(counter == vertexNums[v]);
}

//
// Order to decomposition
//

void graphToTree(OrdinaryDecomposition & decomposition, int v, int parent, vector<int> & num, vector<TWNode> & result) {
	num[v] = result.size();

	result.push_back(TWNode(num[parent], vector<int>(), decomposition.vertices[v]));

	for (int w : decomposition.graph[v])
		if (w != parent) {
			graphToTree(decomposition,w,v,num,result);
			result[num[v]].children.push_back(num[w]);
		}
}

vector<TWNode> graphToTree(OrdinaryDecomposition decomposition) {
	int root = 1;
	for (int node = 1; node < (int)decomposition.graph.size(); node++) {
		if (decomposition.graph[node].size() == 1)
			root = node;
	}

	vector<TWNode> result({TWNode(-1,vector<int>({1}),vector<int>())});
	std::vector<int> num(decomposition.graph.size());
	num[0] = 0;
	graphToTree(decomposition,root,0,num,result);

	return result;
}

OrdinaryDecomposition orderToDecomposition(vector<int> const & order, vector<int> const & parent) {
	int n = order.size()-1;

	vector<vector<int>> graph(n+1,vector<int>());

	for (int v = 1; v <= n; v++)
		if (parent[v] != 0) {
			graph[v].push_back(parent[v]);
			graph[parent[v]].push_back(v);
		}

	vector<vector<int>> vertices(n+1);
	for (int v = 1; v <= n; v++) {
		vertices[v] = LN[v];
		vertices[v].push_back(v);
	}

	return OrdinaryDecomposition(move(vertices),move(graph));

#ifdef DEBUG
	for (int v = 1; v < ((int) order.size()); v++) {
		printf("%d :", v);
		for (int w : G[v])
			printf(" %d", w);
		printf("\n");
	}
	vector<bool> colored(order.size(),false);

#endif

	// rooting the tree

}

//
// LN
//

vector<vector<int>> computeLN(vector<int> const & num) {
	auto result = vector<vector<int>>(num.size());

	for (int v = 1; v < ((int) num.size()); v++)
		for (int w : G[v]) if (num[w] < num[v])
			result[v].push_back(w);

	return result;
}

//
// Parent
//

vector<int> computeParent(vector<int> const & num, int root) {
	vector<int> result(((int) num.size()),0);

	for (int v = 1; v < ((int) num.size()); v++)
		for (int w : G[v])
			if (num[w] < num[v] and num[w] > num[result[v]])
				result[v] = w;

	for (int v = 1; v < ((int) num.size()); v++)
		if (result[v] == 0 and v != root)
			result[v] = root;

	return result;
}

//
// Nodes which can be final
//

void markFinal(vector<TWNode> & nodes, int root) {
	if (nodes[root].type == ADD_VERTEX)
		return;

	nodes[root].is_final = true;
	for (int child : nodes[root].children)
		markFinal(nodes, child);
}

//
// Sort decomposition
//
//

int findRoot(vector<TWNode> const & nodes) {
	for (int i = 0; i < ((int) nodes.size()); i++)
		if (nodes[i].parent == -1)
			return i;
	return -1;
}

void sortDecomposition(vector<TWNode> const & nodes, int root, vector<TWNode> & result, vector<int> & new_num) {
	new_num[root] = result.size();
	result.push_back(nodes[root]);

#ifdef DEBUG
	printf("ROOT: %d\n", root);
#endif

	for (int child : nodes[root].children)
		sortDecomposition(nodes, child, result, new_num);
}

void sortDecomposition(vector<TWNode> & nodes) {
	vector<TWNode> result;
	vector<int> new_num(nodes.size()); 

	int root = findRoot(nodes);

	sortDecomposition(nodes, root, result, new_num);

	for (auto & node : result) {
		if (node.parent != -1)
			node.parent = new_num[node.parent];
		for (int & child : node.children)
			child = new_num[child];
	}

	nodes = result;
}

//
// The decomposition
//

OrdinaryDecomposition decompose(int n, vector<vector<int>> _G, set<pair<int,int>> _edges) {
	if (n == 0)
		return OrdinaryDecomposition();

	G = _G;
	edges = _edges;

	vertexNums = vector<int>(G.size());
	mark = vector<bool>(G.size());

	for (auto e : edges)
		isOriginal[e] = true;

	for (int v = 1; v <= n; v++) {
		deleted[v] = false;
		mark[v] = 0;
		vertexNums[v] = computeMissingedges(v);
	}
#ifdef DEBUG
	printf("N: %d\n", n);
	printf("FIRST PHASE FINISHED\n");
#endif

	vector<int> order;
	order.push_back(0);
	
	for (int i = 1; i <= n; i++) {
		int v = findMinVertex(n);
		order.push_back(v);
		deleteVertex(v);
	}

	reverse(next(order.begin()), order.end());
	vector<int> num = invPerm(order);

#ifdef DEBUG
	printf("ORDER:\n");
	for (int v : order) {
		printf("%d ", v);
	}
	printf("\n");
	for (int i = 1; i < num.size(); i++) {
		printf("num[%d] = %d\n", i, num[i]);
	}
#endif

	vector<int> parent = computeParent(num, order[1]);
	LN = computeLN(num);

#ifdef DEBUG
	for (int i = 1; i < num.size(); i++) {
		printf("parent[%d] = %d\n", i, parent[i]);
	}
	for (int i = 1; i < LN[i].size(); i++) {
		printf("LN[%d]:", i);
		for (int w : LN[i])
			printf(" %d", w);
		printf("\n");
	}

	printf("LNs:\n");
	for (int i = 1; i < LN[i].size(); i++)
		printf("%d ", LN[i].size());
	printf("\n");
#endif

	return orderToDecomposition(order,parent);
}

vector<TWNode> decomposeNicely(int n, vector<vector<int>> _G, set<pair<int,int>> _edges) {
	return makeDecompositionNice(decompose(n,_G,_edges));
}

OrdinaryDecomposition reduceStupidBranches(OrdinaryDecomposition decomposition) {
	int n = decomposition.graph.size()-1;

	for (int node = 1; node <= n; node++)
		sort(decomposition.vertices[node].begin(),decomposition.vertices[node].end());

	vector<bool> deleted(n+1,false);
	vector<int> deg(n+1,1);
	queue<int> leaves;

	for (int node = 1; node <= n; node++)
		if (decomposition.graph[node].size() == 1)
			leaves.push(node);
		else
			deg[node] = (int) decomposition.graph[node].size();

	int nodes_left = n;
	while (not leaves.empty() and nodes_left > 1) {
		int node = leaves.front();
		leaves.pop();

		for (int neigh : decomposition.graph[node])
			if (not deleted[neigh] and isDiffEmpty(decomposition.vertices[node],decomposition.vertices[neigh])) {
				deleted[node] = true;
				deg[neigh]--;

				if (deg[neigh] == 1)
					leaves.push(neigh);

				nodes_left--;
			}
	}

	OrdinaryDecomposition result;

	// get new nums

	vector<int> new_num(n+1);

	result.vertices.push_back(vector<int>());
	for (int node = 1; node <= n; node++)
		if (not deleted[node]) {
			new_num[node] = result.vertices.size();
			result.vertices.push_back(decomposition.vertices[node]);
		}

	// clean graph

	result.graph.push_back(vector<int>());
	for (int node = 1; node <= n; node++)
		if (not deleted[node]) {
			vector<int> new_edges;
			for (int neigh : decomposition.graph[node])
				if (not deleted[neigh])
					new_edges.push_back(new_num[neigh]);
			result.graph.push_back(new_edges);
		}

	return result;
}

vector<TWNode> makeDecompositionNice(OrdinaryDecomposition decomposition) {
	auto result = graphToTree(reduceStupidBranches(decomposition));

	createLeaves(result);
	createAddAndRemoveNodes(result);
	sortVertices(result);
	rebuildJoinNodes(result);
	createAddEdgeNodes(result);
	removeNoneNodes(result);
	sortDecomposition(result);
	markFinal(result,0);
	
	return result;
}

bool operator==(TWNode const & A, TWNode const & B) {
	if (A.type != B.type)
		return false;
	return true;
}

int whichChildIAm(int node_num, vector<TWNode> const & nodes) {
	auto const & parent = nodes[nodes[node_num].parent];

	if (parent.children[0] == node_num)
		return 0;
	else
		return 1;
}

vector<TWNode> decompositionWithoutedges(vector<TWNode> nodes, set<pair<int,int>> edges) {
	for (int i = 1; i < ((int)nodes.size()); i++) {
		auto & node = nodes[i];
		if (node.type == ADD_EDGE) {
			int v = node.vertices[node.v];
			int w = node.vertices[node.w];

			if (edges.count(edge(v,w)) == 1) {
				int parent = node.parent;
				int child = node.children[0];
				int num = whichChildIAm(i, nodes);
				
				nodes[parent].children[num] = child;
				nodes[child].parent = parent;
				node.parent = -2;
			}
		}

		for (auto edge : edges)
			if (node.edges.count(edge) == 1)
				node.edges.erase(edge);
	}

	return nodes;
}

void setEdgeModes(vector<TWNode> & decomposition, EdgeSet const & deleted, EdgeSet const & chosen) {
	for (auto & node : decomposition)
		if (node.type == ADD_EDGE) {
			int v = node.vertices[node.v];
			int w = node.vertices[node.w];
			auto e = edge(v,w);

			if (deleted.count(e) != 0)
				node.mode = DELETED;
			else if (chosen.count(e) != 0)
				node.mode = CHOSEN;
			else
				node.mode = AVAILABLE;
		}
}

void setGraphAndEdges(Graph _G, EdgeSet _edges) {
	G = _G;
	edges = _edges;
	for (auto edge : edges)
		isOriginal[edge] = true;
}
