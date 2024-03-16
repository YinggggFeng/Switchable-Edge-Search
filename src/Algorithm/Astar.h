#ifndef ASTAR
#define ASTAR

#include <queue>
#include <chrono>
using namespace std::chrono;

#include "../ADG/ADG_utilities.h"
#include "simulator.h"

class Astar {
  public:
    Astar();
    Astar(int input_timeout);
    Astar(int input_timeout, bool input_fast_version);
    ADG startExplore(ADG &adg, int input_sw_cnt);
    int heuristic_graph(ADG &adg, vector<int> *ts, vector<int> *values);
    int slow_heuristic(ADG &adg, vector<int> &states);

    int compute_partial_cost(ADG &adg);

    void print_stats();
    void print_stats(ofstream &outFile);
    
  private:
    class Compare {
      public:
        bool operator() (Node* s1, Node* s2)
        {
          int val1 = get<1>(*s1);
          int val2 = get<1>(*s2);

          return val1 > val2;
        }
    };

    class slow_Compare {
      public:
        bool operator() (slow_Node* s1, slow_Node* s2)
        {
          int g1 = get<1>(*s1);
          int h1 = get<2>(*s1);
          int g2 = get<1>(*s2);
          int h2 = get<2>(*s2);

          return g1+h1 > g2+h2;
        }
    };

    int calcTime(Simulator simulator);
    ADG exploreNode();
    ADG slow_exploreNode();
    tuple<int, int, int> branch(Graph &graph, vector<int> *values);
    tuple<bool, int, int, int> slow_branch(ADG &adg, vector<int> *states);

    microseconds heuristicT = std::chrono::microseconds::zero();
    microseconds branchT = std::chrono::microseconds::zero();
    microseconds sortT = std::chrono::microseconds::zero();
    microseconds pqT = std::chrono::microseconds::zero();
    microseconds copy_free_graphsT = std::chrono::microseconds::zero();
    microseconds dfsT = std::chrono::microseconds::zero();
    microseconds termT = std::chrono::microseconds::zero();

    microseconds totalT  = std::chrono::seconds::zero();

    int explored_node_cnt = 0;
    int pruned_node_cnt = 0;
    int added_node_cnt = 0;

    int vertex_cnt = 0;
    int sw_edge_cnt = 0;
    
    int timeout = 300;

    vector<int> currents;
    priority_queue<Node*, vector<Node*>, Compare> pq;
    priority_queue<slow_Node*, vector<slow_Node*>, slow_Compare> slow_pq;
    int agentCnt = 0;

    bool fast_version = false;
};
#endif