#include <cstdio>
#include <set>
#include <map>
#include <vector>
#include <unordered_map>

#ifdef USE_MEMORY
#include "hc_solver_common.h"
#else
#ifdef USE_DECISIVE
#include "hc_solver_common_d.h"
#else
#include "hc_solver_common_cc.h"
#endif
#endif

std::pair<bool,std::vector<int>> solve(std::vector<TWNode>&,std::vector<std::vector<int>>&,std::set<std::pair<int,int>>&,int);

