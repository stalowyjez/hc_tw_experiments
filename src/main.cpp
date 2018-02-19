#include "hc_solver.h"
#include "strange_io.h"

using namespace std;

int main() {
	auto p  = readHCPGraph();

	auto edges = p.first;
	auto G = p.second;

	int n = G.size()-1;
	int m = edges.size();

	printf("%d %d\n", n, m);
	for (auto e : edges)
		printf("%d %d\n", e.first, e.second);

	auto decomposition = decomposeNicely(n, G, edges);

	auto result = solve(decomposition,G,edges,n);

	if (result.first) {
		for (int v : result.second)
			printf("%d ", v);
		printf("\n");
	} else {
		printf("-1\n");
	}
}
