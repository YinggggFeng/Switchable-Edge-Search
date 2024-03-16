#include "simulator.h"

Simulator::Simulator(ADG input_adg) {
  adg = input_adg;
  vector<int> init_states(get_agentCnt(adg), 0);
  states = init_states;
}

Simulator::Simulator(ADG input_adg, vector<int> visited_states) {
  adg = input_adg;
  states = visited_states;
}

int Simulator::checkMovable(vector<int>& movable) {
  int timeSpent = 0;
  int agentCnt = get_agentCnt(adg);
  for (int agent = 0; agent < agentCnt; agent++) {
    int state = states[agent];
    if (state >= get_stateCnt(adg, agent) - 1) {
      continue;
    }
    timeSpent += 1;
    int next_state = state + 1;

    vector<pair<int, int>> dependencies = get_nonSwitchable_inNeibPair(adg, agent, next_state);
    movable[agent] = 1;
    for (pair<int, int> dependency: dependencies) {
      int dep_agent = get<0>(dependency);
      int dep_state = get<1>(dependency);
      
      if (dep_agent != agent) {
        if (dep_state > states[dep_agent]) {
          movable[agent] = 0;
          break;
        }
      }
    }
  }
  return timeSpent;
}

bool Simulator::incident_to_switchable(int *v_from, int *v_to) {
  int agentCnt = get_agentCnt(adg);
  Graph &graph = get<0>(adg);
  for (int agent = 0; agent < agentCnt; agent++) {
    int state = states[agent];
    if (state >= get_stateCnt(adg, agent) - 1) continue;

    state += 1;
    set<int>& inNeib = get_switchable_inNeib(graph, compute_vertex(get<2>(adg), agent, state));
    for (auto it = inNeib.begin(); it != inNeib.end(); it++) {
      int from = *it;
      *v_from = from;
      *v_to = compute_vertex(get<2>(adg), agent, state);
      return true;
    }

    if (state >= get_stateCnt(adg, agent) - 1) continue;
    state += 1;
    set<int>& outNeib = get_switchable_outNeib(graph, compute_vertex(get<2>(adg), agent, state));
    for (auto it = outNeib.begin(); it != outNeib.end(); it++) {
      int to = *it;
      *v_to = to;
      *v_from = compute_vertex(get<2>(adg), agent, state);
      return true;
    }
  }
  return false;
}

int Simulator::checkMovable(vector<int>& movable, vector<int>& haventStop) {
  int timeSpent = 0;
  int agentCnt = get_agentCnt(adg);
  for (int agent = 0; agent < agentCnt; agent++) {
    int state = states[agent];
    if (state >= get_stateCnt(adg, agent) - 1) {
      continue;
    }
    timeSpent += 1;
    int next_state = state + 1;

    vector<pair<int, int>> dependencies = get_nonSwitchable_inNeibPair(adg, agent, next_state);
    movable[agent] = 1;
    haventStop[agent] = 1;
    for (pair<int, int> dependency: dependencies) {
      int dep_agent = get<0>(dependency);
      int dep_state = get<1>(dependency);
      
      if (dep_agent != agent) {
        if (dep_state > states[dep_agent]) {
          movable[agent] = 0;
          break;
        }
      }
    }
  }
  return timeSpent;
}

int Simulator::step(bool switchCheck) {
  int agentCnt = get_agentCnt(adg);
  vector<int> movable(agentCnt, 0);
  int timeSpent = checkMovable(movable);
  int moveCnt = 0;

  for (int agent = 0; agent < agentCnt; agent++) {
    if (movable[agent] == 1) {
      states[agent] += 1;
      moveCnt += 1;
    }
  }
  if (moveCnt == 0 && timeSpent != 0) {
    std::cout << "err\n";
    exit(0);
  }
  return timeSpent;
}

void Simulator::print_location(ofstream &outFile, Location location) {
  int i = get<0>(location);
  int j = get<1>(location);
  outFile << "(" << i << "," << j << ")->";
}

int Simulator::print_soln(const char* outFileName) {
  ofstream outFile;
  outFile.open(outFileName);
  int totalSpend = 0;
  int stepSpend = 0;

  if (outFile.is_open()) {
    vector<vector<Location>> expanded_paths;
    int agentCnt = get_agentCnt(adg);
    Paths &paths = get<1>(adg);

    for (int agent = 0; agent < agentCnt; agent ++) {
      vector<Location> expanded_path;
      expanded_path.push_back(get<0>((paths[agent])[0]));
      expanded_paths.push_back(expanded_path);
    }

    stepSpend = step(false);
    while (stepSpend != 0) {
      outFile << "step=" << stepSpend << "\n";
      for (int agent = 0; agent < agentCnt; agent ++) {
        Location new_location = get<0>((paths[agent])[(states[agent])]);
        if (!((same_locations(new_location, (expanded_paths[agent]).back())) && 
            ((size_t)(states[agent]) == (paths[agent]).size() - 1))) {
          (expanded_paths[agent]).push_back(new_location);
        }
      }
      totalSpend += stepSpend;
      stepSpend = step(false);
    }

    for (int agent = 0; agent < agentCnt; agent ++) {
      outFile << "Agent " << agent << ": ";
      vector<Location> &expanded_path = expanded_paths[agent];
      for (Location location: expanded_path) {
        print_location(outFile, location);
      }
      outFile << std::endl;
    }
    outFile.close();
  }

  return totalSpend;
}

int Simulator::print_soln() {
  int totalSpend = 0;
  int stepSpend = 0;

  stepSpend = step(false);
  while (stepSpend != 0) {
    totalSpend += stepSpend;
    stepSpend = step(false);
  }
  
  return totalSpend;
}