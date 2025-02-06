#include "Support/Test_Generator.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <tuple>

void run_simulation(
    const int starting_seed, 
    const int ending_seed,
    const int num_tasks,
    const int time_min,
    const int time_max,
    const int slowdown_little_min,
    const int slowdown_little_max,
    const std::vector<double> stateless_ratios,
    const std::vector<std::tuple<int, int>> resources,
    const std::vector<std::string> algorithms,
    const std::string scenario_file,
    const std::string results_file) {

    std::ofstream scenario_output, result_output;
    scenario_output.open(scenario_file, std::ios_base::app);
    result_output.open(results_file, std::ios_base::app);
    
    for (int rng_seed = starting_seed; rng_seed < ending_seed; ++rng_seed) {
        for (double stateless_ratio : stateless_ratios) {
            std::cout << "- Running for seed " << rng_seed << " and SR " << stateless_ratio << std::endl;
            auto chain = generate_task_chain(rng_seed, num_tasks, time_min, time_max, slowdown_little_min, slowdown_little_max, stateless_ratio);
            std::vector<double> loads_big = std::get<0>(chain);
            std::vector<double> loads_little = std::get<1>(chain);
            std::vector<bool> states = std::get<2>(chain);

            std::string scenario = scenario_to_string(rng_seed, num_tasks, time_min, time_max, slowdown_little_min, slowdown_little_max, stateless_ratio, loads_big, loads_little, states, ',');
            scenario_output << scenario;

            for (const auto& resource : resources) {
                int resources_big = std::get<0>(resource);
                int resources_little = std::get<1>(resource);
                for (const std::string& algorithm : algorithms) {
                    auto schedule = run_test(loads_big, loads_little, states, resources_big, resources_little, algorithm);
                    std::vector<std::tuple<int, int, bool, double>> partition = schedule.first;
                    double max_load = schedule.second;

                    std::string result = result_to_string(rng_seed, num_tasks, resources_big, resources_little, time_max, stateless_ratio, partition, max_load, algorithm, ',');
                    result_output << result;
                }
            }
        }
    }
    scenario_output.close();
    result_output.close();
}

int main() {
    // Parameters
    const std::vector<std::string> algorithms = {"HeRAD", "2CATAC", "FERTAC", "OTAC_big", "OTAC_little"};
    const std::string scenario_file = "simulation_scenarios.csv";
    const std::string results_file = "simulation_results.csv";
    const int time_min = 1;
    const int slowdown_little_min = 1;
    const int slowdown_little_max = 5;
    const std::vector<double> stateless_ratios = {0.2, 0.5, 0.8};

    std::ofstream scenario_output(scenario_file);
    scenario_output << scenario_head();
    scenario_output.close();
    std::ofstream result_output(results_file);
    result_output << result_head();
    result_output.close();

    // Experiment
    {
        const int number_of_seeds = 1000;
        const int starting_seed = 0;
        const int num_tasks = 20;
        const int time_max = 100;
        const std::vector<std::tuple<int, int>> resources = {{16, 4}, {10, 10}, {4, 16}};

        run_simulation(
            starting_seed, 
            starting_seed + number_of_seeds,
            num_tasks,
            time_min,
            time_max,
            slowdown_little_min,
            slowdown_little_max,
            stateless_ratios,
            resources,
            algorithms,
            scenario_file,
            results_file);
    }

    return 0;
}