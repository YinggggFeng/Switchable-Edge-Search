#include <fstream>

#include "Algorithm/Astar.h"

void print_for_replanning(ADG &adg, vector<int> states, ofstream &outFile_path) {
  outFile_path << "version 1\n";
  int agentCnt = get_agentCnt(adg);
  for (int agent = 0; agent < agentCnt; agent ++) {
    outFile_path << "4	random-32-32-10.map	32	32	";
    Location l = get_state_target(adg, agent, states[agent]);
    Location e = get_state_target(adg, agent, get_stateCnt(adg, agent)-1);
    outFile_path << get<1>(l) << "	" << get<0>(l) << "	" << get<1>(e) << "	" << get<0>(e) << "	18.72792206\n";
  }
}

int Simulator::step_wdelay(int p, bool *delay_mark, vector<int> &delayed_agents) {
  int agentCnt = get_agentCnt(adg);

  random_device rd;  
  mt19937 gen(rd());
  uniform_int_distribution<> distrib(1, 1000);

  vector<int> movable(agentCnt, 0);
  vector<int> haventStop(agentCnt, 0);
  int timeSpent = checkMovable(movable, haventStop);
  int moveCnt = 0;

  for (int agent = 0; agent < agentCnt; agent++) {
    if (haventStop[agent] == 1) {
      if (distrib(gen) <= p) {
        delayed_agents[agent] = 1;
        *delay_mark = true;
      }
    }
  }

  if (*delay_mark) {
    return 0;
  }

  for (int agent = 0; agent < agentCnt; agent++) {
    if (movable[agent] == 1) {
      states[agent] += 1;
      moveCnt += 1;
    }
  }
  if (moveCnt == 0 && timeSpent != 0) {
    return -1;
  }
  return timeSpent;
}

int Simulator::simulate_wdelay(int p, int dlow, int dhigh, ofstream &outFile, ofstream &outFile_slow, ofstream &outFile_path, ofstream &outFile_setup, int timeout) {
  int agentCnt = get_agentCnt(adg);
  int stepSpend = 1;
  bool delay_mark = false;
  vector<int> delayed_agents(agentCnt, 0);

  while (stepSpend != 0) {
    stepSpend = step_wdelay(p, &delay_mark, delayed_agents);
    if (delay_mark) // a delay just happened
    {
      print_for_replanning(adg, states, outFile_path);
      outFile_path.close();
      // int delayed_state = states[delayed_agent];

      int input_sw_cnt;

      microseconds timer_constructADG(0);
      auto start = high_resolution_clock::now();
      ADG adg_delayed = construct_delayed_ADG(adg, dlow, dhigh, delayed_agents, states, &input_sw_cnt, outFile_setup);
      outFile_setup.close();
      auto stop = high_resolution_clock::now();
      timer_constructADG += duration_cast<microseconds>(stop - start);

      ADG adg_delayed_copy = copy_ADG(adg_delayed);
      set_switchable_nonSwitchable(get<0>(adg_delayed_copy));
      Simulator simulator_original(adg_delayed_copy);
      int originalTime = simulator_original.print_soln();
      Simulator simulator_ori_trunc(adg_delayed_copy, states);
      int oriTime_trunc = simulator_ori_trunc.print_soln();

      ADG adg_delayed_slow = copy_ADG(adg_delayed);

      microseconds timer(0);
      start = high_resolution_clock::now();
      Astar search(timeout, true);
      ADG replanned_adg = search.startExplore(adg_delayed, input_sw_cnt);
      stop = high_resolution_clock::now();
      timer += duration_cast<microseconds>(stop - start);

      if (duration_cast<seconds>(timer).count() >= timeout) {
        outFile << timer.count() << "," << 
        timer.count() + timer_constructADG.count() << ",,,,,";
        search.print_stats(outFile);
        exit(0);
      }
      
      Simulator simulator_res(replanned_adg);
      int timeSum = simulator_res.print_soln("out.txt");
      Simulator simulator_res_trunc(replanned_adg, states);
      int timeSum_trunc = simulator_res_trunc.print_soln();

      outFile << timer.count() << "," << 
      timer.count() + timer_constructADG.count() << "," << 
      originalTime << "," <<
      timeSum << "," << 
      oriTime_trunc << "," <<
      timeSum_trunc << ",";
      
      search.print_stats(outFile);

      microseconds timer_slow(0);
      start = high_resolution_clock::now();
      Astar search_slow(timeout, false);
      ADG replanned_slow = search_slow.startExplore(adg_delayed_slow, input_sw_cnt);
      stop = high_resolution_clock::now();
      timer_slow += duration_cast<microseconds>(stop - start);

      if (duration_cast<seconds>(timer_slow).count() >= timeout) {
        outFile_slow << timer_slow.count() << "," << 
        timer_slow.count() + timer_constructADG.count() << ",,,,,";
        search_slow.print_stats(outFile_slow);
        exit(0);
      }
      
      Simulator simulator_slow(replanned_slow);
      timeSum = simulator_slow.print_soln();
      Simulator simulator_slow_trunc(replanned_slow, states);
      timeSum_trunc = simulator_slow_trunc.print_soln();

      outFile_slow << timer_slow.count() << "," << 
      timer_slow.count() + timer_constructADG.count() << "," << 
      originalTime << "," <<
      timeSum << "," << 
      oriTime_trunc << "," <<
      timeSum_trunc << ",";
      
      search_slow.print_stats(outFile_slow);
      exit(0);
    }
    if (stepSpend < 0) return -1; // stuck
  }
  std::cout << "no delay happened\n";
  return 0;
}

int main(int argc, char** argv) {
  char* fileName = argv[1];
  int p = atoi(argv[2]);
  int dlow = atoi(argv[3]);
  int dhigh = atoi(argv[4]);
  const char* outFileName = argv[5];
  const char* outFileName_slow = argv[6];
  const char* outFileName_path = argv[7];
  const char* outFileName_setup = argv[8];
  int timeout = 90;

  ADG adg = construct_ADG(fileName);

  ofstream outFile;
  outFile.open(outFileName, ios::app);
  ofstream outFile_slow;
  outFile_slow.open(outFileName_slow, ios::app);
  ofstream outFile_path;
  outFile_path.open(outFileName_path, ios::app);
  ofstream outFile_setup;
  outFile_setup.open(outFileName_setup, ios::app);
  if (outFile.is_open()) {
    // if (print_header) {
    //   outFile << "search time,search + construct time,original total cost,"	
    //   << "solution total cost,original remain cost,solution remain cost,"
    //   << "explored node,pruned node,added node,heuristic time,branch time,"
    //   << "sort time,priority queue time,copy&free graph time,dfs time" << endl;
    // }

    Simulator simulator(adg);
    simulator.simulate_wdelay(p, dlow, dhigh, outFile, outFile_slow, outFile_path, outFile_setup, timeout);
    outFile.close();
    outFile_slow.close();
  }
  return 0;
}