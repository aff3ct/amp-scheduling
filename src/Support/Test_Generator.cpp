#include "Support/Test_Generator.h"

std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());

std::tuple<std::vector<double>, std::vector<double>, std::vector<bool>> generate_task_chain(
    int rng_seed,
    int num_tasks,
    int time_min,
    int time_max,
    double slowdown_little_min,
    double slowdown_little_max,
    double stateless_ratio) {

    rng.seed(rng_seed);
    std::uniform_int_distribution<int> dist_time(time_min, time_max);
    std::uniform_real_distribution<double> dist_slowdown(slowdown_little_min, slowdown_little_max);

    std::vector<double> loads_big(num_tasks);
    std::vector<double> loads_little(num_tasks);
    std::vector<bool> states(num_tasks);

    for (int i = 0; i < num_tasks; ++i) {
        loads_big[i] = dist_time(rng);
        loads_little[i] = std::ceil(loads_big[i] * dist_slowdown(rng));
        states[i] = i < num_tasks * stateless_ratio;
    }

    std::shuffle(states.begin(), states.end(), rng);

    return std::make_tuple(loads_big, loads_little, states);
}

std::pair<std::vector<std::tuple<int, int, bool, double>>, double> run_test(
    std::vector<double> loads_big,
    std::vector<double> loads_little,
    std::vector<bool> states,
    int resources_big,
    int resources_little,
    std::string algorithm) {

    if (algorithm == "HeRAD") {
        HeRAD scheduler(loads_big, loads_little, states, resources_big, resources_little);
        return scheduler.compute_schedule();
    } else if (algorithm == "2CATAC") {
        TwoCATAC scheduler(loads_big, loads_little, states, resources_big, resources_little);
        return scheduler.compute_schedule();
    } else if (algorithm == "FERTAC") {
        FERTAC scheduler(loads_big, loads_little, states, resources_big, resources_little);
        return scheduler.compute_schedule();
    } else if (algorithm == "OTAC_big") {
        OTAC scheduler(loads_big, states, resources_big, true);
        return scheduler.compute_schedule();
    } else if (algorithm == "OTAC_little") {
        OTAC scheduler(loads_little, states, resources_little, false);
        return scheduler.compute_schedule();
    }

    return std::make_pair(std::vector<std::tuple<int, int, bool, double>>(), 0.0);
}

std::string scenario_head() {
    return "RNG seed,number of tasks,stateless ratio,min task time,max task time,min slowdown,max slowdown,loads in big resources,loads in small resources,task states\n";
}

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
    char SEP) {

    std::ostringstream oss;
    oss << rng_seed << SEP << num_tasks << SEP << stateless_ratio << SEP << time_min << SEP << time_max << SEP << slowdown_little_min << SEP << slowdown_little_max << SEP;

    for (size_t i = 0; i < loads_big.size(); ++i) {
        if (i != 0) oss << "|";
        oss << loads_big[i];
    }
    oss << SEP;

    for (size_t i = 0; i < loads_little.size(); ++i) {
        if (i != 0) oss << "|";
        oss << loads_little[i];
    }
    oss << SEP;

    for (size_t i = 0; i < states.size(); ++i) {
        if (i != 0) oss << "|";
        oss << (states[i] ? "SL" : "SF");
    }
    oss << SEP << "\n";

    return oss.str();
}

std::string result_head() {
    return "RNG seed,algorithm,number of tasks,big resources,little resources,time max,stateless ratio,period,big used,little used,number of stages,tasks per stage,resources per stage,types of resources,load per stage\n";
}

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
    char SEP) {

    int sum_big = 0;
    int sum_little = 0;
    std::vector<int> tasks_in_stages;
    std::vector<int> resources_in_stages;
    std::vector<std::string> resource_type_in_stages;
    std::vector<double> load_in_stages;

    for (const auto& stage : partition) {
        int tasks, r_used;
        bool r_type;
        double load;
        std::tie(tasks, r_used, r_type, load) = stage;
        tasks_in_stages.push_back(tasks);
        load_in_stages.push_back(load);
        resources_in_stages.push_back(r_used);
        if (r_type) {
            sum_big += r_used;
            resource_type_in_stages.push_back("Big");
        } else {
            sum_little += r_used;
            resource_type_in_stages.push_back("Little");
        }
    }

    std::ostringstream oss;
    oss << rng_seed << SEP << algorithm << SEP << num_tasks << SEP << resources_big << SEP << resources_little << SEP << time_max
        << SEP << stateless_ratio << SEP << max_load << SEP << sum_big << SEP << sum_little << SEP << partition.size() << SEP;

    for (size_t i = 0; i < tasks_in_stages.size(); ++i) {
        if (i != 0) oss << "|";
        oss << tasks_in_stages[i];
    }
    oss << SEP;

    for (size_t i = 0; i < resources_in_stages.size(); ++i) {
        if (i != 0) oss << "|";
        oss << resources_in_stages[i];
    }
    oss << SEP;

    for (size_t i = 0; i < resource_type_in_stages.size(); ++i) {
        if (i != 0) oss << "|";
        oss << resource_type_in_stages[i];
    }
    oss << SEP;

    for (size_t i = 0; i < load_in_stages.size(); ++i) {
        if (i != 0) oss << "|";
        oss << load_in_stages[i];
    }
    oss << SEP << "\n";

    return oss.str();
}

std::string perf_head() {
    return "RNG seed,algorithm,number of tasks,big resources,little resources,stateless ratio,time (us)\n";
}

std::string perf_to_string(
    int rng_seed, 
    int num_tasks, 
    int resources_big, 
    int resources_little, 
    double stateless_ratio, 
    std::string algorithm, 
    long total_time,
    char SEP) {
    
    std::ostringstream oss;
    oss << rng_seed << SEP << algorithm << SEP << num_tasks << SEP << resources_big << SEP
        << resources_little << SEP << stateless_ratio << SEP << total_time << SEP << "\n";

    return oss.str();
}