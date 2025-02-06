#include "Support/Test_Generator.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <tuple>

#define SL true
#define SF false

/*
./bin/dvbs2_rx_sched --sim-stats --src-type USER --src-path ../conf/src/K_14232.src --rad-type USER_BIN --rad-rx-file-path /scratch/cassagnea-nfs/out_tx.bin -F 8 --mod-cod QPSK-S_8/9 --dec-implem NMS --dec-ite 10 --dec-simd INTER --snk-path /dev/null --rad-rx-no-loop -T FILE -J ../sched.json -P 1000
MODCOD: QPSK-S_8/9
interframe: 8 (AVX 256-bit)
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
#              Radio |           receive |  no |     0 |       * ||     1000 |     0.13 |   1.05 ||   131.65 |   114.25 |   239.82
#         Multiplier |         imultiply |  no |     1 |       * ||     1000 |     0.14 |   1.10 ||   138.32 |   125.08 |   507.38
#       Coarse_Synch |       synchronize |  no |     2 |       * ||     1000 |     0.11 |   0.90 ||   113.69 |   105.71 |   381.12
#        Matched Flt |           filter1 |  no |     3 |       * ||     1000 |     0.33 |   2.66 ||   334.79 |   309.35 |   785.15
#        Matched Flt |           filter2 |  no |     4 |       * ||     1000 |     0.33 |   2.62 ||   329.30 |   317.18 |   781.01
#        Gardner Syn |       synchronize |  no |     5 |       * ||     1000 |     1.34 |  10.66 ||  1341.90 |  1333.59 |  1494.96
#        Gardner Syn |           extract |  no |     6 |       * ||     1000 |     0.06 |   0.47 ||    58.68 |    50.06 |   155.73
#           Mult agc |         imultiply |  no |     7 |       * ||     1000 |     0.06 |   0.50 ||    63.45 |    61.55 |    92.75
#          Frame Syn |      synchronize1 |  no |     8 |       * ||     1000 |     0.37 |   2.91 ||   365.93 |   360.21 |   630.72
#          Frame Syn |      synchronize2 |  no |     9 |       * ||     1000 |     0.08 |   0.64 ||    81.08 |    78.94 |   132.15
#       Scrambler_PL |        descramble | yes |    10 |       * ||     1000 |     0.03 |   0.20 ||    25.10 |    22.84 |    47.96
#          L&R F Syn |       synchronize |  no |    11 |       * ||     1000 |     0.05 |   0.43 ||    54.28 |    52.89 |    80.29
#       Fine P/F Syn |       synchronize | yes |    12 |       * ||     1000 |     0.25 |   2.02 ||   253.84 |   152.28 |   487.97
#             Framer |        remove_plh | yes |    13 |       * ||     1000 |     0.05 |   0.38 ||    47.44 |    45.94 |   121.77
#          Estimator |          estimate | yes |    14 |       * ||     1000 |     0.03 |   0.26 ||    32.43 |    31.82 |    69.69
#              Modem |        demodulate | yes |    15 |       * ||     1000 |     2.12 |  16.86 ||  2123.07 |  2111.83 |  2540.95
#        Interleaver |      deinterleave | yes |    16 |       * ||     1000 |     0.03 |   0.23 ||    29.25 |    27.75 |    77.84
#       LDPC Decoder |       decode_siho | yes |    17 |       * ||     1000 |     0.24 |   1.90 ||   239.65 |   216.64 |  1694.95
#        BCH Decoder |       decode_hiho | yes |    18 |       * ||     1000 |     6.21 |  49.31 ||  6209.00 |  5979.63 | 11310.41
#       Scrambler_BB |        descramble | yes |    19 |       * ||     1000 |     0.56 |   4.44 ||   558.96 |   545.85 |   861.18
#               Sink |              send |  no |    20 |       * ||     1000 |     0.03 |   0.27 ||    34.61 |    25.01 |  8513.73
#             Source |          generate |  no |    21 |       * ||     1000 |     0.02 |   0.13 ||    16.90 |    14.95 |    37.19
#            Monitor |     check_errors2 | yes |    22 |       * ||     1000 |     0.01 |   0.07 ||     9.22 |     8.00 |    25.83
# -------------------|-------------------|-----|-------|---------||----------|----------|--------||----------|----------|----------
#              TOTAL |                 * |  no |     * |       * ||     1000 |    12.59 | 100.00 || 12592.54 | 12091.34 | 31070.54
*/
const std::vector<double> loads_big = {13165,13832,11369,33479,32930,134190,5868,6345,36593,8108,2510,5428,25384,4744,3243,212307,2925,23965,620900,55896,3461,1690,922};

/*
# On PUID n°12
# ---------------------------------------------------------------||------------------------------||--------------------------------
#                  Statistics for the given task                 ||       Basic statistics       ||        Measured latency
#               ('*' = any, '-' = same as previous)              ||          on the task         ||
# ---------------------------------------------------------------||------------------------------||--------------------------------
# -------------------|-------------------|-----|-------|---------||----------|----------|--------||----------|----------|----------
#        MODULE NAME |         TASK NAME | REP | ORDER |   TIMER ||    CALLS |     TIME |   PERC ||  AVERAGE |  MINIMUM |  MAXIMUM
#                    |                   |     |       |         ||          |      (s) |    (%) ||     (us) |     (us) |     (us)
# -------------------|-------------------|-----|-------|---------||----------|----------|--------||----------|----------|----------
#              Radio |           receive |  no |     0 |       * ||     1000 |     0.13 |   0.59 ||   133.22 |   115.06 |   180.05
#         Multiplier |         imultiply |  no |     1 |       * ||     1000 |     0.32 |   1.41 ||   318.06 |   314.52 |   596.31
#       Coarse_Synch |       synchronize |  no |     2 |       * ||     1000 |     0.43 |   1.90 ||   428.97 |   426.41 |   817.68
#        Matched Flt |           filter1 |  no |     3 |       * ||     1000 |     0.71 |   3.16 ||   711.90 |   707.33 |  1261.58
#        Matched Flt |           filter2 |  no |     4 |       * ||     1000 |     0.71 |   3.16 ||   712.59 |   704.64 |  1732.97
#        Gardner Syn |       synchronize |  no |     5 |       * ||     1000 |     2.39 |  10.59 ||  2387.11 |  2180.01 |  2569.75
#        Gardner Syn |           extract |  no |     6 |       * ||     1000 |     0.14 |   0.60 ||   135.13 |   120.27 |   218.56
#           Mult agc |         imultiply |  no |     7 |       * ||     1000 |     0.16 |   0.70 ||   157.39 |   156.24 |   181.99
#          Frame Syn |      synchronize1 |  no |     8 |       * ||     1000 |     0.85 |   3.76 ||   848.09 |   842.66 |   999.66
#          Frame Syn |      synchronize2 |  no |     9 |       * ||     1000 |     0.20 |   0.88 ||   197.93 |   189.83 |   231.98
#       Scrambler_PL |        descramble | yes |    10 |       * ||     1000 |     0.07 |   0.29 ||    65.94 |    65.03 |    72.33
#          L&R F Syn |       synchronize |  no |    11 |       * ||     1000 |     0.20 |   0.90 ||   203.18 |   200.47 |   223.60
#       Fine P/F Syn |       synchronize | yes |    12 |       * ||     1000 |     0.36 |   1.58 ||   356.20 |   353.85 |   372.71
#             Framer |        remove_plh | yes |    13 |       * ||     1000 |     0.09 |   0.39 ||    87.72 |    84.39 |   105.43
#          Estimator |          estimate | yes |    14 |       * ||     1000 |     0.07 |   0.29 ||    65.43 |    64.60 |    71.18
#              Modem |        demodulate | yes |    15 |       * ||     1000 |     5.74 |  25.49 ||  5742.44 |  5722.86 |  6732.90
#        Interleaver |      deinterleave | yes |    16 |       * ||     1000 |     0.05 |   0.21 ||    47.58 |    41.39 |   150.62
#       LDPC Decoder |       decode_siho | yes |    17 |       * ||     1000 |     1.02 |   4.55 ||  1024.37 |   569.42 |  1707.33
#        BCH Decoder |       decode_hiho | yes |    18 |       * ||     1000 |     8.17 |  36.24 ||  8166.17 |  7965.11 |  9462.18
#       Scrambler_BB |        descramble | yes |    19 |       * ||     1000 |     0.62 |   2.76 ||   621.83 |   609.78 |  1113.28
#               Sink |              send |  no |    20 |       * ||     1000 |     0.08 |   0.34 ||    75.59 |    74.38 |    82.58
#             Source |          generate |  no |    21 |       * ||     1000 |     0.02 |   0.10 ||    23.39 |    20.70 |    38.82
#            Monitor |     check_errors2 | yes |    22 |       * ||     1000 |     0.02 |   0.09 ||    20.47 |    19.57 |    28.03
# -------------------|-------------------|-----|-------|---------||----------|----------|--------||----------|----------|----------
#              TOTAL |                 * |  no |     * |       * ||     1000 |    22.53 | 100.00 || 22530.73 | 21548.49 | 28951.51
*/
const std::vector<double> loads_little = {13322,31806,42897,71190,71259,238711,13513,15739,84809,19793,6594,20318,35620,8772,6543,574244,4758,102437,816617,62183,7559,2339,2047};
const std::vector<bool> task_states = {SF,SF,SF,SF,SF,SF,SF,SF,SF,SF,SL,SF,SL,SL,SL,SL,SL,SL,SL,SL,SF,SF,SL};

void print_solution(
    std::vector<std::tuple<int, int, bool, double>> partition,
    int resources_big,
    int resources_little,
    std::string algorithm) {
    
    std::ofstream solution_output("u9_" + algorithm + "_" + 
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
    const std::vector<std::tuple<int, int>> resources = {{6, 8}, {3, 4}};
    const std::vector<std::string> algorithms = {"HeRAD", "2CATAC", "FERTAC", "OTAC_big", "OTAC_little"};

    const std::string results_file = "u9_results.csv";
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