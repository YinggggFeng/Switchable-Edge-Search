#ifndef SIMULATOR
#define SIMULATOR

#include "../ADG/generate_ADG.h"

class Simulator {
  public:         
    ADG adg;
    vector<int> states;
    
    Simulator(ADG adg);
    Simulator(ADG adg, vector<int> visited_states);
    int step(bool switchCheck);
    int checkMovable(vector<int>& movable, vector<int>& haventStop);
    int checkMovable(vector<int>& movable);
    void print_location(ofstream &outFile, Location location);
    int print_soln(const char* outFileName);
    int print_soln();

    bool incident_to_switchable(int *v_from, int *v_to);


    int step_wdelay(int p, bool *delay_mark, vector<int> &delayed_agents);
    // bool amove(vector<int>& moved, int agent, int *timeSpent, int *delayer, int p, int d);
    int simulate_wdelay(int p, int dlow, int dhigh, ofstream &outFile, ofstream &outFile_slow, ofstream &outFile_path, ofstream &outFile_setup, int timeout);
};
#endif