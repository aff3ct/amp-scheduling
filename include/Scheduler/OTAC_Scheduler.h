#pragma once
#include "Support/Loads.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cassert>
#include <limits>
#include <cmath>

struct StagesOTAC {
    const bool valid;
    const int num_tasks;
    const int resources;
    const double max_load;
    StagesOTAC* next_stage;
    StagesOTAC(const bool valid, const int num_tasks, const int resources, const double max_load, StagesOTAC* next_stage) 
        : valid(valid), num_tasks(num_tasks), resources(resources), max_load(max_load), next_stage(next_stage){}    
};

// Class to compute a schedule for two kinds of resources
class OTAC {
private:
    std::vector<double> loads;
    std::vector<bool> task_states;
    int resources;
    IntervalLoad intervals;
    IntervalState interval_state;
    StatefulList stateful_list;
    int num_tasks;
    double max_load;
    double max_stateful;
    bool r_type;

public:
    OTAC(const std::vector<double>& loads, const std::vector<bool>& task_states, int resources, bool r_type);
    std::pair<std::vector<std::tuple<int, int, bool, double>>, double> compute_schedule();
    std::pair<std::vector<std::tuple<int, int, bool, double>>, double> extract_solution(StagesOTAC* solution);
    StagesOTAC* schedule_given_period(double period, int start, int resources);
};