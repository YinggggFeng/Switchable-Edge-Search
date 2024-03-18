# Switchable-Edge-Search

This repository is the implementation of A Real-Time Rescheduling Algorithm for Multi-robot Plan Execution published in ICAPS 2024.

The source code for K-Robust CBS is available at https://github.com/nobodyczcz/Lazy-Train-and-K-CBS. The "wait-on-start" branch supports the delayed situation that we considered.

Build:

```bash
g++ -Wall -Werror -Ofast src/types.h src/graph/.h src/graph/.cpp src/ADG/.h src/ADG/.cpp src/Algorithm/.h src/Algorithm/.cpp src/simulate.cpp
```

Run:
```bash
./a.out {path input file} {delay probability -- an integer in [0, 1000]} {lower bound of the delay length -- an integer} {upper bound of the delay length -- an integer} {output file (stats) for the graph-based module} {output file (stats) for the execution-based module} {output file for the start and goal locations when a delay happens} {output file for the index of the delayed agents and the length of the delay}
```

Example:
```bash
./a.out example/path.txt 10 10 20 stats_graph.csv stats_exec.csv locations.txt delay_setup.txt
```

The above command reads an example path, uses a delay probability = 1 percent and a delay length range = [10, 20], and creates two .csv files to record the output stats and two .txt files to record the information about the delay.

The stats columns are (from left to right):

runtime || runtime + the time for constructing the TPG || original total cost of the TPG || replanned total cost || original remaining cost of the TPG (the parts after the delay) || replanned remaining cost

The rest columns are about runtime breakdown and the number of search nodes.
