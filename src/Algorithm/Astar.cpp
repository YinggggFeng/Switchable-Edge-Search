#include "Astar.h"

Astar::Astar() {
}

Astar::Astar(int input_timeout) {
  timeout = input_timeout;
}

Astar::Astar(int input_timeout, bool input_fast_version) {
  timeout = input_timeout;
  fast_version = input_fast_version;
}


int Astar::calcTime(Simulator simulator) {
  int stepSpend = 0;
  int totalSpend = 0;

  stepSpend = simulator.step(false);
  while (stepSpend != 0) {
    if (stepSpend < 0) return -1; // stuck
    totalSpend += stepSpend;
    stepSpend = simulator.step(false);
  }
  return totalSpend;
}


int Astar::heuristic_graph(ADG &adg, vector<int> *ts, vector<int> *values) {
  Graph &graph = get<0>(adg);

  for (int i: (*ts)) {
    int prevVal = values->at(i);
    set<int> outNeib = get_nonSwitchable_outNeib(graph, i);
    for (auto it = outNeib.begin(); it != outNeib.end(); it++) {
      int j = *it;
      int weight = 1;
      if (values->at(j) < prevVal + weight) values->at(j) = prevVal + weight;
    }
  }
  int sum = 0;
  for (int agent = 0; agent < agentCnt; agent ++) {
    int goalVert = compute_vertex(get<2>(adg), agent, get_stateCnt(adg, agent) - 1);
    sum += values->at(goalVert);
  }

  // int partial_cost = compute_partial_cost(adg);
  // if (partial_cost != sum) {
  //   std::cout << "mismatched! PC = " << partial_cost << ", SUM = " << sum << "\n";
  // }
  return sum;
}

tuple<int, int, int> Astar::branch(Graph &graph, vector<int> *values) {
  int maxDiff = -1;
  int maxI = -1;
  int maxJ = -1;
  for (int i = 0; i < get<3>(graph); i++) {
    int iTime = values->at(i);
    set<int> &outNeib = get_switchable_outNeib(graph, i);
    for (auto it : outNeib) {
      int j = it;
      int jTime = values->at(j);
      int diff = iTime - jTime;
      if (diff > maxDiff) {
        maxDiff = diff;
        maxI = i;
        maxJ = j;
      }
      if (maxDiff > 0) {
        break;
      } 
    }
    if (maxDiff > 0) {
      break;
    } 
  }
  return make_tuple(maxDiff, maxI, maxJ);
}

void Astar::print_stats(ofstream &outFile) {
    outFile << explored_node_cnt << "," << 
    pruned_node_cnt << "," << 
    added_node_cnt << "," << 
    vertex_cnt << "," << 
    sw_edge_cnt << "," << 
    heuristicT.count() << "," << 
    branchT.count() << "," <<
    sortT.count() << "," << 
    pqT.count() << "," << 
    copy_free_graphsT.count() << "," << 
    termT.count() << "," << 
    dfsT.count() << endl;
}

void Astar::print_stats() {
    std::cout << "explored_node_cnt =" << explored_node_cnt << "\n";
    std::cout << "pruned_node_cnt =" << pruned_node_cnt << "\n";
    std::cout << "added_node_cnt =" << added_node_cnt << "\n\n";

    std::cout << "-------------------Time breakdown: \n";
    std::cout << "heuristic time: " << heuristicT.count() << "\n";
    std::cout << "branch time: " << branchT.count() << "\n";
    std::cout << "sort time: " << sortT.count() << "\n";
    std::cout << "pq time: " << pqT.count() << "\n";
    std::cout << "copy_free_graphs time: " << branchT.count() << "\n";
    std::cout << "dfs time: " << dfsT.count() << "\n";
}

ADG Astar::exploreNode() {
  auto start = high_resolution_clock::now();
  while (pq.size() > 0) {
    explored_node_cnt += 1;

    if (explored_node_cnt % 1000000 == 0) {
      std::cout << explored_node_cnt << "\n";
    }

    auto start_pq_pop = high_resolution_clock::now();
    Node* node = pq.top();
    pq.pop();
    auto end_pq_pop = high_resolution_clock::now();
    pqT += duration_cast<microseconds>(end_pq_pop - start_pq_pop);
    
    ADG &adg = get<0>(*node);
    vector<int> *values = get<2>(*node);
    Graph &graph = get<0>(adg);

    int maxDiff, maxI, maxJ;
    auto start_branch = high_resolution_clock::now();
    tie(maxDiff, maxI, maxJ) = branch(graph, values);
    auto end_branch = high_resolution_clock::now();
    branchT += duration_cast<microseconds>(end_branch - start_branch);

    bool terminate = true;

    if (true) {
      terminate = (maxDiff < 0);
    } else {
      for (int v = 0; v < get<3>(graph); v ++) {
        set<int>& outNeib = get_switchable_outNeib(graph, v);
        if (outNeib.size() != 0) {
          terminate = false;
          maxI = v;
          maxJ = *(outNeib.begin());
          break;
        }
      }
    }
    auto end_term = high_resolution_clock::now();
    termT += duration_cast<microseconds>(end_term - end_branch);

    if (terminate || ((duration_cast<seconds>(end_branch - start)).count() >= timeout)) {
      while (pq.size() > 0) {
        start_pq_pop = high_resolution_clock::now();
        Node* delete_node = pq.top();
        end_pq_pop = high_resolution_clock::now();
        free_underlying_graph(get<0>(*delete_node));
        auto end_graph_free = high_resolution_clock::now();

        pqT += duration_cast<microseconds>(end_pq_pop - start_pq_pop);
        copy_free_graphsT += duration_cast<microseconds>(end_graph_free - end_pq_pop);

        vector<int> *delete_vec = get<2>(*delete_node);
        delete delete_vec;
        delete delete_node;

        start_pq_pop = high_resolution_clock::now();
        pq.pop();
        end_pq_pop = high_resolution_clock::now();
        pqT += duration_cast<microseconds>(end_pq_pop - start_pq_pop);
      }
      auto start_graph_free = high_resolution_clock::now();
      ADG res = copy_ADG(adg);
      free_underlying_graph(adg);
      auto end_graph_free = high_resolution_clock::now();
      copy_free_graphsT += duration_cast<microseconds>(end_graph_free - start_graph_free);

      delete values;
      delete node;

      set_switchable_nonSwitchable(get<0>(res));
      return res;
    } else {
      auto start_graph_copy = high_resolution_clock::now();
      ADG copy = copy_ADG(adg);
      auto end_graph_copy = high_resolution_clock::now();
      copy_free_graphsT += duration_cast<microseconds>(end_graph_copy - start_graph_copy);

      // Forward
      // Fix the edge
      rem_type2_switchable_edge(graph, maxI, maxJ);
      set_type2_nonSwitchable_edge(graph, maxI, maxJ);

      auto start_dfs = high_resolution_clock::now();
      bool pruned_forward = check_cycle_dfs(graph, maxI);
      auto end_dfs = high_resolution_clock::now();
      dfsT += duration_cast<microseconds>(end_dfs - start_dfs);

      if (pruned_forward) { // Prune node
        auto start_graph_free = high_resolution_clock::now();
        free_graph(graph);
        auto end_graph_free = high_resolution_clock::now();
        copy_free_graphsT += duration_cast<microseconds>(end_graph_free - start_graph_free);
        pruned_node_cnt += 1;
      } else {
        vector<int>* newts_tv_init = nullptr;
        vector<int>* newts_vt_init = nullptr;
        sortResult newInitResult = make_pair(newts_tv_init, newts_vt_init);

        vector<int>* newts_tv;
        vector<int>* newts_vt;
        vector<int>* node_values = new vector<int>(get<3>(graph), 0);

        auto start_sort = high_resolution_clock::now();
        tie(newts_tv, newts_vt) = topologicalSort(graph, newInitResult, &currents, -1, -1);
        auto end_sort = high_resolution_clock::now();
        int val = heuristic_graph(adg, newts_tv, node_values);
        auto end_heuristic = high_resolution_clock::now();

        sortT += duration_cast<microseconds>(end_sort - start_sort);
        heuristicT += duration_cast<microseconds>(end_heuristic - end_sort);

        delete newts_tv;
        delete newts_vt;

        Node *forward_node = new Node;
        *forward_node = make_tuple(adg, val, node_values);

        auto start_pq_push = high_resolution_clock::now();
        pq.push(forward_node);
        auto end_pq_push = high_resolution_clock::now();
        pqT += duration_cast<microseconds>(end_pq_push - start_pq_push);

        added_node_cnt += 1;
      }

      // Backward 
      Graph &copyGraph = get<0>(copy);

      // Fix the edge
      int backI = maxJ+1;
      int backJ = maxI-1;

      rem_type2_switchable_edge(copyGraph, maxI, maxJ);
      set_type2_nonSwitchable_edge(copyGraph, backI, backJ);

      start_dfs = high_resolution_clock::now();
      bool pruned_backward = check_cycle_dfs(copyGraph, backI);
      end_dfs = high_resolution_clock::now();
      dfsT += duration_cast<microseconds>(end_dfs - start_dfs);

      if (pruned_backward) { // Prune node
        auto start_graph_free = high_resolution_clock::now();
        free_graph(copyGraph);
        auto end_graph_free = high_resolution_clock::now();
        copy_free_graphsT += duration_cast<microseconds>(end_graph_free - start_graph_free);
        pruned_node_cnt += 1;
      } else {
        vector<int>* newts_tv_init = nullptr;
        vector<int>* newts_vt_init = nullptr;
        sortResult newInitResult = make_pair(newts_tv_init, newts_vt_init);

        vector<int>* newts_tv;
        vector<int>* newts_vt;
        vector<int>* node_values = new vector<int>(get<3>(copyGraph), 0);

        auto start_sort = high_resolution_clock::now();
        tie(newts_tv, newts_vt) = topologicalSort(copyGraph, newInitResult, &currents, -1, -1);
        auto end_sort = high_resolution_clock::now();
        int val = heuristic_graph(copy, newts_tv, node_values);
        auto end_heuristic = high_resolution_clock::now();

        sortT += duration_cast<microseconds>(end_sort - start_sort);
        heuristicT += duration_cast<microseconds>(end_heuristic - end_sort);

        delete newts_tv;
        delete newts_vt;

        Node *backward_node = new Node;
        *backward_node = make_tuple(copy, val, node_values);

        auto start_pq_push = high_resolution_clock::now();
        pq.push(backward_node);
        auto end_pq_push = high_resolution_clock::now();
        pqT += duration_cast<microseconds>(end_pq_push - start_pq_push);

        added_node_cnt += 1;
      }
    }
    delete values;
    delete node;
  }
  throw invalid_argument("no solution found");
}

tuple<bool, int, int, int> Astar::slow_branch(ADG &adg, vector<int> *states) {
  // std::cout << "old states";
  // for (auto i: *states) {
  //   std::cout << i << ' ';
  // }
  // std::cout << "\n";

  int stepSpend = 0;
  int totalSpend = 0;
  Simulator simulator(adg, *states);

  int v_from = -1;
  int v_to = -1;

  if (simulator.incident_to_switchable(&v_from, &v_to)) {
    // std::cout << "initial incident\n";
    return make_tuple(false, v_from, v_to, totalSpend); // terminate
  }

  stepSpend = simulator.step(false);
  while (stepSpend != 0) {
    totalSpend += stepSpend;
    if (simulator.incident_to_switchable(&v_from, &v_to)) {
      *states = simulator.states;
      // std::cout << "new states";
      // for (auto i: simulator.states) {
      //   std::cout << i << ' ';
      // }
      // std::cout << ", totalSpend = " << totalSpend << "\n";
      return make_tuple(false, v_from, v_to, totalSpend); // terminate
    }
    stepSpend = simulator.step(false);
  }
  return make_tuple(true, -1, -1, totalSpend); // terminate
}

int Astar::slow_heuristic(ADG &adg, vector<int> &states) {
  Simulator simulator(adg, states);
  int cost = simulator.print_soln();
  return cost;
}

int Astar::compute_partial_cost(ADG &adg) {
  // Graph &graph = get<0>(copy);
  // remove_all_switchable_edges(graph);

  Simulator simulator(adg);
  int cost = simulator.print_soln();
  return cost;
}

ADG Astar::slow_exploreNode() {
  auto start = high_resolution_clock::now();
  while (slow_pq.size() > 0) {
    explored_node_cnt += 1;
    if (explored_node_cnt % 100000 == 0) {
      std::cout << explored_node_cnt << "\n";
    }

    auto start_pq_pop = high_resolution_clock::now();
    slow_Node* node = slow_pq.top();
    // std::cout << "g =" << get<1>(*node) << ", h=" << get<2>(*node) << "\n";
    slow_pq.pop();
    auto end_pq_pop = high_resolution_clock::now();
    pqT += duration_cast<microseconds>(end_pq_pop - start_pq_pop);
    
    ADG &adg = get<0>(*node);
    vector<int> states = get<3>(*node);

    int v_from, v_to, g_delta;
    bool terminate;
    auto start_branch = high_resolution_clock::now();
    tie(terminate, v_from, v_to, g_delta) = slow_branch(adg, &states);
    auto end_branch = high_resolution_clock::now();
    branchT += duration_cast<microseconds>(end_branch - start_branch);

    auto end_term = high_resolution_clock::now();
    termT += duration_cast<microseconds>(end_term - end_branch);

    if (terminate || ((duration_cast<seconds>(end_branch - start)).count() >= timeout)) {
      while (slow_pq.size() > 0) {
        start_pq_pop = high_resolution_clock::now();
        slow_Node* delete_node = slow_pq.top();
        end_pq_pop = high_resolution_clock::now();
        free_underlying_graph(get<0>(*delete_node));
        auto end_graph_free = high_resolution_clock::now();

        pqT += duration_cast<microseconds>(end_pq_pop - start_pq_pop);
        copy_free_graphsT += duration_cast<microseconds>(end_graph_free - end_pq_pop);

        delete delete_node;

        start_pq_pop = high_resolution_clock::now();
        slow_pq.pop();
        end_pq_pop = high_resolution_clock::now();
        pqT += duration_cast<microseconds>(end_pq_pop - start_pq_pop);
      }
      auto start_graph_free = high_resolution_clock::now();
      ADG res = copy_ADG(adg);
      free_underlying_graph(adg);
      auto end_graph_free = high_resolution_clock::now();
      copy_free_graphsT += duration_cast<microseconds>(end_graph_free - start_graph_free);

      delete node;
      set_switchable_nonSwitchable(get<0>(res));
      return res;
    } else {
      int g = get<1>(*node);
      auto start_graph_copy = high_resolution_clock::now();
      ADG copy = copy_ADG(adg);
      auto end_graph_copy = high_resolution_clock::now();
      copy_free_graphsT += duration_cast<microseconds>(end_graph_copy - start_graph_copy);

      Graph &graph = get<0>(adg);

      // Forward
      // Fix the edge
      rem_type2_switchable_edge(graph, v_from, v_to);
      set_type2_nonSwitchable_edge(graph, v_from, v_to);

      auto start_dfs = high_resolution_clock::now();
      bool pruned_forward = check_cycle_dfs(graph, v_from);
      auto end_dfs = high_resolution_clock::now();
      dfsT += duration_cast<microseconds>(end_dfs - start_dfs);

      if (pruned_forward) { // Prune node
        auto start_graph_free = high_resolution_clock::now();
        free_graph(graph);
        auto end_graph_free = high_resolution_clock::now();
        copy_free_graphsT += duration_cast<microseconds>(end_graph_free - start_graph_free);
        pruned_node_cnt += 1;
      } else {
        auto start_heuristic = high_resolution_clock::now();
        int h = slow_heuristic(adg, states);
        auto end_heuristic = high_resolution_clock::now();
        heuristicT += duration_cast<microseconds>(end_heuristic - start_heuristic);

        slow_Node *forward_node = new slow_Node;

        // int partial_cost = compute_partial_cost(adg);

        // if (g+g_delta+h!= partial_cost) {
        //   std::cout << "mismatched cost!!!";
        // }
        // std::cout << "g = " << g << ", g_delta = " << g_delta << ", h=" << h << ", partial cost = " << partial_cost << "\n";
        *forward_node = make_tuple(adg, g+g_delta, h, states);

        auto start_pq_push = high_resolution_clock::now();
        slow_pq.push(forward_node);
        auto end_pq_push = high_resolution_clock::now();
        pqT += duration_cast<microseconds>(end_pq_push - start_pq_push);

        added_node_cnt += 1;
      }

      // Backward 
      Graph &copyGraph = get<0>(copy);

      // Fix the edge
      int backI = v_to+1;
      int backJ = v_from-1;

      rem_type2_switchable_edge(copyGraph, v_from, v_to);
      set_type2_nonSwitchable_edge(copyGraph, backI, backJ);

      start_dfs = high_resolution_clock::now();
      bool pruned_backward = check_cycle_dfs(copyGraph, backI);
      end_dfs = high_resolution_clock::now();
      dfsT += duration_cast<microseconds>(end_dfs - start_dfs);

      if (pruned_backward) { // Prune node
        auto start_graph_free = high_resolution_clock::now();
        free_graph(copyGraph);
        auto end_graph_free = high_resolution_clock::now();
        copy_free_graphsT += duration_cast<microseconds>(end_graph_free - start_graph_free);
        pruned_node_cnt += 1;
      } else {
        auto start_heuristic = high_resolution_clock::now();
        int h = slow_heuristic(copy, states);
        auto end_heuristic = high_resolution_clock::now();
        heuristicT += duration_cast<microseconds>(end_heuristic - start_heuristic);

        slow_Node *backward_node = new slow_Node;
        // int partial_cost = compute_partial_cost(copy);
        // if (g+g_delta+h!= partial_cost) {
        //   std::cout << "mismatched cost!!!";
        // }
        // std::cout << "g = " << g << ", g_delta = " << g_delta << ", h=" << h << ", partial cost = " << partial_cost << "\n";
        
        *backward_node = make_tuple(copy, g+g_delta, h, states);

        auto start_pq_push = high_resolution_clock::now();
        slow_pq.push(backward_node);
        auto end_pq_push = high_resolution_clock::now();
        pqT += duration_cast<microseconds>(end_pq_push - start_pq_push);

        added_node_cnt += 1;
      }
    }
    delete node;
  }
  throw invalid_argument("no solution found");
}

ADG Astar::startExplore(ADG &adg, int input_sw_cnt) {
  vertex_cnt = get<3>(get<0>(adg));
  sw_edge_cnt = input_sw_cnt;
  // std::cout << "vertex_cnt = " << vertex_cnt << ", sw_edge_cnt = " << sw_edge_cnt << "\n";
  if (fast_version) {
    agentCnt = get_agentCnt(adg);
    Graph &graph = get<0>(adg);

    for (int agent = 0; agent < agentCnt; agent ++) {
      int current = compute_vertex(get<2>(adg), agent, 0);
      currents.push_back(current);
    }
    
    vector<int>* ts_tv_init = nullptr;
    vector<int>* ts_vt_init = nullptr;
    sortResult initResult = make_pair(ts_tv_init, ts_vt_init);

    auto start_sort = high_resolution_clock::now();
    sortResult result = topologicalSort(graph, initResult, &currents, -1, -1);
    auto end_sort = high_resolution_clock::now();
    sortT += duration_cast<microseconds>(end_sort - start_sort);

    delete result.second;
    vector<int>* ts_tv = result.first;
    vector<int>* node_values = new vector<int>(get<3>(graph), 0);

    auto start_heuristic = high_resolution_clock::now();
    int val = heuristic_graph(adg, ts_tv, node_values);
    auto end_heuristic = high_resolution_clock::now();
    heuristicT += duration_cast<microseconds>(end_heuristic - start_heuristic);
    delete ts_tv;

    Node *root = new Node;
    *root = make_tuple(adg, val, node_values);

    auto start_pq_push = high_resolution_clock::now();
    pq.push(root);
    auto end_pq_push = high_resolution_clock::now();
    pqT += duration_cast<microseconds>(end_pq_push - start_pq_push);

    return exploreNode();
  } else {
    agentCnt = get_agentCnt(adg);

    slow_Node *root = new slow_Node;
    vector<int> states(agentCnt, 0);
    *root = make_tuple(adg, 0, 0, states);

    auto start_pq_push = high_resolution_clock::now();
    slow_pq.push(root);
    auto end_pq_push = high_resolution_clock::now();
    pqT += duration_cast<microseconds>(end_pq_push - start_pq_push);

    return slow_exploreNode();
  }
}

