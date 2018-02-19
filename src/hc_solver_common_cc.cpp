#include <algorithm>
#include <vector>
#include <memory>
#include <set>
#include <map>
#include <functional>
#include <unordered_map>
#include <complex>

#include "basic.h"
#include "hc_solver_common_cc.h"
#include "edge_selection.h"
#include "poly.h"

//#define DEBUG

using namespace std;

//
// Constructors
//

CCPartialSolution::CCPartialSolution() :
	types(0l),
	poly(1l)
{}

CCPartialSolution::CCPartialSolution(unsigned long long _types, unsigned long long _poly) :
	types(_types),
	poly(_poly)
{}

//
// Vertex Types
//

inline void setType(unsigned long long & types, int pos, Type type) {
	unsigned long long mask = 3l << (pos << 1l);
	types = (types & (~ mask)) + (type << (pos << 1l));
}

inline Type getType(unsigned long long const & types, int pos) {
	return ((types >> (pos << 1l)) % 4l);
}


//
// For sets
//

bool operator<(CCPartialSolution const & A, CCPartialSolution const & B) {
	if (A.types != B.types)
		return A.types < B.types;
	else
		return A.poly < B.poly;
}

bool operator==(CCPartialSolution const & A, CCPartialSolution const & B) {
	return A.types == B.types;
}

bool operator!=(CCPartialSolution const & A, CCPartialSolution const & B) {
	return A.types != B.types;
}

//
// Add vertex
//

CCPartialSolutions addVertex(TWNode const & node, CCPartialSolutions && solutions) {
	int k = node.vertices.size();
	int v = node.v;

	for (auto & solution : solutions) {
		for (int i = k-1; i > v; i--)
			setType(solution.types, i, getType(solution.types,i-1));

		setType(solution.types, v, NN);
	}

	return solutions;
}

//
// Add edge
//

CCPartialSolutions addRightEdge(CCPartialSolution solution, int v, int w, unsigned long long edge_poly, bool chosen) {
	CCPartialSolutions result;
	if (not chosen)
		result.push_back(solution);

	unsigned long long v_type = getType(solution.types,v);
	unsigned long long w_type = getType(solution.types,w);

	if (v_type == BT or w_type == BT or v_type == LF or w_type == LF)
		return result;

	CCPartialSolution new_solution = solution;
	new_solution.poly = polyMul(solution.poly,edge_poly);

	if (v_type == NN)
		setType(new_solution.types, v, RT);
	else
		setType(new_solution.types, v, BT);

	if (w_type == NN)
		setType(new_solution.types, w, RT);
	else
		setType(new_solution.types, w, BT);

	result.push_back(new_solution);
	return result;
}

CCPartialSolutions addEdge(CCPartialSolution solution, int v, int w, unsigned long long edge_poly, bool chosen) {
	CCPartialSolutions result;
	if (not chosen)
		result.push_back(solution);

	unsigned long long v_type = getType(solution.types,v);
	unsigned long long w_type = getType(solution.types,w);

	if (v_type == BT or w_type == BT or (v_type == LF and w_type == RT) or (v_type == RT and w_type == LF))
		return result;

	if (v_type == NN and w_type == NN) {
		CCPartialSolution new_solution_a = solution;
		new_solution_a.poly = polyMul(solution.poly,edge_poly);
		setType(new_solution_a.types,v,LF);
		setType(new_solution_a.types,w,LF);

		CCPartialSolution new_solution_b = solution;
		new_solution_b.poly = polyMul(solution.poly,edge_poly);
		setType(new_solution_b.types,v,RT);
		setType(new_solution_b.types,w,RT);

		result.push_back(new_solution_a);
		result.push_back(new_solution_b);
		return result;
	}

	CCPartialSolution new_solution = solution;
	new_solution.poly = polyMul(solution.poly,edge_poly);

	if (v_type == NN) {
		setType(new_solution.types, v, w_type);
		setType(new_solution.types, w, BT);
	} else if (w_type == NN) {
		setType(new_solution.types, w, v_type);
		setType(new_solution.types, v, BT);
	} else {
		setType(new_solution.types, v, BT);
		setType(new_solution.types, w, BT);
	}

	result.push_back(new_solution);
	return result;
}

CCPartialSolutions addEdge(TWNode const & node, CCPartialSolutions const & solutions) {
	if (node.mode == DELETED)
		return solutions;

	bool chosen = (node.mode == CHOSEN);

	unsigned long long edge_poly = randomPoly();

	function<CCPartialSolutions(CCPartialSolution)> func;

	if (node.vertices[node.v] == 1 or node.vertices[node.w] == 1)
		 func = [&](CCPartialSolution const & ps) {
			 return addRightEdge(ps,node.v,node.w,edge_poly,chosen);
		 };
	else
		func = [&](CCPartialSolution const & ps) {
			return addEdge(ps,node.v,node.w,edge_poly,chosen);
		};

	return flatMap(solutions,func);
}

//
// Del vertex
//

CCPartialSolutions delVertex(CCPartialSolution solution, int v, int k) {
	if (getType(solution.types,v) != BT)
		return CCPartialSolutions();

	for (int i = v; i < k; i++)
		setType(solution.types, i, getType(solution.types,i+1));

	setType(solution.types, k, NN);

	return CCPartialSolutions{solution};
}

CCPartialSolutions delVertex(TWNode const & node, CCPartialSolutions & solutions) {
	function<CCPartialSolutions(CCPartialSolution)> func =
		[&](CCPartialSolution const & ps){return delVertex(ps,node.v,node.vertices.size());};
	return flatMap(solutions,func);
}

//
// Join
//

#ifndef USE_CC_FAST_JOIN

inline CCPartialSolutions join(CCPartialSolution const & solution_a, CCPartialSolution const & solution_b, int k) {
	CCPartialSolution new_solution;
	new_solution.poly = polyMul(solution_a.poly, solution_b.poly);

	for (int v = 0; v < k; v++) {
		int a_type = getType(solution_a.types,v);
		int b_type = getType(solution_b.types,v);

		if (a_type == NN or b_type == NN)
			setType(new_solution.types,v,a_type+b_type);
		else if (a_type == BT or b_type == BT or a_type != b_type)
			return CCPartialSolutions();
		else
			setType(new_solution.types,v,BT);
	}

	return CCPartialSolutions({new_solution});
}

CCPartialSolutions basicJoin(TWNode const & node, CCPartialSolutions && solutions_a, CCPartialSolutions && solutions_b) {
	CCPartialSolutions result;
	for (auto solution_a : solutions_a)
		for (auto solution_b : solutions_b) {
			auto new_solutions = join(solution_a,solution_b,node.vertices.size());
			result.insert(result.begin(),new_solutions.begin(),new_solutions.end());
		}
	return result;
}

typedef size_t coloring_t;

inline unsigned long long join(unsigned long long coloring_a, unsigned long long coloring_b, int k) {
	unsigned long long result = 0l;

	for (int v = 0; v < k; v++) {
		int a_type = getType(coloring_a,v);
		int b_type = getType(coloring_b,v);

		if (a_type == 0 or b_type == 0)
			setType(result,v,a_type+b_type);
		else
			setType(result,v,BT);
	}

	return result;
}

pair<coloring_t,coloring_t> coloringToTwo(coloring_t coloring, size_t k) {
	coloring_t coloring_a=0l, coloring_b=0l;

	for (size_t i = 0l; i < k; i++) {
		size_t color = coloring%9l;
		coloring /= 9l;

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
			case 6 :
				coloring_a += 0l;
				coloring_b += 3l;
				break;
			case 7 :
				coloring_a += 3l;
				coloring_b += 0l;
				break;
			case 8 :
				coloring_a += 3l;
				coloring_b += 3l;
				break;
		}
	}

	return mp(coloring_a,coloring_b);
}

CCPartialSolutions join(TWNode const & node, CCPartialSolutions && solutions_a, CCPartialSolutions && solutions_b) {
	size_t k = node.vertices.size();

	if (solutions_a.size()*solutions_b.size() < 32*pow(9l,k))
		return basicJoin(node,move(solutions_a),move(solutions_b));

	map<coloring_t,unsigned long long> map_a;
	for (auto p : solutions_a)
		map_a[p.types] = p.poly;

	map<coloring_t,unsigned long long> map_b;
	for (auto p : solutions_b)
		map_b[p.types] = p.poly;

	map<coloring_t,unsigned long long> map_result;
	for (size_t coloring = 0l; coloring < pow(9l,k); coloring++) {
		auto colorings = coloringToTwo(coloring,k);
		if (map_a.count(colorings.first) == 0 or map_b.count(colorings.second) == 0)
			continue;

		auto result_coloring = join(colorings.first,colorings.second,k);
		auto result_poly = polyMul(map_a[colorings.first],map_b[colorings.second]);

		if (map_result.count(result_coloring) == 0)
			map_result[result_coloring] = result_poly;
		else
			map_result[result_coloring] ^= result_poly;
	}


	CCPartialSolutions result;
	for (auto solution : map_result)
		result.push_back(CCPartialSolution(solution.first,solution.second));

	return result;
}

#else

typedef vector<complex<long long>> CCPoly;
typedef vector<CCPoly> CCFunction;

//
// Common
//

size_t coloringSize(unsigned long long coloring, int k) {
	size_t result = 0l;

	for (int i = 1; i <= k; i++) {
		auto color = coloring % 4l;
		coloring = coloring >> 2l;

		if (color == LF or color == RT)
			result += 1l;
		else if (color == BT)
			result += 2l;
	}

	return result;
}

//
// FFT
//

inline CCPoly vecPolySMul(CCPoly poly, complex<long long> n) {
	for (auto & a : poly)
		a *= n;
	return poly;
}

inline CCPoly vecPolyAdd(CCPoly A, CCPoly const & B) {
	for (size_t i = 0l; i < A.size(); i++)
		A[i] += B[i];
	return A;
}

CCFunction fft(CCFunction && result, size_t k) {
	complex<long long> eps(0,1);
	for (int pos = 0; pos < (int)k; pos++) {
		int base = (1<<(pos<<1));
		int real_pos = (pos<<1);
		int suf_base = base<<2;

		for (int sufix = 0; sufix < (1<<(k<<1)); sufix += suf_base)
			for (int prefix = 0; prefix < base; prefix++) {
				CCFunction tmp{
					result[prefix + (0<<real_pos) + sufix],
					result[prefix + (1<<real_pos) + sufix],
					result[prefix + (2<<real_pos) + sufix],
					result[prefix + (3<<real_pos) + sufix]
				};

				for (int t = 0; t <= 3; t++)
					result[prefix + (t<<real_pos) + sufix] = 
						vecPolyAdd(
							vecPolyAdd(
								vecPolySMul(tmp[0],pow(eps,(t*0)%4)),
								vecPolySMul(tmp[1],pow(eps,(t*1)%4))
							), vecPolyAdd(
								vecPolySMul(tmp[2],pow(eps,(t*2)%4)),
								vecPolySMul(tmp[3],pow(eps,(t*3)%4))
							)
						);
			}
	}

	return result;
}

inline vector<CCFunction> fftAll(vector<CCFunction> && functions, size_t k) {
	for (auto & f : functions)
		f = fft(move(f),k);
	return functions;
}

//
// To Divided Function
//

CCPoly toVecPoly(unsigned long long poly) {
	CCPoly result(128,0);
	for (int i = 0; i < 64; i++) {
		result[i] = poly%2;
		poly >>= 1;
	}
	return result;
}

CCFunction toCCFunction(CCPartialSolutions const & solutions, size_t f_size) {
	CCFunction result(f_size,CCPoly(128,0));
	
	for (auto solution : solutions)
		result[solution.types] = toVecPoly(solution.poly);

	return result;
}

vector<CCFunction> toFunctionsBySize(CCPartialSolutions && solutions, size_t k, size_t f_size) {
	vector<CCPartialSolutions> divided_solutions(2*k+1);
	for (auto solution : solutions)
		divided_solutions[coloringSize(solution.types,k)].push_back(solution);

	vector<CCFunction> result;
	for (auto solutions : divided_solutions)
		result.push_back(toCCFunction(move(solutions),f_size));

	return result;
}

//
// To Solution
//

unsigned long long minusColors(unsigned long long colors) {
	unsigned long long pos = 1;

	while (pos <= colors) {
		if (colors&pos)
			colors = colors^(pos<<1);
		pos = pos << 2;
	}

	return colors;
}

vector<long long> reduceVecPoly(CCPoly && vec_poly, unsigned int k) {
	vector<long long> result;

	for (auto & a : vec_poly)
		result.push_back(a.real() >> k);

	return result;
}

unsigned long long toSimplePoly(vector<long long> vec_poly) {
	unsigned long long low=0l, high=0l;
	for(int i = 63; i >= 0; i--) {
		low <<= 1;
		low += vec_poly[i]%2l;
		high <<= 1;
		high += vec_poly[i+64]%2l;
	}
	return reduceOnly(low,high);
}

CCFunction addFuncs(CCFunction const & func_a, CCFunction const & func_b, size_t func_size) {
	CCFunction result(func_size);

	for (size_t i = 0l; i < func_size; i++)
		result[i] = vecPolyAdd(func_a[i],func_b[i]);

	return result;
}

CCPartialSolutions mergeToResult(map<pair<size_t,size_t>,CCFunction> && functions, size_t k, size_t f_size) {
	CCPartialSolutions result;

	for (unsigned long long coloring = 0; coloring < f_size; coloring++) {
		size_t s = coloringSize(coloring,k);
		unsigned long long poly = 0l;

		for (size_t s1 = 0l; s1 <= s; s1++)
			poly ^= toSimplePoly(reduceVecPoly(move(functions[mp(s1,s-s1)][coloring]),2*k));

		if (poly != 0l)
			result.push_back(CCPartialSolution(minusColors(coloring),poly));
	}

	return result;	
}

//
// The Join
//

CCPoly vecPolyMul(CCPoly & A, CCPoly & B) {
	CCPoly result(128,0l);
	for (int i = 0; i < 64; i++)
		for (int j = 0; j < 64; j++)
			result[i+j] += A[i]*B[j];
	return result;
}

CCFunction mulFunctions(CCFunction && A, CCFunction && B) {
	for (size_t i = 0l; i < A.size(); i++)
		A[i] = vecPolyMul(A[i],B[i]);
	return A;
}

CCPartialSolutions join(TWNode const & node, CCPartialSolutions && solutions_a, CCPartialSolutions && solutions_b) {
	size_t k = node.vertices.size();
	size_t f_size = 1<<(k<<1);

	vector<CCPartialSolutions> divided_solutions_a(2*k+1);
	for (auto solution : solutions_a)
		divided_solutions_a[coloringSize(solution.types,k)].push_back(solution);

	vector<CCPartialSolutions> divided_solutions_b(2*k+1);
	for (auto solution : solutions_b)
		divided_solutions_b[coloringSize(solution.types,k)].push_back(solution);

	map<unsigned long long,unsigned long long> result;

	for (size_t s1 = 0l; s1 <= 2*k; s1++)
		for (size_t s2 = 0l; s1+s2 <= 2*k; s2++) {
			auto func = fft(mulFunctions(
				fft(toCCFunction(divided_solutions_a[s1],f_size),k),
				fft(toCCFunction(divided_solutions_b[s2],f_size),k)
			),k);
			size_t s = s1 + s2;

			for (unsigned long long coloring = 0; coloring < f_size; coloring++)
				if (s == coloringSize(coloring,k)) {
					auto poly = toSimplePoly(reduceVecPoly(move(func[coloring]),2*k));
					if (poly != 0l) {
						if (result.count(coloring) == 0)
							result[coloring] = poly;
						else
							result[coloring] ^= poly;
					}
				}
		}

	CCPartialSolutions vec_result;
	for (auto p : result)
		vec_result.push_back(CCPartialSolution(minusColors(p.first),p.second));

	return vec_result;	
}

#endif

//
// Processing nodes
//

CCPartialSolutions HCSolver::process_node(int root, vector<TWNode> const & nodes) {

	vector<CCPartialSolutions> partial_solutions;

	for (auto child : nodes[root].children)
		partial_solutions.push_back(process_node(child, nodes));

	CCPartialSolutions result;

	switch (nodes[root].type) {
		case ADD_VERTEX :
			result = addVertex(nodes[root],move(partial_solutions[0]));
			break;
		case ADD_EDGE :
			result = addEdge(nodes[root],partial_solutions[0]);
			break;
		case DEL_VERTEX :
			result = delVertex(nodes[root],partial_solutions[0]);
			break;
		case JOIN :
			result = join(nodes[root],move(partial_solutions[0]),move(partial_solutions[1]));
			break;
		case LEAF :
			return CCPartialSolutions{CCPartialSolution()};
		default :
			return partial_solutions[0];

	}

	return reduction(move(result),(int)nodes[root].vertices.size());
}

//
// Constructor
//

HCSolver::HCSolver(function<CCPartialSolutions(CCPartialSolutions,int)> r) :
	reduction(r)
{}

//
// Getting result
//

bool HCSolver::hasHC(vector<TWNode> const & nodes) {
	initRandomPolys();
	CCPartialSolutions solutions = process_node(0, nodes);

	return (solutions.size() != 0 and solutions[0].poly != 0l);
}

pair<bool,vector<int>> HCSolver::getHC(vector<TWNode> & nodes, vector<vector<int>> & graph, set<pair<int,int>> & edges, int n) {
	if (not hasHC(nodes))
		return mp(false,vector<int>());

	EdgeSet selected_edges = edge_selection::simpleSelection(graph,edges,[&](EdgeSet deleted, EdgeSet chosen){
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


CCPartialSolutions simple_reduction(CCPartialSolutions && result, size_t k) {
	if (result.size() <= 1)
		return result;

	sort(result.begin(),result.end());
	CCPartialSolutions new_result;

	for (int i = 0; i <= ((int)result.size())-2; i++)
		if (result[i] != result[i+1]) {
			if (result[i].poly != 0l)
				new_result.push_back(result[i]);
		} else {
			result[i+1].poly ^= result[i].poly;
		}
	if (result[result.size()-1].poly != 0l)
		new_result.push_back(result[result.size()-1]);

	return new_result;
}

