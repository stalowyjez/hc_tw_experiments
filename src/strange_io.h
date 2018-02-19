#include <algorithm>
#include <vector>
#include <cstdlib>
#include <cstdio>

#include "treewidth.h"

#ifndef STRANGE_IO

#define STRANGE_IO

std::pair<std::set<std::pair<int,int>>,std::vector<std::vector<int>>> readGraph(FILE*);
std::pair<std::set<std::pair<int,int>>,std::vector<std::vector<int>>> readGraph();
std::pair<std::set<std::pair<int,int>>,std::vector<std::vector<int>>> readHCPGraph(FILE*);
std::pair<std::set<std::pair<int,int>>,std::vector<std::vector<int>>> readHCPGraph();

void printDecomposition(OrdinaryDecomposition decomposition, int n);
void printDecomposition(std::vector<TWNode> nodes, int n);
void printDecompositionDebug(std::vector<TWNode> nodes);

OrdinaryDecomposition readTD();

#endif

