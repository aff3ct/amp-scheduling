#include "Scheduler/OTAC_Scheduler.h"

OTAC::OTAC(const std::vector<double>& loads, const std::vector<bool>& task_states, int resources, bool r_type)
    : loads(loads), task_states(task_states), resources(resources), r_type(r_type),
    intervals(loads, loads, task_states), interval_state(task_states), stateful_list(task_states) {
    num_tasks = loads.size();
    max_load = *std::max_element(loads.begin(), loads.end());

    // finds the longest stateful task
    max_stateful = 0.0;
    for (int i = 0; i < num_tasks; ++i) {
        if (!task_states[i]) {  // stateful
            max_stateful = std::max(max_stateful, loads[i]);
        }
    }

    // check we have resources to work with
    assert(resources > 0);
    assert(num_tasks > 0);
}

// Compute a schedule for two kinds of resources
std::pair<std::vector<std::tuple<int, int, bool, double>>, double> OTAC::compute_schedule() {
    double min_period = intervals.compute_load(0, num_tasks - 1, 1, r_type).first / resources;
    min_period = std::max(min_period, max_stateful);
    double max_period = min_period + max_load;
    double epsilon = 1.0 / resources;
    StagesOTAC* best_solution;

    while (max_period - min_period >= epsilon) {
        double test_period = (max_period + min_period) / 2.0;
        StagesOTAC* best_result = schedule_given_period(test_period, 0, resources);
        if (best_result->valid) {
            best_solution = best_result;
            max_period = best_result->max_load;
        } else {
            min_period = test_period;
        }
    }
    return extract_solution(best_solution);
}

std::pair<std::vector<std::tuple<int, int, bool, double>>, double> OTAC::extract_solution(StagesOTAC* solution) {
    std::vector<std::tuple<int, int, bool, double>> partitions;
    double max_load = solution->max_load;
    int start_task = 0;
    while (solution != NULL) {
        int tasks_in_interval = solution->num_tasks;
        int r_used = solution->resources;
        int end_task = start_task + tasks_in_interval - 1;
        double load = intervals.compute_load(start_task, end_task, r_used, r_type).first;
        partitions.emplace_back(tasks_in_interval, r_used, r_type, load);
        start_task += tasks_in_interval;
        solution = solution->next_stage;
    }
    std::pair<std::vector<std::tuple<int, int, bool, double>>, double> result;
    result.first = partitions;
    result.second = max_load;
    return result;
} 

// Schedule given a period
StagesOTAC* OTAC::schedule_given_period(double period, int start, int resources) {
    int b = start;
    int e = b + 1;

    // First packing
    while (e < num_tasks && intervals.compute_load(b, e, 1, r_type).first <= period) {
        e += 1;
    }

    int res_needed = std::ceil(intervals.compute_load(b, e - 1, 1, r_type).first / period);

    // If the interval is stateless, more tasks may be added
    if (e < num_tasks && interval_state.is_stateless(b, e - 1)) {
        int next_stateful = stateful_list.next_stateful_task(e - 1);
        if (next_stateful != e) {
            // More tasks can be added
            e = next_stateful;
            res_needed = std::ceil(intervals.compute_load(b, next_stateful - 1, 1, r_type).first / period);
            // We have enough resources for these tasks
            if (next_stateful != num_tasks) {
                // There exists more tasks after this stage
                // Checks if the resources can be better distributed
                int proposed_end = next_stateful - 1;
                while (intervals.compute_load(proposed_end, next_stateful, 1, r_type).first <= period) {
                    proposed_end -= 1;
                }
                if (intervals.compute_load(b, proposed_end, res_needed - 1, r_type).first <= period) {
                    // Better to leave some stateless tasks for the next stage
                    res_needed = res_needed - 1;
                    while (intervals.compute_load(b, proposed_end + 1, res_needed, r_type).first <= period) {
                        proposed_end += 1;
                    }
                    e = proposed_end + 1;
                }
            }
        }
    }

    // Finishes the stage
    // 1. Check if it failed to find a valid stage, return an empty schedule
    if (res_needed > resources){
        StagesOTAC* stages = new StagesOTAC(false, 0, 0, 0.0, NULL);
        return stages;
    }
    int stage = e - b;
    int extra_res = resources - res_needed;
    double load = intervals.compute_load(b, e-1, res_needed, r_type).first;
    // 2. Check if this was the last stage
    if (e == num_tasks) {
        StagesOTAC* stages = new StagesOTAC(true, stage, res_needed, load, NULL);
        return stages;           
    } else {
        // 3. Calls the recursion for the next stage with both types of resources
        StagesOTAC* best_result = schedule_given_period(period, e, extra_res);
        if (best_result->valid == false) {
            StagesOTAC* stages = new StagesOTAC(false, 0, 0, 0.0, NULL);
            return stages;
        } else {
            double max_load = std::max(load, best_result->max_load);
            StagesOTAC* stages = new StagesOTAC(true, stage, res_needed, max_load, best_result);
            return stages;
        }
    }
}