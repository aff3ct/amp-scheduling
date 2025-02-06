#include "Scheduler/DP_Scheduler.h"

HeRAD::HeRAD(const std::vector<double>& loads_big,
                               const std::vector<double>& loads_little,
                               const std::vector<bool>& task_states,
                               int resources_big,
                               int resources_little)
    : intervals(loads_big, loads_little, task_states),
    interval_state(task_states),
    num_tasks(loads_big.size()),
    resources_big(resources_big),
    resources_little(resources_little) {

    assert(resources_big > 0 && resources_little > 0);
    assert(num_tasks > 0);

    min_loads.resize(num_tasks, std::vector<std::vector<double>>(resources_big + 1, std::vector<double>(resources_little + 1, std::numeric_limits<double>::infinity())));
    previous_coordinates.resize(num_tasks, std::vector<std::vector<std::pair<int, int>>>(resources_big + 1, std::vector<std::pair<int, int>>(resources_little + 1, {0, 0})));
    cumm_resources.resize(num_tasks, std::vector<std::vector<std::pair<int, int>>>(resources_big + 1, std::vector<std::pair<int, int>>(resources_little + 1, {0, 0})));
    type_of_resource.resize(num_tasks, std::vector<std::vector<bool>>(resources_big + 1, std::vector<bool>(resources_little + 1, false)));
    interval_start.resize(num_tasks, std::vector<std::vector<int>>(resources_big + 1, std::vector<int>(resources_little + 1, 0)));
}

void HeRAD::compute_initial_plane(int end_task) {
    for (int r_little = 1; r_little <= resources_little; ++r_little) {
        auto [load, r_used] = intervals.compute_load(0, end_task, r_little, Little);
        min_loads[end_task][0][r_little] = load;
        cumm_resources[end_task][0][r_little] = {0, r_used};
        type_of_resource[end_task][0][r_little] = Little;
    }

    for (int r_big = 1; r_big <= resources_big; ++r_big) {
        auto [load, r_used] = intervals.compute_load(0, end_task, r_big, Big);
        for (int r_little = 0; r_little <= resources_little; ++r_little) {
            auto [_, total_r_little] = cumm_resources[end_task][0][r_little];
            double little_load = min_loads[end_task][0][r_little];
            // PRIORITIES: < load, > little resources, < resources
            if (load < little_load) {
                // better to use big resources
                min_loads[end_task][r_big][r_little] = load;
                cumm_resources[end_task][r_big][r_little] = {r_used, 0};
                type_of_resource[end_task][r_big][r_little] = Big;
            } else {
                // better to use small resources 
                min_loads[end_task][r_big][r_little] = min_loads[end_task][0][r_little];
                cumm_resources[end_task][r_big][r_little] = {0, total_r_little};
                type_of_resource[end_task][r_big][r_little] = Little;                    
            }
        }
    }
}

void HeRAD::recompute_cell(int end_task, int resources_big, int resources_little) {
    // for a given cell in the matrix, tries:
    // different starting points w/ different numbers of resources
    double current_load = min_loads[end_task][resources_big][resources_little];
    auto [current_coord_big, current_coord_little] = previous_coordinates[end_task][resources_big][resources_little];
    bool current_type_of_resource = type_of_resource[end_task][resources_big][resources_little];
    auto [current_big, current_little] = cumm_resources[end_task][resources_big][resources_little];
    int current_start_task = interval_start[end_task][resources_big][resources_little];

    // First step: get the previous best solution with fewer resources
    if (resources_little > 0) {
        double neigh_load = min_loads[end_task][resources_big][resources_little-1];
        auto [neigh_big, neigh_little] = cumm_resources[end_task][resources_big][resources_little-1];
        if ((neigh_load < current_load) ||
            ((neigh_load == current_load) && (neigh_little > current_little) && (neigh_big < current_big)) ||
            ((neigh_load == current_load) && (neigh_little <= current_little) && (neigh_big <= current_big))) {
            current_load = min_loads[end_task][resources_big][resources_little-1];
            auto coord = previous_coordinates[end_task][resources_big][resources_little-1];
            current_coord_big = coord.first;
            current_coord_little = coord.second;
            current_type_of_resource = type_of_resource[end_task][resources_big][resources_little-1];
            auto bl = cumm_resources[end_task][resources_big][resources_little-1];
            current_big = bl.first;
            current_little = bl.second;
            current_start_task = interval_start[end_task][resources_big][resources_little-1];
        }
    }
    if (resources_big > 0) {
        double neigh_load = min_loads[end_task][resources_big-1][resources_little];
        auto [neigh_big, neigh_little] = cumm_resources[end_task][resources_big-1][resources_little];
        if ((neigh_load < current_load) ||
            ((neigh_load == current_load) && (neigh_little > current_little) && (neigh_big < current_big)) ||
            ((neigh_load == current_load) && (neigh_little <= current_little) && (neigh_big <= current_big))) {
            current_load = min_loads[end_task][resources_big-1][resources_little];
            auto coord = previous_coordinates[end_task][resources_big-1][resources_little];
            current_coord_big = coord.first;
            current_coord_little = coord.second;
            current_type_of_resource = type_of_resource[end_task][resources_big-1][resources_little];
            auto bl = cumm_resources[end_task][resources_big-1][resources_little];
            current_big = bl.first;
            current_little = bl.second;
            current_start_task = interval_start[end_task][resources_big-1][resources_little];
        }
    }
    // Second step: try all possible starts with different resources
    for (int start_task = end_task; start_task > 0; --start_task){
        // find the minimal solution for big resources
        auto [load_b, big_r_usable] = intervals.compute_load(start_task, end_task, resources_big, Big);
        for (int r_big = 1; r_big <= big_r_usable; ++r_big){
            auto [big_load, big_r_used] = intervals.compute_load(start_task, end_task, r_big, Big);
            double max_big_load = std::max(big_load, min_loads[start_task-1][resources_big-r_big][resources_little]);
            auto [big_big, big_little] = cumm_resources[start_task-1][resources_big-r_big][resources_little];
            big_big += big_r_used;
            // PRIORITIES: < load, > little resources, < resources
            if ((max_big_load < current_load) ||
                ((max_big_load == current_load) && (big_little > current_little) && (big_big < current_big)) ||
                ((max_big_load == current_load) && (big_little <= current_little) && (big_big <= current_big))) {
                    current_load = max_big_load;
                    current_big = big_big;
                    current_little = big_little;
                    current_coord_big =  resources_big-r_big;
                    current_coord_little = big_little;
                    current_type_of_resource = Big;
                    current_start_task = start_task;  
            }
        }
        // find the minimal solution for little resources
        auto [load_l, little_r_usable] = intervals.compute_load(start_task, end_task, resources_little, Little);
        for (int r_little = 1; r_little <= little_r_usable; ++r_little){
            auto [little_load, little_r_used] = intervals.compute_load(start_task, end_task, r_little, Little);
            double max_little_load = std::max(little_load, min_loads[start_task-1][resources_big][resources_little-r_little]);
            auto [little_big, little_little] = cumm_resources[start_task-1][resources_big][resources_little-r_little];
            little_little += little_r_used;
            // PRIORITIES: < load, > little resources, < resources
            if ((max_little_load < current_load) ||
                ((max_little_load == current_load) && (little_little > current_little) && (little_big < current_big)) ||
                ((max_little_load == current_load) && (little_little <= current_little) && (little_big <= current_big))) {
                    current_load = max_little_load;
                    current_big = little_big;
                    current_little = little_little;
                    current_coord_big =  little_big;
                    current_coord_little = resources_little-r_little;
                    current_type_of_resource = Little;
                    current_start_task = start_task;  
            }
        }
    }
    // saves the best solution found for this cell
    min_loads[end_task][resources_big][resources_little] = current_load;
    previous_coordinates[end_task][resources_big][resources_little] = {current_coord_big, current_coord_little};
    type_of_resource[end_task][resources_big][resources_little] = current_type_of_resource;
    cumm_resources[end_task][resources_big][resources_little] = {current_big, current_little};
    interval_start[end_task][resources_big][resources_little] = current_start_task;
}

auto HeRAD::extract_solution() {
    int last_task = num_tasks - 1;
    int r_big = resources_big;
    int r_little = resources_little;
    int start = std::numeric_limits<int>::max();
    double max_load = 0;
    std::vector<std::tuple<int, int, bool, double>> partitions;
    std::vector<std::tuple<int, int, bool, double>> compacted_partitions;

    while (start > 0) {
        auto [big_r_used, little_r_used] = cumm_resources[last_task][r_big][r_little];
        bool r_type = type_of_resource[last_task][r_big][r_little];
        start = interval_start[last_task][r_big][r_little];
        auto [big_before, little_before] = previous_coordinates[last_task][r_big][r_little];

        if (start > 0) {
            auto [cumm_big_r, cumm_little_r] = cumm_resources[start-1][big_before][little_before];
            big_r_used -= cumm_big_r;
            little_r_used -= cumm_little_r;
        }

        int tasks_in_interval = last_task - start + 1;
        int r_used = r_type == Big ? big_r_used : little_r_used;
        auto [load, res] = intervals.compute_load(start, last_task, r_used, r_type);
        partitions.emplace_back(tasks_in_interval, r_used, r_type, load);
        max_load = std::max(max_load, load);  

        r_big = big_before;
        r_little = little_before;
        last_task = start - 1;
    }

    std::reverse(partitions.begin(), partitions.end());
    // compacting stages
    compacted_partitions.emplace_back(partitions[0]);
    int j = 0;
    start = 0;
    for(int i = 1; i < partitions.size(); ++i) {
        auto [prev_tasks, prev_used, prev_type, prev_load] = compacted_partitions[j];
        auto [cur_tasks, cur_used, cur_type, cur_load] = partitions[i];
        if ((prev_type == cur_type) && 
            (interval_state.is_stateless(start, start + prev_tasks - 1)) &&
            (interval_state.is_stateless(start + prev_tasks, start + prev_tasks + cur_tasks - 1))) {
            // can merge the stages
            auto [load, res] = intervals.compute_load(start, 
                                                      start + prev_tasks + cur_tasks - 1,
                                                      prev_used + cur_used,
                                                      prev_type);
            std::tuple<int, int, bool, double> fused(prev_tasks + cur_tasks, prev_used + cur_used, prev_type, load);
            compacted_partitions[j] = fused;
        } else {
            compacted_partitions.emplace_back(partitions[i]);
            start = start + prev_tasks; // start of the next stage
            ++j;
        }
    }

    std::pair<std::vector<std::tuple<int, int, bool, double>>, double> result;
    result.first = compacted_partitions;
    result.second = max_load;
    return result;
}

std::pair<std::vector<std::tuple<int, int, bool, double>>, double> HeRAD::compute_schedule() {
    compute_initial_plane(0);                      // solution for the first task
    for (int end_task = 1; end_task < num_tasks; ++end_task){
        compute_initial_plane(end_task);
        for (int r_big = 0; r_big <= resources_big; ++r_big){
            for (int r_little = 0; r_little <= resources_little; ++r_little){
                if ((r_big == 0) && (r_little == 0))
                    continue;
                recompute_cell(end_task, r_big, r_little);
            }
        }
    }
    return extract_solution();
}
