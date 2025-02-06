#pragma once
#include "Scheduler/DP_Scheduler.h"
#include "Scheduler/Greedy_Scheduler.h"
#include "Scheduler/OTAC_Scheduler.h"

#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <sstream>
#include <chrono>

std::tuple<std::vector<double>, std::vector<double>, std::vector<bool>> generate_task_chain(
    int rng_seed,
    int num_tasks,
    int time_min,
    int time_max,
    double slowdown_little_min,
    double slowdown_little_max,
    double stateless_ratio);

std::pair<std::vector<std::tuple<int, int, bool, double>>, double> run_test(
    std::vector<double> loads_big,
    std::vector<double> loads_little,
    std::vector<bool> states,
    int resources_big,
    int resources_little,
    std::string algorithm);

std::string scenario_head();

std::string scenario_to_string(
    int rng_seed,
    int num_tasks,
    int time_min,
    int time_max,
    double slowdown_little_min,
    double slowdown_little_max,
    double stateless_ratio,
    std::vector<double> loads_big,
    std::vector<double> loads_little,
    std::vector<bool> states,
    char SEP);

std::string result_head();

std::string result_to_string(
    int rng_seed,
    int num_tasks,
    int resources_big,
    int resources_little,
    int time_max,
    double stateless_ratio,
    std::vector<std::tuple<int, int, bool, double>> partition,
    double max_load,
    std::string algorithm,
    char SEP);

std::string perf_head();

std::string perf_to_string(
    int rng_seed, 
    int num_tasks, 
    int resources_big, 
    int resources_little, 
    double stateless_ratio, 
    std::string algorithm, 
    long total_time,
    char SEP);