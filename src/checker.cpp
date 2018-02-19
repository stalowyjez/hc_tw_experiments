#include <cstdio>
#include <set>
#include <algorithm>
#include <vector>

#include "basic.h"

using namespace std;

int main() {
	int n, m;
	scanf("%d %d", &n, &m);

	vector<vector<int>> Graph(n+1);
	set<pair<int,int>> edges;

	for (int i = 1; i <= m; i++) {
		int A, B;
		scanf("%d %d", &A, &B);

		Graph[A].push_back(B);
		Graph[B].push_back(A);

		edges.insert(edge(A,B));
	}

	set<int> vertices;
	vector<int> hc;
	for (int i = 1; i <= n; i++) {
		int v;
		scanf("%d", &v);

		hc.push_back(v);
		vertices.insert(v);
	}

	bool good = true;

	//printf("%d %d\n", n, vertices.size());

	if (vertices.size() != n)
		good = false;

	for (int i = 0; i < n; i++)
		if (edges.count(edge(hc[i],hc[(i+1)%n])) == 0) {
			//printf("DO NOT HAVE EDGE: %d %d\n", hc[i], hc[(i+1)%n]);
			good = false;
		}

	if (good)
		printf("1\n");
	else
		printf("0\n");
}
