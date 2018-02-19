#include <vector>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <functional>

#ifndef BASIC

#define BASIC 1

#define INF 1000000000
#define V 100001

#define MUL 5501
#define MOD 1230599

#define mp make_pair
#define pb bush_back

#define edge(a,b) (a>b?mp(b,a):mp(a,b))
#define getSecond(e,v) ((e).first == (v) ? (e).second : (e).first)

//
// long long as a set
//

#define ls_oneHot(x) (1l<<(x))
#define ls_contains(s,x) ((s)&(ls_oneHot(x)))
#define ls_erase(s,x) s=(s&(~(ls_oneHot(x))))
#define ls_insert(s,x) s=(s|(ls_oneHot(x)))
#define ls_incNumNew(s) s=((s<<1l)+1l)
#define ls_incNum(s) s=(s<<1l)

#define getBit(n,i) ((n>>i)%2)

//
// vector as set functions
//

bool isDiffEmpty(std::vector<int> & A, std::vector<int> & B);
int firstInDiff(std::vector<int> & A, std::vector<int> & B);

template <class T>
std::vector<T> withElement(std::vector<T> A, T element) {
	std::vector<T> result;
	for (T v : A)
		result.push_back(v);
	result.push_back(element);
	return result;
}

template <class T>
std::vector<T> withoutElement(std::vector<T> A, T element) {
	std::vector<T> result;
	for (T v : A) if (v != element)
		result.push_back(v);
	return result;
}

std::vector<int> invPerm(std::vector<int> const & perm);

template<class T>
std::vector<T> setDiff(std::set<T> A, std::set<T> B) {
	std::vector<T> result;

	auto ptrB = B.begin();

	for (T a : A) {
		while (ptrB != B.end() and a > *ptrB)
			ptrB++;
		if (ptrB == B.end() or *ptrB > a)
			result.push_back(a);
	}

	return result;
}

//
// flatMap
//

/*template<class S>
void flatMap(std::vector<S> & objs, std::function<std::vector<S>(S)> conversion) {
	int initial_size = objs.size();
	int ptr = 0;

	for (int i = 0; i < initial_size; i++) {
		auto res = conversion(objs[i]);

		if (res.size() > 0) {
			int res_ptr = 0;

			while (ptr <= i and res_ptr < res.size()) {
				objs[ptr] = res[res_ptr];
				ptr++;
				res_ptr++;
			}

			while (res_ptr < res.size()) {
				objs.push_back(res[res_ptr]);
				res_ptr++;
			}
		}
	}
}*/

template<class S, class T>
std::vector<T> flatMap(std::vector<S> objs, std::function<std::vector<T>(S)> conversion) {
	std::vector<T> result;
	for (S s : objs)
		for (T t : conversion(s))
			result.push_back(t);
	return result;
}

typedef std::set<std::pair<int,int>> EdgeSet;
typedef std::vector<std::vector<int>> Graph;

#endif

