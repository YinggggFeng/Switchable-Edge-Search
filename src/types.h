#ifndef TYPES
#define TYPES

#include <stdlib.h>
#include <tuple>
#include <vector>
#include <set>
#include <utility>

using namespace std;

// <arr[time]=vertex, arr[vertex]=time>
typedef pair<vector<int>*, vector<int>*> sortResult;

// <outNeighbors, inNeighbors>
typedef pair<set<int>*, set<int>*> subGraph;

// <type1 Graph, non-switchable Type2 Graph, switchable Type2 Graph, num nodes>
typedef tuple<subGraph, subGraph, subGraph, int> Graph;
typedef pair<int, int> Location;
typedef vector<pair<Location, int>> Path;
typedef vector<Path> Paths;
typedef tuple<Graph, Paths, vector<int>> ADG;
typedef tuple<ADG, int, vector<int>*> Node;

typedef tuple<ADG, int, int, vector<int>> slow_Node;


#endif

