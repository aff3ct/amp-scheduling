# AMP Scheduling

Repository containing the implementation of scheduling strategies for partially-replicable task chains over heterogeneous/asymmetric processors.
It includes FERTAC, 2CATAC, HeRAD, and an implementation of OTAC.

To simulate schedules, use `./run_simulation_tests.sh`.

To profile the schedulers locally, use `./run_performance_tests.sh` (it should take under 4 hours).

To compute the schedules for DVB-S2 in two heterogeneous architectures, use `./simulate_dvbs2_schedules.sh`.

To analyze previously generated results and generate figures, decompress `previous_results.zip` and use `jupyter notebook` to run the analysis in `previous_results/CSV_Files/Result Analysis.ipynb`. You can also copy it to this root folder to analyze your own results.
