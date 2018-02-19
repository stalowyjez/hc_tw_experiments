#include "strange_io.h"
#include "treewidth.h"
#include "hc_solver.h"

using namespace std;

int main(int argc, char * argv[]) {
	FILE * file_pointer = fopen(argv[1], "r");
	auto p  = readHCPGraph(file_pointer);
	fclose(file_pointer);

	auto edges = p.first;
	auto G = p.second;

	int n = G.size()-1;
	int m = edges.size();

	printf("%d %d\n", n, m);
	for (auto e : edges)
		printf("%d %d\n", e.first, e.second);

	auto ordinary_decomposition = readTD();
	setGraphAndEdges(G,edges);
	auto decomposition = makeDecompositionNice(ordinary_decomposition);

	auto result = solve(decomposition,G,edges,n);

	if (result.first) {
		for (int v : result.second)
			printf("%d ", v);
		printf("\n");
	} else {
		printf("-1\n");
	}
}
