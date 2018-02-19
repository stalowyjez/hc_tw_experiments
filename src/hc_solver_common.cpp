#include <algorithm>
#include <vector>
#include <memory>
#include <cmath>
#include <set>
#include <map>
#include <functional>
#include <unordered_map>
#include <cstdlib>
#include <cstring>

#include "basic.h"
#include "gaussian.h"
#include "hc_solver_common.h"

using namespace std;

HCEdge::HCEdge() {
	this->edge = mp(-1,-1);
}

HCEdge::HCEdge(pair<int,int> const & edge, shared_ptr<HCEdge> pv) {
	this->edge = edge;
	this->previous = pv;
}

PartialSolution::PartialSolution(shared_ptr<HCEdge> e) :
	edges(e)
{
	for (int i = 0; i < PS_SIZE; i++)
		this->match[i] = 0;
}

PartialSolution::PartialSolution() {
	for (int i = 0; i < PS_SIZE; i++)
		this->match[i] = 0;
}

void setPSVal(char * match, int pos, int val) {
	match[pos] = val;
}

int getPSVal(const char * match, int pos) {
	return match[pos];
}


//
// For sets
//

size_t hash<PartialSolution>::operator() (PartialSolution const & ps) const {
	size_t result = 0L;

	for (int i = 0; i < PS_SIZE; i++)
		result = (result * MUL + ps.match[i]) % MOD;

	return result;
}

bool operator==(PartialSolution const & A, PartialSolution const & B) {
	for (int i = 0; i < PS_SIZE; i++)
		if (A.match[i] != B.match[i])
			return false;
	return true;
}

bool operator!=(PartialSolution const & A, PartialSolution const & B) {
	return not (A==B);
}

bool operator<(PartialSolution const & A, PartialSolution const & B) {
	for (int i = 0; i < PS_SIZE; i++)
		if (A.match[i] < B.match[i]) {
			return true;
		} else if (A.match[i] > B.match[i]) {
			return false;
		} else {}

	return false;
}

//
// Add vertex
//

vector<PartialSolution> addVertex(PartialSolution solution, int v, int k) {
	for (int i = k; i > v; i--) {
		int match = getPSVal(solution.match,i-1);
		if (match >= v and match != TWO_EDGES)
			setPSVal(solution.match,i,match+1);
		else
			setPSVal(solution.match,i,match);
	}

	setPSVal(solution.match, v, v);

	for (int i = v-1; i >= 0; i--) {
		int match = getPSVal(solution.match,i);
		if (match >= v and match != TWO_EDGES)
			setPSVal(solution.match,i,match+1);
	}

	return vector<PartialSolution>({solution});
}

PartialSolutions addVertex(TWNode const & node, PartialSolutions const & solutions) {
	function<vector<PartialSolution>(PartialSolution)> func =
		[&](PartialSolution const & ps){return addVertex(ps,node.v,node.vertices.size()-1);};
	return flatMap(solutions,func);
}

//
// Add edge
//

bool isReallyFinal(const char * match, int k, int v, int w) {
	for (int i = 0; i < k; i++) {
		int match_i = getPSVal(match, i);
		if ((match_i != TWO_EDGES and i != v and i != w)
				or (i == v and match_i != w)
				or (i == w and match_i != v))
			return false;
	}

	return true;
}

#ifdef MARKED_EDGES
vector<PartialSolution> addEdge(PartialSolution solution, int v, int w, pair<int,int> const & e, int k, bool is_final, bool is_marked) {
	if (is_marked and is_final and isReallyFinal(solution.match, k, v, w)) {
		setPSVal(solution.match,v,TWO_EDGES);
		setPSVal(solution.match,w,TWO_EDGES);

		solution.edges = shared_ptr<HCEdge>(new HCEdge(e,solution.edges));

		return vector<PartialSolution>({solution});
	}

	int v_match = getPSVal(solution.match,v);
	int w_match = getPSVal(solution.match,w);

	if (is_marked) {
		if (v_match != v)
			setPSVal(solution.match,v,TWO_EDGES);
		if (w_match != w)
			setPSVal(solution.match,w,TWO_EDGES);

		setPSVal(solution.match,v_match,w_match);
		setPSVal(solution.match,w_match,v_match);

		solution.edges = shared_ptr<HCEdge>(new HCEdge(e,solution.edges));
	}

	return PartialSolutions({solution});
}
#else
vector<PartialSolution> addEdge(PartialSolution solution, int v, int w, pair<int,int> const & e, int k, bool is_final) {
	if (is_final and isReallyFinal(solution.match, k, v, w)) {
		setPSVal(solution.match,v,TWO_EDGES);
		setPSVal(solution.match,w,TWO_EDGES);

		solution.edges = shared_ptr<HCEdge>(new HCEdge(e,solution.edges));

		return vector<PartialSolution>({solution});
	}

	int v_match = getPSVal(solution.match,v);
	int w_match = getPSVal(solution.match,w);

	if (v_match != TWO_EDGES and w_match != TWO_EDGES and v_match != w) {
		PartialSolution new_solution = solution;

		if (v_match != v)
			setPSVal(new_solution.match,v,TWO_EDGES);
		if (w_match != w)
			setPSVal(new_solution.match,w,TWO_EDGES);

		setPSVal(new_solution.match,v_match,w_match);
		setPSVal(new_solution.match,w_match,v_match);

		new_solution.edges = shared_ptr<HCEdge>(new HCEdge(e,solution.edges));

#ifdef DEBUG
		auto current = solution.edges;
		printf("EDGES IN A SOLUTION AFTER ADDING EDGE (%d,%d):", current->edge.first, current->edge.second);
		fflush(NULL);
		while (current->edge.first != -1) {
			printf(" (%d,%d)", current->edge.first, current->edge.second);
			current = current->previous;
		}
#endif

		return PartialSolutions({solution,new_solution});
	} else {
		return PartialSolutions({solution});
	}
}
#endif

PartialSolutions addEdge(TWNode const & node, PartialSolutions const & solutions) {

	auto e = mp(node.vertices[node.v],node.vertices[node.w]);
	auto is_final = node.is_final;
	auto k = node.vertices.size();
#ifdef MARKED_EDGES
	auto is_marked = node.marked;
#endif

	function<PartialSolutions(PartialSolution)> func =
		[&](PartialSolution const & ps){
#ifdef MARKED_EDGES
			return addEdge(ps,node.v,node.w,e,k,is_final,is_marked);
#else
			return addEdge(ps,node.v,node.w,e,k,is_final);
#endif
		};
	return flatMap(solutions,func);
}

//
// Del vertex
//

PartialSolutions delVertex(PartialSolution solution, int v, int k) {
	if (getPSVal(solution.match,v) != TWO_EDGES)
		return PartialSolutions();

	for (int i = 0; i < v; i++) {
		int match = getPSVal(solution.match,i);
		if (match > v and match != TWO_EDGES)
			setPSVal(solution.match,i,match-1);
	}
	for (int i = v; i <= k; i++) {
		int match = getPSVal(solution.match,i+1);
		if (match > v and match != TWO_EDGES)
			setPSVal(solution.match,i,match-1);
		else
			setPSVal(solution.match,i,match);
	}

	setPSVal(solution.match,k+1,0);

	return PartialSolutions({solution});
}

PartialSolutions delVertex(TWNode const & node, PartialSolutions & solutions) {
	function<PartialSolutions(PartialSolution)> func =
		[&](PartialSolution const & ps){return delVertex(ps,node.v,node.vertices.size()-1);};
	return flatMap(solutions,func);
}

//
// Join
// 

inline size_t edgeHash(shared_ptr<HCEdge> const edges) {
	return (size_t) edges.get();
}

shared_ptr<HCEdge> getTailEdge(shared_ptr<HCEdge> edges) {
	if (edges->edge.first == -1)
		return edges;
	else
		return getTailEdge(edges->previous);
}

void copyTree(shared_ptr<HCEdge> edges, unordered_map<size_t,shared_ptr<HCEdge>> & copied_version) {
	if (copied_version.count(edgeHash(edges)))
		return;

	copyTree(edges->previous,copied_version);
	copied_version[edgeHash(edges)] = shared_ptr<HCEdge>(new HCEdge(edges->edge,copied_version[edgeHash(edges->previous)]));
}

PartialSolutions deepCopy(PartialSolutions const & solutions, shared_ptr<HCEdge> base_edges) {
	if (solutions.size() == 0)
		return PartialSolutions();

	PartialSolutions result;
	unordered_map<size_t,shared_ptr<HCEdge>> copied_version;

	// Preparing tree
	
	copied_version[edgeHash(getTailEdge(solutions.begin()->edges))] = base_edges;
	for (auto solution : solutions)
		copyTree(solution.edges,copied_version);

	// Copying

	for (auto solution : solutions) {
		solution.edges = copied_version[edgeHash(solution.edges)];
		result.push_back(solution);
	}

	return result;
}

PartialSolutions deepCopy(PartialSolutions const & solutions, shared_ptr<HCEdge> base_edges, unordered_map<size_t,unordered_map<size_t,shared_ptr<HCEdge>>> & edge_map) {
	if (solutions.size() == 0)
		return PartialSolutions();

	PartialSolutions result;

	if (edge_map.count(edgeHash(base_edges)) == 0) {
		edge_map[edgeHash(base_edges)] = unordered_map<size_t,shared_ptr<HCEdge>>();
		edge_map[edgeHash(base_edges)][edgeHash(getTailEdge(solutions.begin()->edges))] = base_edges;
	}

	for (auto solution : solutions)
		copyTree(solution.edges,edge_map[edgeHash(base_edges)]);

	for (auto solution : solutions) {
		solution.edges = edge_map[edgeHash(base_edges)][edgeHash(solution.edges)];
		result.push_back(solution);
	}

	return result;
}

/*bool checkEdges(shared_ptr<HCEdge> e) {
	set<pair<int,int>> S;

	while (e->edge != mp(-1,-1))
		if (S.count(e->edge) != 0)
			return false;
		else {
			S.insert(e->edge);
			e = e->previous;
		}

	return true;
}*/

PartialSolutions join(PartialSolution base_solution, PartialSolution solution, int k) {
	for (int v = 0; v < k; v++) {
		int v_match_1 = getPSVal(base_solution.match,v);
		int v_match_2 = getPSVal(solution.match,v);

		if (v_match_1 == TWO_EDGES) {
			setPSVal(base_solution.match,v,v_match_2);
			setPSVal(solution.match,v,TWO_EDGES);
		}

		v_match_1 = getPSVal(base_solution.match,v);
		v_match_2 = getPSVal(solution.match,v);

		if (v_match_2 == TWO_EDGES)
			if (v_match_1 != v)
				return PartialSolutions();
	}

	for (int v = 0; v < k; v++) {
		int w = getPSVal(base_solution.match,v);
		if (w >= v)
			continue;

		int v_match = getPSVal(solution.match,v);
		int w_match = getPSVal(solution.match,w);

		if (v_match == w or w_match == v)
			return PartialSolutions();

		if (v_match != v)
			setPSVal(solution.match,v,TWO_EDGES);
		if (w_match != w)
			setPSVal(solution.match,w,TWO_EDGES);

		setPSVal(solution.match,v_match,w_match);
		setPSVal(solution.match,w_match,v_match);
	}

	return vector<PartialSolution>({solution});
}


PartialSolutions join(PartialSolution base_solution, PartialSolutions const & solutions, int k) {
	function<vector<PartialSolution>(PartialSolution)> func =
		[&](PartialSolution ps){return join(base_solution, ps, k);};
	return flatMap(deepCopy(solutions, base_solution.edges), func);
}

PartialSolutions join(PartialSolution base_solution, PartialSolutions const & solutions, int k, unordered_map<size_t,unordered_map<size_t,shared_ptr<HCEdge>>> & edge_map) {
	function<vector<PartialSolution>(PartialSolution)> func =
		[&](PartialSolution ps){return join(base_solution, ps, k);};
	return flatMap(deepCopy(solutions,base_solution.edges,edge_map), func);
}

//#define ALT_JOIN

#ifndef ALT_JOIN

PartialSolutions join(TWNode const & node, PartialSolutions const & solutions_a, PartialSolutions const & solutions_b) {
	function<PartialSolutions(PartialSolution)> func =
		[&](PartialSolution const & ps1){return join(ps1,solutions_a,node.vertices.size());};
	return flatMap(solutions_b,func);
}

#else

PartialSolutions basicJoin(TWNode const & node, PartialSolutions const & solutions_a, PartialSolutions const & solutions_b) {
	function<PartialSolutions(PartialSolution)> func =
		[&](PartialSolution const & ps1){return join(ps1,solutions_a,node.vertices.size());};
	return flatMap(solutions_b,func);
}

PartialSolutions basicJoin(TWNode const & node, PartialSolutions const & solutions_a, PartialSolutions const & solutions_b, unordered_map<size_t,unordered_map<size_t,shared_ptr<HCEdge>>> & edge_map) {
	function<PartialSolutions(PartialSolution)> func =
		[&](PartialSolution const & ps1){return join(ps1,solutions_a,node.vertices.size(),edge_map);};
	return flatMap(solutions_b,func);
}


typedef size_t coloring_t;

pair<coloring_t,coloring_t> coloringToTwo(coloring_t coloring, size_t k) {
	coloring_t coloring_a=0l, coloring_b=0l;

	for (size_t i = 0l; i < k; i++) {
		size_t color = coloring%6l;
		coloring /= 6l;

		coloring_a <<= 2l;
		coloring_b <<= 2l;

		switch (color) {
			case 0 :
				coloring_a += 0l;
				coloring_b += 0l;
				break;
			case 1 :
				coloring_a += 1l;
				coloring_b += 0l;
				break;
			case 2 :
				coloring_a += 0l;
				coloring_b += 1l;
				break;
			case 3 :
				coloring_a += 1l;
				coloring_b += 1l;
				break;
			case 4 :
				coloring_a += 2l;
				coloring_b += 0l;
				break;
			case 5 :
				coloring_a += 0l;
				coloring_b += 2l;
				break;
		}
	}

	return mp(coloring_a,coloring_b);
}

PartialSolutions join(TWNode const & node, PartialSolutions const & solutions_a, PartialSolutions const & solutions_b) {
	size_t k = node.vertices.size();

	if (solutions_a.size()*solutions_b.size() < 32*pow(6l,k))
		return basicJoin(node,solutions_a,solutions_b);

	PartialSolutions result;
	unordered_map<size_t,unordered_map<size_t,shared_ptr<HCEdge>>> edge_map;

	unordered_map<coloring_t,PartialSolutions> map_a;
	for (auto solution : solutions_a) {
		auto coloring = getBucket(solution.match,k);
		if (map_a.count(coloring))
			map_a[coloring] = PartialSolutions();
		map_a[coloring].push_back(solution);
	}

	unordered_map<coloring_t,PartialSolutions> map_b;
	for (auto solution : solutions_b) {
		auto coloring = getBucket(solution.match,k);
		if (map_b.count(coloring))
			map_b[coloring] = PartialSolutions();
		map_b[coloring].push_back(solution);
	}

	for (size_t coloring = 0l; coloring <= pow(6l,k); coloring++) {
		auto colorings = coloringToTwo(coloring,k);
		if (map_a.count(colorings.first) == 0 or map_b.count(colorings.second) == 0)
			continue;
		for (auto solution : basicJoin(node,map_a[colorings.first],map_b[colorings.second],edge_map))
			result.push_back(solution);
	}

	return result;
}

#endif

//
// Constructor
//

HCSolver::HCSolver(function<PartialSolutions(PartialSolutions,int)> r) :
	tail_edge(shared_ptr<HCEdge>(new HCEdge())),
	reduction(r)
{}

//
// Processong nodes
//

PartialSolutions HCSolver::process_node(int root, vector<TWNode> const & nodes) {

	int k = nodes[root].vertices.size();

	vector<PartialSolutions> partial_solutions;

	for (auto child : nodes[root].children)
		partial_solutions.push_back(process_node(child, nodes));

	PartialSolutions result;

	switch (nodes[root].type) {
		case ADD_VERTEX :
			result = addVertex(nodes[root],partial_solutions[0]);
			break;
		case ADD_EDGE :
			result = reduction(addEdge(nodes[root],partial_solutions[0]),k);
			break;
		case DEL_VERTEX :
			result = delVertex(nodes[root],partial_solutions[0]);
			break;
		case JOIN :
			result = reduction(join(nodes[root],partial_solutions[0],partial_solutions[1]),k);
			break;
		case LEAF :
			result = PartialSolutions({PartialSolution(tail_edge)});
			break;
		default :
			result = partial_solutions[0];
	}

	//return reduction(move(result),nodes[root].vertices.size());
	return result;
}

//
// Getting result
//

pair<bool,vector<int>> HCSolver::getHC(vector<TWNode> & nodes, vector<vector<int>> & graph, set<pair<int,int>> & edges, int n) {
	PartialSolutions solutions = process_node(0, nodes);

	if (solutions.size() != 0) {

		auto solution = *solutions.begin();

		vector<int> result;
		vector<vector<int>> neighbours(n+1);

		auto current = solution.edges;

		while (current->edge.first != -1) {
			neighbours[current->edge.first].push_back(current->edge.second);
			neighbours[current->edge.second].push_back(current->edge.first);
			current = current->previous;
		}

		int v = 1;
		int last = neighbours[1][1];

		for (int i = 1; i <= n; i++) {
			result.push_back(v);
			int tmp = v;

			if(neighbours[v][0] != last)
				v = neighbours[v][0];
			else
				v = neighbours[v][1];
			last = tmp;
		}

		return mp(true,result);

	} else {

		return mp(false,vector<int>());

	}
}

//
// Bucket for PartialSolution
//

size_t getBucket(char const * match, int k) {
	size_t result = 0l;

	for (int v = 0; v < k; v++) {

		auto v_match = getPSVal(match,v);

		result = result << 2;

		if (v_match != v)
			result += 1l;
		if (v_match == TWO_EDGES)
			result += 1l;
	}

	return result;
}

unordered_map<size_t,PartialSolutions> getBuckets(PartialSolutions solutions, int k) {
	unordered_map<size_t,PartialSolutions> result;

	for (auto solution : solutions) {
		auto bucket = getBucket(solution.match, k);
		if (result.count(bucket) == 0)
			result[bucket] = PartialSolutions();
		result[bucket].push_back(solution);
	}

	return result;
}

PartialSolutions simple_reduction(PartialSolutions result, size_t k) {
	if (result.size() <= 1)
		return result;

	sort(result.begin(), result.end());

	auto first_free = result.begin();
	for (auto ptr = result.begin(); ptr != prev(result.end()); ptr++)
		if (*ptr != *next(ptr)) {
			*first_free = *ptr;
			first_free++;
		}
	*first_free = *prev(result.end());
	first_free++;

	if (first_free != prev(result.end()))
		result.erase(first_free, result.end());

	return result;
}

//
// For both reductions
//

int sevNum(char const * match, int k) {
	int result = 0;
	for (int v = 0; v < k; v++) {
		int val = getPSVal(match,v);
		if (val != v and val != TWO_EDGES)
			result++;
	}

	return result;
}

//
// Reductions
//

namespace rgaussian1 {
	//
	// Getting all possible divisions of the set
	//

	vector<set<char>> getDivisions(set<char> s) {
		if (s.size() == 1)
			return vector<set<char>>({set<char>()});

		int v = *s.begin();

		s.erase(v);
		auto result = getDivisions(s);
		int init_size = result.size();

		for (int i = 0; i < init_size; i++) {
			auto new_one = result[i];
			new_one.insert(v);
			result.push_back(new_one);
		}

		return result;
	}

	vector<set<char>> getDivisions(const char * match, int k) {
		set<char> s;
		for (int v = 0; v < k; v++) {
			auto v_match = getPSVal(match,v);
			if (v_match != TWO_EDGES and v_match != v)
				s.insert(v);
		}

		if (s.size() == 0)
			return vector<set<char>>({set<char>()});

		return getDivisions(s);
	}

	bool ifDivisionMatch(set<char> const & div, char const * match, int k) {
		for (int v = 0; v < k; v++) {
			auto v_match = getPSVal(match,v);
			if (div.count(v) == 1 and div.count(v_match) == 0)
				return false;
		}
		return true;
	}

	//
	// Reduction
	//

	PartialSolutions reduction(PartialSolutions solutions, int k) {
		solutions = simple_reduction(solutions,k);

		auto buckets = getBuckets(solutions, k);

		PartialSolutions result;

		for (auto & p : buckets) {
			auto const & bucket = p.second;

			if (bucket.size() == 0)
				continue;

			int sev_num = sevNum(bucket[0].match, k);
			if (sev_num <= 2 or
				(sev_num == 4 and (int)bucket.size() < 4) or
				(sev_num == 6 and (int)bucket.size() < 17) or
				(sev_num == 8 and (int)bucket.size() < 18)
			) {
				result.insert(result.end(), bucket.begin(), bucket.end());
				continue;
			}

			auto divisions = getDivisions(bucket[0].match, k);

			vector<vector<bool>> matrix(bucket.size());
			for (int i = 0; i < ((int)bucket.size()); i++)
				for (auto div : divisions)
					matrix[i].push_back(ifDivisionMatch(div,bucket[i].match,k));

			auto nums = getSignificantRows(matrix);

			for (int num : nums)
				result.push_back(bucket[num]);
		}

		return result;
	}
}

namespace rgaussian2 {
	//
	// Getting needed solutions
	//

	vector<pair<char,char>> intToMatch(vector<int> const & vertices, int n) {
		if (vertices.size() == 0)
			return vector<pair<char,char>>();

		vector<pair<char,char>> result;

		int k = vertices.size()/2;
		int last = vertices[0];

		for (int i = 0; i < k-1; i++) {
			int base = i*2+1;
			int val = getBit(n,i);

			int v = vertices[base+val];
			
			result.push_back(mp(last,v));
			last = vertices[base+(1-val)];
		}

		result.push_back(mp(last,vertices[vertices.size()-1]));

		return result;
	}

	vector<vector<pair<char,char>>> getEdges(const char * base_match, int k) {
		vector<int> vertices;

		// Obtaining vertices

		for (int v = 0; v < k; v++) {
			int match_v = getPSVal(base_match,v);
			if (match_v != v and match_v != TWO_EDGES)
				vertices.push_back(v);
		}

		if (vertices.size() == 0l)
			return vector<vector<pair<char,char>>>({vector<pair<char,char>>()});

		// Getting edges

		vector<vector<pair<char,char>>> result;

		for (int i = 0; i < (1 << ((((int)vertices.size())>>1)-1)); i++)
			result.push_back(intToMatch(vertices, i));

		return result;
	}

	//
	// Checking if solutions matches
	//

	bool ifSolutionMatchesEdges(char const * match, vector<pair<char,char>> const & edges, int k) {
		if (edges.size() == 0)
			return true;

		char tmp_match[PS_SIZE];
		memcpy(tmp_match,match,PS_SIZE);

		for (int v = 0; v < k; v++)
			if (getPSVal(tmp_match,v) == v)
				setPSVal(tmp_match,v,TWO_EDGES);

		for (int i = 0; i < ((int)edges.size()); i++) {
			auto edge = edges[i];

			int v = edge.first;
			int w = edge.second;

			int match_v = getPSVal(tmp_match,v);
			int match_w = getPSVal(tmp_match,w);

			if (i == ((int)edges.size()-1) and isReallyFinal(tmp_match,k,v,w))
				return true;

			if (match_v == TWO_EDGES or match_w == TWO_EDGES or match_v == w)
				return false;

			setPSVal(tmp_match,v,TWO_EDGES);
			setPSVal(tmp_match,w,TWO_EDGES);
			setPSVal(tmp_match,match_w,match_v);
			setPSVal(tmp_match,match_v,match_w);
		}

		return false;
	}

	//
	// Reduction
	//

	PartialSolutions reduction(PartialSolutions solutions, int k) {
		solutions = simple_reduction(solutions,k);

		auto buckets = getBuckets(solutions, k);

		PartialSolutions result;

		for (auto & p : buckets) {
			auto & bucket = p.second;

			int sev_num = sevNum(bucket[0].match, k);
			if (sev_num <= 2 or
				(sev_num == 4 and (int)bucket.size() < 3) or
				(sev_num == 6 and (int)bucket.size() < 5) or
				(sev_num == 8 and (int)bucket.size() < 9)
			) {
				result.insert(result.end(), bucket.begin(), bucket.end());
				continue;
			}

			auto fits = getEdges(bucket[0].match, k);

			vector<vector<bool>> matrix(bucket.size());
			for (int i = 0; i < ((int)bucket.size()); i++)
				for (auto fit : fits)
					matrix[i].push_back(ifSolutionMatchesEdges(bucket[i].match,fit,k));

			auto nums = getSignificantRows(matrix);

			for (int num : nums)
				result.push_back(bucket[num]);
		}

		return result;
	}
}
