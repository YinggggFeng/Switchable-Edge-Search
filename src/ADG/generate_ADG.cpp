#include "generate_ADG.h"

bool same_locations(Location location1, Location location2) {
  int i1 = location1.first;
  int j1 = location1.second;
  int i2 = location2.first;
  int j2 = location2.second;
  
  return (i1 == i2 && j1 == j2);
}

// Return path and stateCnt of an agent
tuple<Path, int> parse_path(string line) {
  int i, j, stateCnt = 0;
  int time = 0;
  size_t comma_pos, leftPar_pos, rightPar_pos;
  Path path;
  Location prev_location = make_pair(-1, -1);

  while ((leftPar_pos = line.find("(")) != string::npos) {
    // Process an index pair
    comma_pos = line.find(",");
    i = stoi(line.substr(leftPar_pos + 1, comma_pos));
    rightPar_pos = line.find(")");
    j = stoi(line.substr(comma_pos + 1, rightPar_pos));
    line.erase(0, rightPar_pos + 1);

    // Create a location tuple and add it to the path
    Location location = make_pair(i, j);
    if (!same_locations(location, prev_location)) {
      stateCnt ++;
      path.push_back(make_pair(location, time));
      prev_location = location;
    }
    time++;
  }
  return make_tuple(path, stateCnt);
}

// Return all paths, accumulated counts of states, and States
tuple<Paths, vector<int>> parse_soln(char* fileName) {
  Paths paths;
  vector<int> accum_stateCnts;
  int sumStates = 0;

  string fileName_string = fileName;
  ifstream file(fileName_string);
  if (file.is_open()) {
    string line;
    while (getline(file, line)) {
      // Sanity check that the line is a path
      if (line[0] == 'A') {
        tuple<Path, int> parse_result = parse_path(line);
        Path path = get<0>(parse_result);
        int stateCnt = get<1>(parse_result);

        // Done with the agent
        paths.push_back(path);
        sumStates += stateCnt;
        accum_stateCnts.push_back(sumStates);
      }
    }
    file.close();
  } else {
    std::cout << "exit\n";
    exit(0);
  }
  return make_tuple(paths, accum_stateCnts);
}

void add_type1_edges(Graph &graph, Paths &paths, vector<int> &accum_stateCnts) {
  int agentCnt = paths.size();
  for (int agent = 0; agent < agentCnt; agent++) {
    Path path = paths[agent];
    int stateCnt = path.size();
    int prev_vertex = -1;
    
    for (int state = 0; state < stateCnt; state++) {
      int curr_vertex = compute_vertex(accum_stateCnts, agent, state);
      if (prev_vertex >= 0) {
        set_type1_edge(graph, prev_vertex, curr_vertex);
      }
      prev_vertex = curr_vertex;
    }
  }
}

void add_type2_edges(Graph &graph, Paths &paths, vector<int> &accum_stateCnts) {
  int agentCnt = paths.size();
  // Looping through agents
  for (int agent1 = 0; agent1 < agentCnt; agent1++) {
    Path path1 = paths[agent1];
    int stateCnt1 = path1.size();
    for (int agent2 = agent1 + 1; agent2 < agentCnt; agent2 ++) {
      Path path2 = paths[agent2];
      int stateCnt2 = path2.size();

      // Looping through states
      for (int state1 = 0; state1 < stateCnt1; state1++) {
        pair<Location, int> pair1 = path1[state1];
        Location location1 = get<0>(pair1);
        for (int state2 = 0; state2 < stateCnt2; state2++) {
          pair<Location, int> pair2 = path2[state2];
          Location location2 = get<0>(pair2);

          int time1 = get<1>(pair1);
          int time2 = get<1>(pair2);

          if (same_locations(location1, location2) && (time1 >= 0) && (time2 >= 0)) {
            // Add a type2 edge
            int vertex1 = compute_vertex(accum_stateCnts, agent1, state1);
            int vertex2 = compute_vertex(accum_stateCnts, agent2, state2);

            // Set edges -- fix the starting out-edges and ending in-edges
            if (time1 < time2) {
              if ((state1 == 0) || (state2 == stateCnt2 - 1)) {
                set_type2_nonSwitchable_edge(graph, vertex1+1, vertex2);
              } else {
                set_type2_switchable_edge(graph, vertex1+1, vertex2);
              }
            }
            else {
              if ((state2 == 0) || (state1 == stateCnt1 - 1)) {
                set_type2_nonSwitchable_edge(graph, vertex2+1, vertex1);
              } else {
                set_type2_switchable_edge(graph, vertex2+1, vertex1);
              }
            }

          }
        }
      }
    }
  }
}

int add_type2_edges_cnt(Graph &graph, Paths &paths, vector<int> &accum_stateCnts) {
  int cnt = 0;
  int agentCnt = paths.size();
  // Looping through agents
  for (int agent1 = 0; agent1 < agentCnt; agent1++) {
    Path path1 = paths[agent1];
    int stateCnt1 = path1.size();
    for (int agent2 = agent1 + 1; agent2 < agentCnt; agent2 ++) {
      Path path2 = paths[agent2];
      int stateCnt2 = path2.size();

      // Looping through states
      for (int state1 = 0; state1 < stateCnt1; state1++) {
        pair<Location, int> pair1 = path1[state1];
        Location location1 = get<0>(pair1);
        for (int state2 = 0; state2 < stateCnt2; state2++) {
          pair<Location, int> pair2 = path2[state2];
          Location location2 = get<0>(pair2);

          int time1 = get<1>(pair1);
          int time2 = get<1>(pair2);

          if (same_locations(location1, location2) && (time1 >= 0) && (time2 >= 0)) {
            // Add a type2 edge
            int vertex1 = compute_vertex(accum_stateCnts, agent1, state1);
            int vertex2 = compute_vertex(accum_stateCnts, agent2, state2);

            // Set edges -- fix the starting out-edges and ending in-edges
            if (time1 < time2) {
              if ((state1 == 0) || (state2 == stateCnt2 - 1)) {
                set_type2_nonSwitchable_edge(graph, vertex1+1, vertex2);
              } else {
                set_type2_switchable_edge(graph, vertex1+1, vertex2);
                cnt += 1;
              }
            }
            else {
              if ((state2 == 0) || (state1 == stateCnt1 - 1)) {
                set_type2_nonSwitchable_edge(graph, vertex2+1, vertex1);
              } else {
                set_type2_switchable_edge(graph, vertex2+1, vertex1);
                cnt += 1;
              }
            }

          }
        }
      }
    }
  }
  return cnt;
}

ADG construct_ADG(char* fileName) {
  Paths paths;
  vector<int> accum_stateCnts;
  tie(paths, accum_stateCnts) = parse_soln(fileName);
  int sumStates = accum_stateCnts.back();

  Graph graph = new_graph(sumStates);
  add_type1_edges(graph, paths, accum_stateCnts);
  add_type2_edges(graph, paths, accum_stateCnts);

  return make_tuple(graph, paths, accum_stateCnts);
}

ADG construct_delayed_ADG(ADG &adg, int dlow, int dhigh, vector<int> &delayed_agents, vector<int> &states, int *input_sw_cnt, ofstream &outFile_setup) {
  int agentCnt = get_agentCnt(adg);
  Paths paths;
  vector<int> accum_stateCnts;
  
  random_device rd;  
  mt19937 gen(rd());
  uniform_int_distribution<> distrib(dlow, dhigh);

  int delay_sum = 0;

  for (int agent = 0; agent < agentCnt; agent ++) {
    if (delayed_agents[agent] == 0) {
      outFile_setup << "0\n";
      paths.push_back((get<1>(adg))[agent]);
    } else { // this is a delayed agent
      Path &ori_path = (get<1>(adg))[agent];
      Path new_path;
      int delayed_state = states[agent];

      for (int state = 0; state <= delayed_state; state ++) {
        new_path.push_back(ori_path[state]);
      }
      pair<Location, int> repeat = make_pair(get<0>(new_path.back()), -1);
      int delay = distrib(gen);
      outFile_setup << delay << "\n";
      delay_sum += delay;
      for (int state = 0; state < delay; state ++) {
        new_path.push_back(repeat);
      }
      int ori_size = ori_path.size();
      for (int state = delayed_state + 1; state < ori_size; state ++) {
        new_path.push_back(ori_path[state]);
      }
      paths.push_back(new_path);
    }

    accum_stateCnts.push_back((get<2>(adg))[agent] + delay_sum);
  }

  Graph graph = new_graph(accum_stateCnts.back());

  add_type1_edges(graph, paths, accum_stateCnts);
  *input_sw_cnt = add_type2_edges_cnt(graph, paths, accum_stateCnts);

  for (int v = 0; v < get<3>(graph); v ++) {
    int agent, state;
    tie(agent, state) = compute_agent_state(accum_stateCnts, v);
    if (state <= states[agent]) {
      set<int> outNeib = get_switchable_outNeib(graph, v);
      for (auto it: outNeib) {
        rem_type2_switchable_edge(graph, v, it);
        set_type2_nonSwitchable_edge(graph, v, it);
        *input_sw_cnt = *input_sw_cnt - 1;
      }
    }
  }

  return make_tuple(graph, paths, accum_stateCnts);
}