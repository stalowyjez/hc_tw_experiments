#include <cstdio>
#include <vector>
#include <set>
#include <queue>

#include "basic.h"
#include "edge_selection.h"

using namespace std;

namespace edge_selection {

//
// Simple selection
//

	EdgeSet simpleSelection(Graph graph, EdgeSet edges, HCCheckFunc check_func) {
		EdgeSet result, deleted_edges;

		vector<int> additional_edges(graph.size(),-2);
		vector<int> needed_edges(graph.size(),2);

		for (auto e : edges) {
			additional_edges[e.first]++;
			additional_edges[e.second]++;
		}

		for (auto e : edges) {
			deleted_edges.insert(e);
			if (
				needed_edges[e.first] != 0 and needed_edges[e.second] != 0 and
				(additional_edges[e.first] == 0 or additional_edges[e.second] == 0 or not check_func(deleted_edges,result))
			) {
				deleted_edges.erase(e);
				result.insert(e);

				needed_edges[e.first]--;
				needed_edges[e.second]--;
			} else {
				additional_edges[e.first]--;
				additional_edges[e.second]--;
			}
		}

		return result;
	}

//
// More complex one
//

	class EdgeSelectionHelper {
		private:
			EdgeSet chosen_edges;
			EdgeSet deleted_edges;

			Graph graph;
			EdgeSet edges;

			vector<int> max_deg;
			vector<int> actual_deg;

			int n;

			void deleteEdge(pair<int,int>);
			void chooseEdge(pair<int,int>);
			void repair();
			void repair(int,queue<int>&,vector<bool>&);

		public:
			EdgeSelectionHelper(Graph,EdgeSet);
			pair<EdgeSet,EdgeSet> getSets();
			void setSets(EdgeSet,EdgeSet);
			bool isFinished();
			int bestVertex();
			int getDeg(int);
			EdgeSet getSolution();
	};

	void EdgeSelectionHelper::deleteEdge(pair<int,int> edge) {
		max_deg[edge.first]--;
		max_deg[edge.second]--;
		deleted_edges.insert(edge);
	}

	void EdgeSelectionHelper::chooseEdge(pair<int,int> edge) {
		actual_deg[edge.first]++;
		actual_deg[edge.second]++;
		chosen_edges.insert(edge);
	}

	void EdgeSelectionHelper::repair() {
		vector<bool> is_on_q(n+1,true);
		queue<int> Q;
		for (int v = 1; v <= n; v++)
			Q.push(v);

		while (not Q.empty()) {
			int v = Q.front();
			Q.pop();
			is_on_q[v] = false;

			repair(v,Q,is_on_q);
		}
	}

	void EdgeSelectionHelper::repair(int v, queue<int> & Q, vector<bool> & is_on_q) {
		if (max_deg[v] > 2 and actual_deg[v] == 2)
			for (int w : graph[v])
				if (deleted_edges.count(edge(v,w)) == 0 and chosen_edges.count(edge(v,w)) == 0) {
					deleteEdge(edge(v,w));
					if (not is_on_q[w]) {
						Q.push(w);
						is_on_q[w] = true;
					}
				}
		if (max_deg[v] == 2 and actual_deg[v] < 2)
			for (int w : graph[v])
				if (deleted_edges.count(edge(v,w)) == 0 and chosen_edges.count(edge(v,w)) == 0) {
					chooseEdge(edge(v,w));
					if (not is_on_q[w]) {
						Q.push(w);
						is_on_q[w] = true;
					}
				}
	}

	EdgeSelectionHelper::EdgeSelectionHelper(Graph _graph, EdgeSet _edges) {
		graph = _graph;
		edges = _edges;

		n = graph.size()-1;

		chosen_edges = EdgeSet();
		deleted_edges = EdgeSet();
		actual_deg = vector<int>(n+1,0);
		max_deg = vector<int>(n+1,0);

		for (int v = 1; v <= n; v++)
			max_deg[v] = graph[v].size();

		repair();
	}

	pair<EdgeSet,EdgeSet> EdgeSelectionHelper::getSets() {
		return mp(deleted_edges,chosen_edges);
	}

	void EdgeSelectionHelper::setSets(EdgeSet deleted, EdgeSet chosen) {
		for (auto edge : deleted)
			if (deleted_edges.count(edge) == 0)
				deleteEdge(edge);

		for (auto edge : chosen)
			if (chosen_edges.count(edge) == 0)
				chooseEdge(edge);

		repair();
	}

	bool EdgeSelectionHelper::isFinished() {
		return n == (int)chosen_edges.size();
	}

	//
	// Here is place for experiments
	//

	int EdgeSelectionHelper::bestVertex() {
		int result = -1;
		int deg = 0;
		int val = 0;

		for (int v = 1; v <= n; v++)
			if ((deg < actual_deg[v] and actual_deg[v] < 2) or (deg == actual_deg[v] and val < max_deg[v])) {
				result = v;
				deg = actual_deg[v];
				val = max_deg[v];
			}

		return result;
	}

	int EdgeSelectionHelper::getDeg(int v) {
		return actual_deg[v];
	}

	EdgeSet EdgeSelectionHelper::getSolution() {
		return chosen_edges;
	}

	pair<int,int> pickNecessary(EdgeSet & deleted, EdgeSet const & chosen, vector<pair<int,int>> available, HCCheckFunc const & check_func) {
		if (available.size() == 1)
			return available[0];

		vector<pair<int,int>> first_half,second_half;
		for (int i = 0; i <= (int)available.size()/2-1; i++)
			first_half.push_back(available[i]);
		for (int i = (int)available.size()/2; i < (int)available.size(); i++)
			second_half.push_back(available[i]);

		auto tmp_deleted = deleted;
		for (auto el : second_half)
			tmp_deleted.insert(el);

		if (check_func(tmp_deleted,chosen)) {
			for (auto el : second_half)
				deleted.insert(el);
			return pickNecessary(deleted,chosen,first_half,check_func);
		} else {
			for (auto el : first_half)
				deleted.insert(el);
			return pickNecessary(deleted,chosen,second_half,check_func);
		}
	}

	pair<int,int> pickNecessaryFirstMove(EdgeSet deleted, EdgeSet chosen, vector<pair<int,int>> available, HCCheckFunc const & check_func) {
		if (available.size() == 1)
			return available[0];

		vector<pair<int,int>> first_half,second_half;
		for (int i = 0; i <= (int)available.size()/2-1; i++)
			first_half.push_back(available[i]);
		for (int i = (int)available.size()/2; i < (int)available.size(); i++)
			second_half.push_back(available[i]);

		auto tmp_deleted = deleted;
		for (auto el : second_half)
			tmp_deleted.insert(el);

		if (check_func(tmp_deleted,chosen))
			return pickNecessaryFirstMove(tmp_deleted,chosen,first_half,check_func);
		else
			return pickNecessaryFirstMove(deleted,chosen,second_half,check_func);
	}

	EdgeSet syphilisSelection(Graph graph, EdgeSet edges, HCCheckFunc check_func) {
		EdgeSelectionHelper helper(graph,edges);

		while (not helper.isFinished()) {
			auto p = helper.getSets();

			auto deleted = p.first;
			auto chosen = p.second;

			int v = helper.bestVertex();

			vector<pair<int,int>> as_edges;
			for (int w : graph[v])
				if (chosen.count(edge(v,w)) == 0 and deleted.count(edge(v,w)) == 0)
					as_edges.push_back(edge(v,w));

			if (helper.getDeg(v) == 0)
				chosen.insert(pickNecessaryFirstMove(deleted,chosen,as_edges,check_func));
			else
				chosen.insert(pickNecessary(deleted,chosen,as_edges,check_func));

			helper.setSets(deleted,chosen);
		}

		return helper.getSolution();
	}
}
