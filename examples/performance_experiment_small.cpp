#include "Support/Test_Generator.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <tuple>
#include <chrono>

int main() {
    // Parameters
    const int experiment_seed = 2025;
    const int number_of_seeds = 50;
    const int starting_seed = 0;
    const int time_min = 1;
    const int time_max = 100;
    const int slowdown_little_min = 1;
    const int slowdown_little_max = 5;
    const std::vector<int> num_tasks_possible = {20, 40, 60};
    const std::vector<double> stateless_ratios = {0.2, 0.5, 0.8};
    const std::vector<std::tuple<int, int>> resources = {{20, 20}, {40, 40}, {60,60}, {80,80}, {100,100}};
    std::vector<std::string> algorithms = {"HeRAD", "2CATAC", "FERTAC", "OTAC_big"};
    const std::string results_file = "performance_results_small.csv";
    std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    rng.seed(experiment_seed);

    // Experiments
    std::ofstream result_output(results_file);
    result_output << perf_head();
    result_output.close();


    for (int rng_seed = starting_seed; rng_seed < starting_seed + number_of_seeds; ++rng_seed) {
        for (int num_tasks : num_tasks_possible){
            for (double stateless_ratio : stateless_ratios) {
                auto chain = generate_task_chain(rng_seed, num_tasks, time_min, time_max, slowdown_little_min, slowdown_little_max, stateless_ratio);
                std::vector<double> loads_big = std::get<0>(chain);
                std::vector<double> loads_little = std::get<1>(chain);
                std::vector<bool> states = std::get<2>(chain);

                for (const auto& resource : resources) {
                    int resources_big = std::get<0>(resource);
                    int resources_little = std::get<1>(resource);
                    std::cout << "- Running for seed " << rng_seed << ", SR " << stateless_ratio << 
                              ", "<< num_tasks << " tasks and " << resources_big << " resources" << std::endl;
                    // random order of algorithms
                    std::shuffle(algorithms.begin(), algorithms.end(), rng);
                    for (std::string algorithm : algorithms) {
                        auto start_time = std::chrono::steady_clock::now();
                        auto schedule = run_test(loads_big, loads_little, states, resources_big, resources_little, algorithm);
                        auto end_time = std::chrono::steady_clock::now();
                        auto total_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
                        std::vector<std::tuple<int, int, bool, double>> partition = schedule.first;
                        double max_load = schedule.second;

                        std::string result = perf_to_string(rng_seed, 
                                                            num_tasks, 
                                                            resources_big, 
                                                            resources_little, 
                                                            stateless_ratio,
                                                            algorithm, 
                                                            total_time,
                                                            ',');
                        result_output.open(results_file, std::ios_base::app);
                        result_output << result;
                        result_output.close();                        
                    }
                }
            }
        }
    }

    return 0;
}