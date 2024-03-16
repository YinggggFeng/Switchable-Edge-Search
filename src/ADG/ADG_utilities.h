#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <cassert>

#include "../graph/graph.h"
#include "../types.h"
#include <random>


using namespace std;

int get_agentCnt(ADG &adg);

int get_stateCnt(ADG &adg, int agent);

int get_totalStateCnt(ADG &adg);

int compute_vertex(vector<int> &accum_stateCnts, int agent, int state);

pair<int, int> compute_agent_state(vector<int> &accum_stateCnts, int v);

int compute_start_vertex(vector<int> &accum_stateCnts, int v);

// Directed
bool is_type2_edge(ADG &adg, int agent1, int state1, int agent2, int state2);

void fix_type2_edge(ADG &adg, int agent1, int state1, int agent2, int state2);

void fix_type2_edge_reversed(ADG &adg, int agent1, int state1, int agent2, int state2);

void shift(ADG &adg, int agent1, int state1, int agent2, int state2);

// Return an vector of agent-state pairs
vector<pair<int, int>> get_switchable_inNeibPair(ADG &adg, int agent, int state);

vector<pair<int, int>> get_switchable_outNeibPair(ADG &adg, int agent, int state);

vector<pair<int, int>> get_nonSwitchable_inNeibPair(ADG &adg, int agent, int state);

vector<pair<int, int>> get_nonSwitchable_outNeibPair(ADG &adg, int agent, int state);

vector<pair<int, int>> get_inNeibPair(ADG &adg, int agent, int state);

vector<pair<int, int>> get_outNeibPair(ADG &adg, int agent, int state);

Location get_state_target(ADG &adg, int agent, int state);

ADG copy_ADG(ADG &adg);

// Run DFS
bool detectCycle(ADG &adg, int agent, int state);

void free_underlying_graph(ADG &adg);

void free_shared_graph(ADG &adg);
