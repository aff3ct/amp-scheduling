#pragma once

#include <iostream>
#include <vector>
#include <numeric>
#include <cassert>
#include <limits>

class StatefulList {
public:
    StatefulList(const std::vector<bool>& task_states) {
        int tasks = task_states.size();
        next_stateful.resize(tasks);
        int start = 0;
        int end = 1;
        while (end < tasks) {
            if (!task_states[end]) { // False if stateful
                for (int i = start; i < end; ++i) {
                    next_stateful[i] = end;
                }
                start = end;
            }
            end += 1;
        }
        for (int i = start; i < end; ++i) {
            next_stateful[i] = tasks;
        }
    }

    int next_stateful_task(int task) const {
        return next_stateful[task];
    }

private:
    std::vector<int> next_stateful;
};

class IntervalState {
public:
    IntervalState(const std::vector<bool>& task_states) {
        int tasks = task_states.size();
        state_matrix.resize(tasks, std::vector<bool>(tasks, false));
        for (int i = 0; i < tasks; ++i) {
            state_matrix[0][i] = task_states[i];
        }
        for (int i = 1; i < tasks; ++i) {
            for (int j = i; j < tasks; ++j) {
                state_matrix[i][j] = state_matrix[i-1][j-1] && state_matrix[0][j];
            }
        }
    }

    bool is_stateless(int start_task, int end_task) const {
        return state_matrix[end_task - start_task][end_task];
    }

private:
    std::vector<std::vector<bool>> state_matrix;
};

class IntervalLoad {
public:
    IntervalLoad(const std::vector<double>& loads_big, const std::vector<double>& loads_little, const std::vector<bool>& task_states)
        : state(task_states) {
        assert(loads_big.size() == loads_little.size());
        assert(loads_big.size() == task_states.size());

        cumm_load_big.resize(loads_big.size());
        cumm_load_little.resize(loads_little.size());

        std::partial_sum(loads_big.begin(), loads_big.end(), cumm_load_big.begin());
        std::partial_sum(loads_little.begin(), loads_little.end(), cumm_load_little.begin());
    }

    std::pair<double, int> compute_load(int start_task, int end_task, int num_resources, bool big = true) const {
        if (num_resources < 1) {
            return {std::numeric_limits<double>::infinity(), 0};
        }

        const auto& load_source = big ? cumm_load_big : cumm_load_little;
        double end_load = load_source[end_task];
        double start_load = (start_task > 0) ? load_source[start_task - 1] : 0;
        double total_load = end_load - start_load;

        if (state.is_stateless(start_task, end_task)) {
            total_load /= num_resources;
        } else {
            num_resources = 1;
        }

        return {total_load, num_resources};
    }

private:
    std::vector<double> cumm_load_big;
    std::vector<double> cumm_load_little;
    IntervalState state;
};