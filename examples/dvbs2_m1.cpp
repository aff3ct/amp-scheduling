#include "Support/Test_Generator.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <tuple>

#define SL true
#define SF false

/*
./bin/dvbs2_rx_sched --sim-stats --src-type USER --src-path ../conf/src/K_14232.src --rad-type USER_BIN --rad-rx-file-path /scratch/cassagnea-nfs/out_tx.bin -F 4 --mod-cod QPSK-S_8/9 --dec-implem NMS --dec-ite 10 --dec-simd INTER --snk-path /dev/null --rad-rx-no-loop -T FILE -J ../sched.json -P 1000
times in x10ns (x0.01us)
MODCOD: QPSK-S_8/9
interframe: 4 (NEON 128-bit)
no noise, ideal conditions for LDPC and BCH decoders 
*/
const std::vector<std::string> task_names = {"receive",
                                             "imultiply",
                                             "synchronize",
                                             "filter1",
                                             "filter2",
                                             "synchronize",
                                             "extract",
                                             "imultiply",
                                             "synchronize1",
                                             "synchronize2",
                                             "descramble",
                                             "synchronize",
                                             "synchronize",
                                             "remove_plh",
                                             "estimate",
                                             "demodulate",
                                             "deinterleave",
                                             "decode_siho",
                                             "decode_hiho",
                                             "descramble",
                                             "send",
                                             "generate",
                                             "check_errors2"};
/* 
# On PUID n°2
# ---------------------------------------------------------------||------------------------------||--------------------------------
#                  Statistics for the given task                 ||       Basic statistics       ||        Measured latency
#               ('*' = any, '-' = same as previous)              ||          on the task         ||
# ---------------------------------------------------------------||------------------------------||--------------------------------
# -------------------|-------------------|-----|-------|---------||----------|----------|--------||----------|----------|----------
#        MODULE NAME |         TASK NAME | REP | ORDER |   TIMER ||    CALLS |     TIME |   PERC ||  AVERAGE |  MINIMUM |  MAXIMUM
#                    |                   |     |       |         ||          |      (s) |    (%) ||     (us) |     (us) |     (us)
# -------------------|-------------------|-----|-------|---------||----------|----------|--------||----------|----------|----------
#              Radio |           receive |  no |     0 |       * ||     1000 |     0.05 |   0.61 ||    52.32 |    39.46 |   101.58
#         Multiplier |         imultiply |  no |     1 |       * ||     1000 |     0.08 |   0.88 ||    75.21 |    74.38 |    96.54
#       Coarse_Synch |       synchronize |  no |     2 |       * ||     1000 |     0.10 |   1.13 ||    96.44 |    95.87 |   109.21
#        Matched Flt |           filter1 |  no |     3 |       * ||     1000 |     0.32 |   3.74 ||   318.90 |   317.67 |   341.71
#        Matched Flt |           filter2 |  no |     4 |       * ||     1000 |     0.32 |   3.69 ||   315.05 |   313.50 |   338.58
#        Gardner Syn |       synchronize |  no |     5 |       * ||     1000 |     0.95 |  11.14 ||   950.59 |   947.50 |   992.04
#        Gardner Syn |           extract |  no |     6 |       * ||     1000 |     0.06 |   0.65 ||    55.45 |    53.21 |    87.50
#           Mult agc |         imultiply |  no |     7 |       * ||     1000 |     0.04 |   0.44 ||    37.12 |    36.71 |    45.25
#          Frame Syn |      synchronize1 |  no |     8 |       * ||     1000 |     0.36 |   4.23 ||   361.02 |   359.79 |   377.75
#          Frame Syn |      synchronize2 |  no |     9 |       * ||     1000 |     0.05 |   0.62 ||    52.88 |    52.04 |    64.79
#       Scrambler_PL |        descramble | yes |    10 |       * ||     1000 |     0.02 |   0.19 ||    16.02 |    15.67 |    25.33
#          L&R F Syn |       synchronize |  no |    11 |       * ||     1000 |     0.05 |   0.59 ||    50.45 |    49.83 |    63.29
#       Fine P/F Syn |       synchronize | yes |    12 |       * ||     1000 |     0.10 |   1.16 ||    99.20 |    98.33 |   113.58
#             Framer |        remove_plh | yes |    13 |       * ||     1000 |     0.02 |   0.27 ||    23.36 |    22.67 |    31.54
#          Estimator |          estimate | yes |    14 |       * ||     1000 |     0.04 |   0.47 ||    40.47 |    40.17 |    48.00
#              Modem |        demodulate | yes |    15 |       * ||     1000 |     2.26 |  26.46 ||  2257.49 |  2252.87 |  2336.04
#        Interleaver |      deinterleave | yes |    16 |       * ||     1000 |     0.02 |   0.25 ||    21.08 |    20.46 |    34.42
#       LDPC Decoder |       decode_siho | yes |    17 |       * ||     1000 |     0.15 |   1.80 ||   153.18 |   151.42 |   179.17
#        BCH Decoder |       decode_hiho | yes |    18 |       * ||     1000 |     3.34 |  39.15 ||  3339.86 |  3294.87 |  3587.08
#       Scrambler_BB |        descramble | yes |    19 |       * ||     1000 |     0.19 |   2.25 ||   191.67 |   178.17 |   205.33
#               Sink |              send |  no |    20 |       * ||     1000 |     0.01 |   0.11 ||     9.54 |     8.96 |    16.92
#             Source |          generate |  no |    21 |       * ||     1000 |     0.00 |   0.05 ||     4.04 |     3.58 |    13.17
#            Monitor |     check_errors2 | yes |    22 |       * ||     1000 |     0.01 |   0.11 ||     9.47 |     9.04 |    14.00
# -------------------|-------------------|-----|-------|---------||----------|----------|--------||----------|----------|----------
#              TOTAL |                 * |  no |     * |       * ||     1000 |     8.53 | 100.00 ||  8530.79 |  8436.15 |  9222.81
*/
const std::vector<double> loads_big = {5232,7521,9644,31890,31505,95059,5545,3712,36102,5288,1602,5045,9920,2336,4047,225749,2108,15318,333986,19167,954,404,947};

/*
# Profiling:
# On PUID n°0
# ---------------------------------------------------------------||------------------------------||--------------------------------
#                  Statistics for the given task                 ||       Basic statistics       ||        Measured latency
#               ('*' = any, '-' = same as previous)              ||          on the task         ||
# ---------------------------------------------------------------||------------------------------||--------------------------------
# -------------------|-------------------|-----|-------|---------||----------|----------|--------||----------|----------|----------
#        MODULE NAME |         TASK NAME | REP | ORDER |   TIMER ||    CALLS |     TIME |   PERC ||  AVERAGE |  MINIMUM |  MAXIMUM
#                    |                   |     |       |         ||          |      (s) |    (%) ||     (us) |     (us) |     (us)
# -------------------|-------------------|-----|-------|---------||----------|----------|--------||----------|----------|----------
#              Radio |           receive |  no |     0 |       * ||     1000 |     0.25 |   1.25 ||   248.27 |   236.75 |   398.62
#         Multiplier |         imultiply |  no |     1 |       * ||     1000 |     0.15 |   0.76 ||   149.94 |   148.12 |   234.33
#       Coarse_Synch |       synchronize |  no |     2 |       * ||     1000 |     0.50 |   2.50 ||   496.64 |   483.42 |   546.21
#        Matched Flt |           filter1 |  no |     3 |       * ||     1000 |     0.90 |   4.55 ||   902.90 |   897.21 |   963.87
#        Matched Flt |           filter2 |  no |     4 |       * ||     1000 |     0.88 |   4.45 ||   883.16 |   878.62 |   965.08
#        Gardner Syn |       synchronize |  no |     5 |       * ||     1000 |     1.47 |   7.40 ||  1468.87 |  1451.04 |  1565.46
#        Gardner Syn |           extract |  no |     6 |       * ||     1000 |     0.11 |   0.53 ||   105.95 |    98.17 |   156.17
#           Mult agc |         imultiply |  no |     7 |       * ||     1000 |     0.08 |   0.38 ||    75.41 |    74.79 |   104.25
#          Frame Syn |      synchronize1 |  no |     8 |       * ||     1000 |     1.06 |   5.37 ||  1064.66 |  1059.00 |  1153.46
#          Frame Syn |      synchronize2 |  no |     9 |       * ||     1000 |     0.17 |   0.85 ||   169.12 |   162.04 |   213.25
#       Scrambler_PL |        descramble | yes |    10 |       * ||     1000 |     0.06 |   0.31 ||    61.02 |    60.62 |    92.88
#          L&R F Syn |       synchronize |  no |    11 |       * ||     1000 |     0.25 |   1.25 ||   247.12 |   244.58 |   330.17
#       Fine P/F Syn |       synchronize | yes |    12 |       * ||     1000 |     0.60 |   3.01 ||   597.82 |   259.04 |   755.25
#             Framer |        remove_plh | yes |    13 |       * ||     1000 |     0.07 |   0.33 ||    65.08 |    64.00 |   107.33
#          Estimator |          estimate | yes |    14 |       * ||     1000 |     0.07 |   0.33 ||    65.35 |    64.17 |   120.54
#              Modem |        demodulate | yes |    15 |       * ||     1000 |     4.84 |  24.39 ||  4838.64 |  4821.57 |  4949.78
#        Interleaver |      deinterleave | yes |    16 |       * ||     1000 |     0.06 |   0.29 ||    58.38 |    57.79 |    71.46
#       LDPC Decoder |       decode_siho | yes |    17 |       * ||     1000 |     0.51 |   2.55 ||   506.70 |   466.58 |  3536.87
#        BCH Decoder |       decode_hiho | yes |    18 |       * ||     1000 |     7.30 |  36.81 ||  7303.49 |  7180.86 | 11132.52
#       Scrambler_BB |        descramble | yes |    19 |       * ||     1000 |     0.46 |   2.34 ||   464.86 |   462.87 |   559.00
#               Sink |              send |  no |    20 |       * ||     1000 |     0.03 |   0.17 ||    33.34 |    30.88 |    44.63
#             Source |          generate |  no |    21 |       * ||     1000 |     0.01 |   0.07 ||    13.57 |    13.17 |    16.75
#            Monitor |     check_errors2 | yes |    22 |       * ||     1000 |     0.02 |   0.11 ||    21.00 |    20.50 |    35.58
# -------------------|-------------------|-----|-------|---------||----------|----------|--------||----------|----------|----------
#              TOTAL |                 * |  no |     * |       * ||     1000 |    19.84 | 100.00 || 19841.29 | 19235.79 | 28053.44
*/
const std::vector<double> loads_little = {24827,14994,49664,90290,88316,146887,10595,7541,106466,16912,6102,24712,59782,6508,6535,483864,5838,50670,730349,46486,3334,1357,2100};
const std::vector<bool> task_states = {SF,SF,SF,SF,SF,SF,SF,SF,SF,SF,SL,SF,SL,SL,SL,SL,SL,SL,SL,SL,SF,SF,SL};

void print_solution(
    std::vector<std::tuple<int, int, bool, double>> partition,
    int resources_big,
    int resources_little,
    std::string algorithm) {
    
    std::ofstream solution_output("m1_" + algorithm + "_" + 
                                  std::to_string(resources_big) + "big_" + 
                                  std::to_string(resources_little) + "little.json");

    solution_output << "[" << std::endl;
    for (int i = 0; i < partition.size(); ++i) {
        auto stage = partition[i];
        int tasks, r_used;
        bool r_type;
        double load;
        std::tie(tasks, r_used, r_type, load) = stage;
        solution_output << "  { \"tasks\": " << tasks << ", \"cores\": [";
        char core = r_type ? 'B' : 'L';
        for (int r = 0; r < r_used; ++r) {
            solution_output << core;
            if (r < r_used - 1) {
                solution_output << ", ";
            }
        }
        solution_output << "] }";
        if (i < partition.size() - 1 ) {
            solution_output << ",";
        }
        solution_output << std::endl;
    }
    solution_output << "]" << std::endl;
    solution_output.close();
}

int main() {

    // Algorithms and resource scenarios
    const std::vector<std::tuple<int, int>> resources = {{16, 4}, {8, 2}};
    const std::vector<std::string> algorithms = {"HeRAD", "2CATAC", "FERTAC", "OTAC_big", "OTAC_little"};

    const std::string results_file = "m1_results.csv";
    std::ofstream result_output(results_file);
    result_output << result_head();

    for (const auto& resource : resources) {
        int resources_big = std::get<0>(resource);
        int resources_little = std::get<1>(resource);
        for (const std::string& algorithm : algorithms) {
            auto schedule = run_test(loads_big, loads_little, task_states, resources_big, resources_little, algorithm);
            std::vector<std::tuple<int, int, bool, double>> partition = schedule.first;
            double max_load = schedule.second;

            std::string result = result_to_string(0, 22, resources_big, resources_little, 0, 0, partition, max_load, algorithm, ',');
            result_output << result;
            print_solution(partition, resources_big, resources_little, algorithm);
        }
    }
        
    result_output.close();
    return 0;
}