#pragma once
#include "Support/Loads.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cassert>
#include <limits>
#include <cmath>
#include <stack>

struct Stages {
    const bool valid;
    const int num_tasks;
    const int resources;
    const bool r_type;
    const double max_load;
    const int total_little;
    const int total_big;
    Stages* next_stage;
    Stages(const bool valid, const int num_tasks, const int resources, const bool r_type, const double max_load, 
           const int total_little, const int total_big, Stages* next_stage) 
        : valid(valid), num_tasks(num_tasks), resources(resources), r_type(r_type), max_load(max_load),
        total_little(total_little), total_big(total_big), next_stage(next_stage){}    
};

// Class to compute a schedule for two kinds of resources (tries both core types at each stage)
class TwoCATAC {
protected:
    std::vector<double> loads_big;
    std::vector<double> loads_little;
    std::vector<bool> task_states;
    int resources_big;
    int resources_little;
    const bool Big = true;
    const bool Little = false;
    IntervalLoad intervals;
    IntervalState interval_state;
    StatefulList stateful_list;
    int num_tasks;
    double max_load;
    double max_stateful;

public:
    TwoCATAC(const std::vector<double>& loads_big, const std::vector<double>& loads_little, const std::vector<bool>& task_states,
          int resources_big, int resources_little);
    std::pair<std::vector<std::tuple<int, int, bool, double>>, double> compute_schedule();
    std::pair<std::vector<std::tuple<int, int, bool, double>>, double> extract_solution(Stages* solution);
    Stages* schedule_given_period(double period, int start, int resources, int other_resources, bool r_type);
    bool first_best_solution(Stages* first_schedule, Stages* second_schedule);
    void free_schedule(Stages* schedule);
    std::pair<int, int> compute_stage(double period, int start, int resources, bool r_type);
};

// Class to compute a schedule for two kinds of resources (tries to use little cores first)
class FERTAC : public TwoCATAC {

public:
    FERTAC(const std::vector<double>& loads_big, const std::vector<double>& loads_little, const std::vector<bool>& task_states,
          int resources_big, int resources_little);
    std::pair<std::vector<std::tuple<int, int, bool, double>>, double> compute_schedule();
    Stages* schedule_given_period(double period, int start, int resources_big, int resources_little);
};