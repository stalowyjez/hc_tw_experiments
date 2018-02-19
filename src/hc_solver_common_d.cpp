#include <algorithm>
#include <vector>
#include <memory>
#include <set>
#include <map>
#include <functional>
#include <unordered_map>
#include <cstring>
#include <cmath>

#include "basic.h"
#include "gaussian.h"
#include "edge_selection.h"
#include "hc_solver_common_d.h"

using namespace std;

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

vector<PartialSolution> addEdge(PartialSolution solution, int v, int w, pair<int,int> const & e, int k, bool is_final) {
	if (is_final and isReallyFinal(solution.match, k, v, w)) {
		setPSVal(solution.match,v,TWO_EDGES);
		setPSVal(solution.match,w,TWO_EDGES);

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

		return PartialSolutions({solution,new_solution});
	} else {
		return PartialSolutions({solution});
	}
}

vector<PartialSolution> addChosenEdge(PartialSolution solution, int v, int w, pair<int,int> const & e, int k, bool is_final) {
	if (is_final and isReallyFinal(solution.match, k, v, w)) {
		setPSVal(solution.match,v,TWO_EDGES);
		setPSVal(solution.match,w,TWO_EDGES);

		return vector<PartialSolution>({solution});
	}

	int v_match = getPSVal(solution.match,v);
	int w_match = getPSVal(solution.match,w);

	if (v_match != TWO_EDGES and w_match != TWO_EDGES and v_match != w) {
		if (v_match != v)
			setPSVal(solution.match,v,TWO_EDGES);
		if (w_match != w)
			setPSVal(solution.match,w,TWO_EDGES);

		setPSVal(solution.match,v_match,w_match);
		setPSVal(solution.match,w_match,v_match);

		return PartialSolutions({solution});
	} else {
		return PartialSolutions();
	}
}

PartialSolutions addEdge(TWNode const & node, PartialSolutions const & solutions) {
	if (node.mode == DELETED)
		return solutions;

	auto e = mp(node.vertices[node.v],node.vertices[node.w]);
	auto is_final = node.is_final;
	auto k = node.vertices.size();
	
	if (node.mode == AVAILABLE) {
		function<PartialSolutions(PartialSolution)> func = 
			[&](PartialSolution const & ps){
				return addEdge(ps,node.v,node.w,e,k,is_final);
			};
		return flatMap(solutions,func);
	} else {
		function<PartialSolutions(PartialSolution)> func = 
			[&](PartialSolution const & ps){
				return addChosenEdge(ps,node.v,node.w,e,k,is_final);
			};
		return flatMap(solutions,func);
	}
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
	return flatMap(solutions, func);
}

PartialSolutions basicJoin(TWNode const & node, PartialSolutions const & solutions_a, PartialSolutions const & solutions_b) {
	function<PartialSolutions(PartialSolution)> func =
		[&](PartialSolution const & ps1){return join(ps1,solutions_a,node.vertices.size());};
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

	map<coloring_t,PartialSolutions> map_a;
	for (auto solution : solutions_a) {
		auto coloring = getBucket(solution.match,k);
		if (map_a.count(coloring))
			map_a[coloring] = PartialSolutions();
		map_a[coloring].push_back(solution);
	}

	map<coloring_t,PartialSolutions> map_b;
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
		for (auto solution : basicJoin(node,map_a[colorings.first],map_b[colorings.second]))
			result.push_back(solution);
	}

	return result;
}

//
// Constructor
//

HCSolver::HCSolver(function<PartialSolutions(PartialSolutions,int)> r) :
	reduction(r)
{}

//
// Processong nodes
//

PartialSolutions HCSolver::process_node(int root, vector<TWNode> const & nodes) {
	int n = nodes[root].vertices.size();

	vector<PartialSolutions> partial_solutions;

	for (auto child : nodes[root].children)
		partial_solutions.push_back(process_node(child, nodes));

	PartialSolutions result;

	switch (nodes[root].type) {
		case ADD_VERTEX :
			result = addVertex(nodes[root],partial_solutions[0]);
			break;
		case ADD_EDGE :
			result = reduction(addEdge(nodes[root],partial_solutions[0]),n);
			break;
		case DEL_VERTEX :
			result = delVertex(nodes[root],partial_solutions[0]);
			break;
		case JOIN :
			result = reduction(join(nodes[root],partial_solutions[0],partial_solutions[1]),n);
			break;
		case LEAF :
			result = PartialSolutions({PartialSolution()});
			break;
		default :
			result = partial_solutions[0];
	}

	return move(result);
}

//
// Getting result
//

bool HCSolver::hasHC(vector<TWNode> const & nodes) {
	return (process_node(0, nodes).size() != 0);
}

pair<bool,vector<int>> HCSolver::getHC(vector<TWNode> & nodes, vector<vector<int>> & graph, set<pair<int,int>> & edges, int n) {
	if (not hasHC(nodes))
		return mp(false,vector<int>());

	EdgeSet selected_edges = edge_selection::syphilisSelection(graph,edges,[&](EdgeSet deleted, EdgeSet chosen){
		setEdgeModes(nodes,deleted,chosen);
		return this->hasHC(nodes);
	});

	vector<vector<int>> neighbours(graph.size());
		
	for (auto e : selected_edges) {
		neighbours[e.first].push_back(e.second);
		neighbours[e.second].push_back(e.first);
	}

	vector<int> result;

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

	vector<PartialSolution> new_result;
	for (int i = 0; i <= ((int)result.size())-2; i++)
		if (result[i] != result[i+1])
			new_result.push_back(result[i]);
	new_result.push_back(result[result.size()-1]);

	return new_result;
}

//
// Needed for both reductions
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
			if (sev_num <= 2 or (int)bucket.size() < (int)(8.0*(double)(1 << (sev_num/2 - 1)))) {
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

			if (sev_num <= 2 or bucket.size() < (size_t)(2.0*(double)(1 << (sev_num/2 - 1)))) {
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
