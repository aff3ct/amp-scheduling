#pragma once
#include "Support/Loads.h"

#include <vector>
#include <algorithm>
#include <numeric>
#include <cassert>
#include <limits>

class HeRAD {
private:
    IntervalLoad intervals;
    IntervalState interval_state;
    int num_tasks;
    const bool Big = true;
    const bool Little = false;
    int resources_big;
    int resources_little;
    std::vector<std::vector<std::vector<double>>> min_loads;
    std::vector<std::vector<std::vector<std::pair<int, int>>>> previous_coordinates;
    std::vector<std::vector<std::vector<std::pair<int, int>>>> cumm_resources;
    std::vector<std::vector<std::vector<bool>>> type_of_resource;
    std::vector<std::vector<std::vector<int>>> interval_start;

public:
    HeRAD(const std::vector<double>& loads_big,
                   const std::vector<double>& loads_little,
                   const std::vector<bool>& task_states,
                   int resources_big,
                   int resources_little);
    void compute_initial_plane(int end_task);
    void recompute_cell(int end_task, int resources_big, int resources_little);
    auto extract_solution();
    std::pair<std::vector<std::tuple<int, int, bool, double>>, double> compute_schedule();
};