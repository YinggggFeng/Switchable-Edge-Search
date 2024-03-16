#include <new>
#include <stdexcept>
#include <iostream>

#include "../types.h"

using namespace std;

Graph new_graph(int n);

void set_type1_edge(Graph& graph, int n1, int n2);

void set_type2_nonSwitchable_edge(Graph& graph, int n1, int n2);

void set_type2_switchable_edge(Graph& graph, int n1, int n2);

// Cannot imagine when we'll need this but still have it here
void rem_type1_edge(Graph& graph, int n1, int n2);

// Also shouldn't need this right?
void rem_type2_nonSwitchable_edge(Graph& graph, int n1, int n2);

void rem_type2_switchable_edge(Graph& graph, int n1, int n2);

void rem_type2_nonSwitchable_neighborhood(Graph& graph, int n);

bool get_type1_edge(Graph& graph, int n1, int n2);

bool get_type2_nonSwitchable_edge(Graph& graph, int n1, int n2);

bool get_type2_switchable_edge(Graph& graph, int n1, int n2);

bool get_edge(Graph& graph, int n1, int n2);

set<int> get_nonSwitchable_outNeib(Graph& graph, int n);

set<int> get_nonSwitchable_inNeib(Graph& graph, int n);
void set_switchable_nonSwitchable(Graph& graph);

// Note that a reference type should not be used to recieve output if a copy is
// desired. If a copy is not needed reference is returned to reduce copying
set<int>& get_switchable_outNeib(Graph& graph, int n);

// Note that a reference type should not be used to recieve output if a copy is
// desired. If a copy is not needed reference is returned to reduce copying
set<int>& get_switchable_inNeib(Graph& graph, int n);

set<int>& get_type2_nonSwitchable_inNeib(Graph& graph, int n);

set<int> get_outNeighbors(Graph& graph, int n);

set<int> get_inNeighbors(Graph& graph, int n);

void set_switchable_nonSwitchable(Graph& graph);

Graph copy_graph(Graph& graph);

void free_graph(Graph& graph);

void free_nonSwitchable(Graph& graph);

void print_graph(Graph& graph);
void print_graph_n2(Graph& graph);
void print_graph_s2(Graph& graph);

bool check_cycle_nonSwitchable(Graph& graph, int start);

bool check_cycle_dfs(Graph& graph, int start);

void remove_all_switchable_edges(Graph& graph);

bool check_cycle_nonSwitchable_old(Graph& graph, int start);

// vector<int>* topologicalSort(Graph& graph, vector<int> starts);

sortResult topologicalSort(Graph& graph, sortResult state, vector<int>* agent_starts, int u, int v);