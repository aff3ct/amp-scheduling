#include "Scheduler/Greedy_Scheduler.h"

TwoCATAC::TwoCATAC(const std::vector<double>& loads_big, const std::vector<double>& loads_little, const std::vector<bool>& task_states,
        int resources_big, int resources_little)
    : loads_big(loads_big), loads_little(loads_little), task_states(task_states), resources_big(resources_big), resources_little(resources_little),
    intervals(loads_big, loads_little, task_states), interval_state(task_states), stateful_list(task_states) {
    num_tasks = loads_big.size();
    max_load = *std::max_element(loads_big.begin(), loads_big.end());
    max_load = std::max(max_load, *std::max_element(loads_little.begin(), loads_little.end()));

    // finds the longest stateful task for each type of resource and takes their minimal
    double max_big = 0;
    for (int i = 0; i < num_tasks; ++i) {
        if (!task_states[i]) {  // stateful
            max_big = std::max(max_big, loads_big[i]);
        }
    }
    max_stateful = max_big;

    // check we have resources to work with
    assert(resources_big > 0 && resources_little > 0);
    assert(num_tasks > 0);
}

// Compute a schedule for two kinds of resources
std::pair<std::vector<std::tuple<int, int, bool, double>>, double> TwoCATAC::compute_schedule() {
    int resources = resources_big + resources_little;
    double min_period = intervals.compute_load(0, num_tasks - 1, 1, Big).first / resources;
    min_period = std::max(min_period, max_stateful);
    double max_period = min_period + max_load;
    double epsilon = 1.0 / resources;
    Stages* best_solution;

    while (max_period - min_period >= epsilon) {
        double test_period = (max_period + min_period) / 2.0;
        Stages* result_big = schedule_given_period(test_period, 0, resources_big, resources_little, Big);
        Stages* result_little = schedule_given_period(test_period, 0, resources_little, resources_big, Little);
        // Choose the best solution
        bool big_best = first_best_solution(result_big, result_little);
        if (big_best) {
            if (result_big->valid) {
                best_solution = result_big;
                max_period = result_big->max_load;
                free_schedule(result_little);
            } else {
                min_period = test_period;
            }
        } else {
            if (result_little->valid) {
                best_solution = result_little;
                max_period = result_little->max_load;
                free_schedule(result_big);
            } else {
                min_period = test_period;            
            }
        }
    }
    return extract_solution(best_solution);
}

std::pair<std::vector<std::tuple<int, int, bool, double>>, double> TwoCATAC::extract_solution(Stages* solution) {
    std::vector<std::tuple<int, int, bool, double>> partitions;
    double max_load = solution->max_load;
    int start_task = 0;
    while (solution != NULL) {
        int tasks_in_interval = solution->num_tasks;
        int r_used = solution->resources;
        bool r_type = solution->r_type;
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
Stages* TwoCATAC::schedule_given_period(double period, int start, int resources, int other_resources, bool r_type) {
    int b = start;
    std::pair<int, int> stage_solution = compute_stage(period, start, resources, r_type);
    int e = stage_solution.first;
    int res_needed = stage_solution.second;

    // Finishes the stage
    int stage = e - b;
    int extra_res = resources - res_needed;
    // 1. Check if it failed to find a valid stage, return an empty schedule
    if ((res_needed > resources) || (b >= e)){
        Stages* stages = new Stages(false, 0, 0, false, 0.0, 0, 0, NULL);
        return stages;
    }
    double load = intervals.compute_load(b, e-1, res_needed, r_type).first;
    if (load > period){
        Stages* stages = new Stages(false, 0, 0, false, 0.0, 0, 0, NULL);
        return stages;
    }
    // 2. Check if this was the last stage
    if (e == num_tasks) {
        int big_res = r_type == Big ? res_needed : 0;
        int little_res = r_type == Little ? res_needed : 0;
        Stages* stages = new Stages(true, stage, res_needed, r_type, load, little_res, big_res, NULL);
        return stages;           
    } else {
        // 3. Calls the recursion for the next stage with both types of resources
        Stages* schedule_current = schedule_given_period(period, e, extra_res, other_resources, r_type);
        Stages* schedule_other = schedule_given_period(period, e, other_resources, extra_res, !r_type);
        bool current_best = first_best_solution(schedule_current, schedule_other);
        if (current_best) {
            free_schedule(schedule_other);
            if (schedule_current->valid == false) {
                free_schedule(schedule_current);
                Stages* stages = new Stages(false, 0, 0, false, 0.0, 0, 0, NULL);
                return stages;
            } else {
                int big_res = r_type == Big ? res_needed + schedule_current->total_big : schedule_current->total_big;
                int little_res = r_type == Little ? res_needed + schedule_current->total_little : schedule_current->total_little;
                double max_load = std::max(load, schedule_current->max_load);
                Stages* stages = new Stages(true, stage, res_needed, r_type, max_load, little_res, big_res, schedule_current);
                return stages;
            }            
        } else {
            free_schedule(schedule_current);
            if (schedule_other->valid == false) {
                free_schedule(schedule_other);
                Stages* stages = new Stages(false, 0, 0, false, 0.0, 0, 0, NULL);
                return stages;
            } else {
                int big_res = r_type == Big ? res_needed + schedule_other->total_big : schedule_other->total_big;
                int little_res = r_type == Little ? res_needed + schedule_other->total_little : schedule_other->total_little;
                double max_load = std::max(load, schedule_other->max_load);
                Stages* stages = new Stages(true, stage, res_needed, r_type, max_load, little_res, big_res, schedule_other);
                return stages;
            } 
        }
    }
}

bool TwoCATAC::first_best_solution(Stages* first_schedule, Stages* second_schedule) {
    // First scenario: at least one of the schedules failed
    if (!first_schedule->valid) {
        return false;
    } else {
        if(!second_schedule->valid) {
            return true;
        }
    }
    // Second scenario: compare the schedules based on
    // 1. total number of little resources used (the larger, the better)
    if ((first_schedule->total_little > second_schedule->total_little) && (first_schedule->total_big < second_schedule->total_big)){
        return true;
    } else {
        if ((first_schedule->total_little < second_schedule->total_little) && (first_schedule->total_big > second_schedule->total_big)){
            return false;
        }
    }
    // 2. total number of resources used (the smaller, the better)
    if (first_schedule->total_little + first_schedule->total_big < second_schedule->total_little + second_schedule->total_big) {
        return true;
    } else {
        return false;
    }
}

void TwoCATAC::free_schedule(Stages* schedule){
    std::stack<Stages*> to_free;
    while (schedule != NULL) {
        to_free.push(schedule);
        schedule = schedule->next_stage;
    }
    while (!to_free.empty()) {
        schedule = to_free.top();
        to_free.pop();
        free(schedule);
    }
}

std::pair<int, int> TwoCATAC::compute_stage(double period, int start, int resources, bool r_type) {
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
            if (res_needed > resources) {
                // We do not have enough resources for these tasks, so we remove tasks from the stage
                res_needed = resources;
                while (b < e && intervals.compute_load(b, e - 1, res_needed, r_type).first > period) {
                    e -= 1;
                }
                if (b < e) {
                    res_needed = std::ceil(intervals.compute_load(b, e - 1, 1, r_type).first / period);
                }
            } else {
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
    }
    std::pair<int, int> stage = {e, res_needed};
    return stage;
}


FERTAC::FERTAC(const std::vector<double>& loads_big, const std::vector<double>& loads_little, const std::vector<bool>& task_states,
        int resources_big, int resources_little) : TwoCATAC(loads_big, loads_little, task_states, resources_big, resources_little){};

// Compute a schedule for two kinds of resources
std::pair<std::vector<std::tuple<int, int, bool, double>>, double> FERTAC::compute_schedule() {
    int resources = resources_big + resources_little;
    double min_period = intervals.compute_load(0, num_tasks - 1, 1, Big).first / resources;
    min_period = std::max(min_period, max_stateful);
    double max_period = min_period + max_load;
    double epsilon = 1.0 / resources;
    Stages* best_solution;

    while (max_period - min_period >= epsilon) {
        double test_period = (max_period + min_period) / 2.0;
        Stages* best_result = schedule_given_period(test_period, 0, resources_big, resources_little);
        if (best_result->valid) {
            best_solution = best_result;
            max_period = best_result->max_load;
        } else {
            min_period = test_period;
        }
    }
    return extract_solution(best_solution);
}

// Schedule given a period
Stages* FERTAC::schedule_given_period(double period, int start, int resources_big, int resources_little) {
    int b = start;
    int e;
    int res_needed;
    std::pair<int, int> stage_solution;
    bool try_big = false;
    bool r_type;

    // First tries scheduling with little resources, then big resources if necessary
    if (resources_little > 0) {
        r_type = Little;
        stage_solution = compute_stage(period, start, resources_little, Little);
        e = stage_solution.first;
        res_needed = stage_solution.second;
        int stage = e - b;
        // 1. Check if it failed to find a valid stage
        if ((res_needed > resources_little) || (b >= e)){
            try_big = true;
        } else {
            double load = intervals.compute_load(b, e-1, res_needed, Little).first;
            if (load > period){
                try_big = true;
            }
        }
    } else {
        try_big = true;
    }
    if (try_big) { // Could not schedule with little resources
        r_type = Big;
        stage_solution = compute_stage(period, start, resources_big, Big);
        e = stage_solution.first;
        res_needed = stage_solution.second;
    }

    // Finishes the stage
    int stage = e - b;
    int resources = r_type == Big ? resources_big : resources_little;
    // 1. Check if it failed to find a valid stage, return an empty schedule
    if ((res_needed > resources) || (b >= e)){
        Stages* stages = new Stages(false, 0, 0, false, 0.0, 0, 0, NULL);
        return stages;
    }
    double load = intervals.compute_load(b, e-1, res_needed, r_type).first;
    if (load > period){
        Stages* stages = new Stages(false, 0, 0, false, 0.0, 0, 0, NULL);
        return stages;
    }
    // 2. Check if this was the last stage
    if (e == num_tasks) {
        int big_res = r_type == Big ? res_needed : 0;
        int little_res = r_type == Little ? res_needed : 0;
        Stages* stages = new Stages(true, stage, res_needed, r_type, load, little_res, big_res, NULL);
        return stages;           
    } else {
        // 3. Calls the recursion for the next stage with the remaining resources
        int remaining_big = r_type == Big ? resources_big - res_needed : resources_big;
        int remaining_little = r_type == Little ? resources_little - res_needed : resources_little;
        Stages* schedule_next = schedule_given_period(period, e, remaining_big, remaining_little);
        if (schedule_next->valid == false) {
                free_schedule(schedule_next);
                Stages* stages = new Stages(false, 0, 0, false, 0.0, 0, 0, NULL);
                return stages;            
        } else {
                int big_res = r_type == Big ? res_needed + schedule_next->total_big : schedule_next->total_big;
                int little_res = r_type == Little ? res_needed + schedule_next->total_little : schedule_next->total_little;
                double max_load = std::max(load, schedule_next->max_load);
                Stages* stages = new Stages(true, stage, res_needed, r_type, max_load, little_res, big_res, schedule_next);
                return stages;            
        }
    }
}