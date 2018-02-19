#include <cstdio>
#include <vector>
#include <list>
#include <algorithm>
#include <set>

#include "strange_io.h"
#include "basic.h"

using namespace std;

pair<set<pair<int,int>>,vector<vector<int>>> readGraph(FILE * input_file) {
	vector<vector<int>> Graph;
	set<pair<int,int>> edges;

	char BUFFER[1000000];
	char * result;
	char * begin = BUFFER;

	list<int> nums;

	bool actualNum = false;
	int num = 0;

	result = fgets(begin,1000000,input_file);
	while (result != NULL) {
		if (BUFFER[0] != 'c') {
			for (int i = 0; BUFFER[i] != '\0'; i++) {
				char c = BUFFER[i];

				if ('0' <= c and '9' >= c) {
					if (not actualNum) {
						actualNum = true;
						num = c - '0';
					} else {
						num *= 10;
						num += c - '0';
					}
				} else {
					if (actualNum) {
						nums.push_back(num);
						actualNum = false;
					}
				}
			}
		}
		result = fgets(begin,1000000,input_file);
	}
	
	int n = nums.front();
	nums.pop_front();
	int m = nums.front();
	nums.pop_front();

	for (int i = 0; i<=n; i++)
		Graph.push_back(vector<int>());

	for (int i = 1; i<=m; i++) {
		int A = nums.front();
		nums.pop_front();
		int B = nums.front();
		nums.pop_front();

		Graph[A].push_back(B);
		Graph[B].push_back(A);

		edges.insert(mp(A,B));
		edges.insert(mp(B,A));
	}

	return mp(edges,Graph);
}

pair<set<pair<int,int>>,vector<vector<int>>> readGraph() {
	return readGraph(stdin);
}

void dropLine() {
	printf("DROP LINE\n");
	int c = 'a';
	while (c != '\n')
		c = getchar();
	printf("END DROP LINE\n");
}

pair<int,char*> getNum(char * ptr) {
	// skipping non nums

	while (*ptr > '9' or *ptr < '0')
		ptr++;

	// reading nums
	
	int result = 0;

	while (*ptr <= '9' and *ptr >= '0') {
		char c = *ptr;
		result *= 10;
		result += c - '0';
		ptr++;
	}
		
	return mp(result,ptr);
}

pair<set<pair<int,int>>,vector<vector<int>>> readHCPGraph(FILE * file_pointer) {
	char BUFFER[1000000];
	char * begin = BUFFER;

	for (int i = 1; i <= 3; i++)
		fgets(begin,1000000,file_pointer);

	fgets(begin,1000000,file_pointer);
	auto p = getNum(begin);
	int n = p.first;

	for (int i = 1; i <= 2; i++)
		fgets(begin,1000000,file_pointer);
	
	vector<vector<int>> Graph(n+1);
	set<pair<int,int>> edges;

	while (true) {
		fgets(begin,1000000,file_pointer);

		if (BUFFER[0] == '-')
			break;

		auto ptr = BUFFER;

		auto p = getNum(ptr);
		int A = p.first;
		ptr = p.second;

		ptr++;

		p = getNum(ptr);
		int B = p.first;

		edges.insert(edge(A,B));
		Graph[A].push_back(B);
		Graph[B].push_back(A);
	}

	fgets(begin,1000000,file_pointer);

	return mp(edges,Graph);
}

pair<set<pair<int,int>>,vector<vector<int>>> readHCPGraph() {
	return readHCPGraph(stdin);
}

void printDecomposition(OrdinaryDecomposition decomposition, int n) {

	int width = 0;
	for (auto const & bucket : decomposition.vertices)
		width = max((int)bucket.size(),width);

	int num = decomposition.vertices.size()-1;

	printf("s td %d %d %d\n", num, width, n);

	for (int i = 1; i <= num; i++) {
		printf("b %d", i);
		for (int v : decomposition.vertices[i]) {
			printf(" %d", v);
		}
		printf("\n");
	}

	for (int node = 1; node <= num; node++)
		for (int neigh : decomposition.graph[node])
			if (neigh < node)
				printf("%d %d\n", node, neigh);
}

void printDecomposition(vector<TWNode> nodes, int n) {

	int width = 0;
	for (auto node : nodes) {
		if (width < ((int) node.vertices.size()))
			width = node.vertices.size();
	}

	int num = nodes.size();

	printf("s td %d %d %d\n", num, width, n);

	for (int i = 0; i < num; i++) {
		printf("b %d", i+1);
		for (int v : nodes[i].vertices) {
			printf(" %d", v);
		}
		printf("\n");
	}

	for (int i = 1; i < num; i++) {
		printf("%d %d\n", i+1, nodes[i].parent+1);
	}
}

void printDecompositionDebug(vector<TWNode> nodes) {
	char nodeTypeStr[6][20] = {"LEAF", "ADD_VERTEX", "ADD_EDGE", "DEL_VERTEX", "JOIN", "NONE"};

	for (int i = 0; i < ((int) nodes.size()); i++) {
		printf("NODE: %d ", i);
		printf("TYPE: %s ", nodeTypeStr[nodes[i].type]);
		printf("PARENT: %d ", nodes[i].parent);
		printf("VERTICES:");
		for (int v : nodes[i].vertices)
			printf(" %d", v);
		printf(" EDGES:");
		for (auto p : nodes[i].edges)
			printf(" (%d,%d) ", p.first, p.second);
		printf("CHILDREN:");
		for (int child : nodes[i].children)
			printf(" %d", child);
		if (nodes[i].type == ADD_VERTEX or nodes[i].type == DEL_VERTEX or nodes[i].type == ADD_EDGE)
			printf(" V: %d", nodes[i].v);
		if (nodes[i].type == ADD_EDGE)
			printf(" W: %d", nodes[i].w);
		printf("\n");
	}
}

vector<int> getNumsFromBuffer(char * BUFFER, int pos) {
	vector<int> result;

	int num;
	bool actualNum = false;

	for (; BUFFER[pos] != '\0'; pos++) {
		char c = BUFFER[pos];

		if ('0' <= c and '9' >= c) {
			if (not actualNum) {
				actualNum = true;
				num = c - '0';
			} else {
				num *= 10;
				num += c - '0';
			}
		} else {
			if (actualNum) {
				result.push_back(num);
				actualNum = false;
			}
		}
	}

	return result;
}

OrdinaryDecomposition readTD() {
	OrdinaryDecomposition result;

	char BUFFER[1000000];
	char * buf_res;
	char * begin = BUFFER;

	vector<int> nums;

	buf_res = fgets(begin,1000000,stdin);
	while (buf_res != NULL) {
		switch (BUFFER[0]) {
			case 'c' :
				break;
			case 's' :
				nums = getNumsFromBuffer(BUFFER,5);
				result.vertices.push_back(vector<int>());
				result.graph = vector<vector<int>>(nums[0]+1);
				break;
			case 'b' :
				nums = getNumsFromBuffer(BUFFER,2);
				nums.erase(nums.begin());
				result.vertices.push_back(nums);
				break;
			default :
				nums = getNumsFromBuffer(BUFFER,0);
				result.graph[nums[0]].push_back(nums[1]);
				result.graph[nums[1]].push_back(nums[0]);
		}
		buf_res = fgets(begin,1000000,stdin);
	}

	return result;
}
