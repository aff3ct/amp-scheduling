CXX = g++
CXXFLAGS = -O3
DEPS = src/Scheduler/*_Scheduler.cpp src/Support/Test_Generator.cpp
INC = -I include/

all: performance simulation m1 ultra9

performance: examples/performance_experiment_small.cpp examples/performance_experiment_large.cpp
	$(CXX) examples/performance_experiment_small.cpp $(DEPS) $(INC) -o perf_exp_small $(CXXFLAGS)
	$(CXX) examples/performance_experiment_large.cpp $(DEPS) $(INC) -o perf_exp_large $(CXXFLAGS)

simulation: examples/simulation_experiment.cpp
	$(CXX) examples/simulation_experiment.cpp $(DEPS) $(INC) -o sim_exp $(CXXFLAGS)

m1: examples/dvbs2_m1.cpp
	$(CXX) examples/dvbs2_m1.cpp $(DEPS) $(INC) -o dvbs2_m1 $(CXXFLAGS)

ultra9: examples/dvbs2_ultra9.cpp
	$(CXX) examples/dvbs2_ultra9.cpp $(DEPS) $(INC) -o dvbs2_ultra9 $(CXXFLAGS)	

clean:
	rm perf_exp_small perf_exp_large sim_exp dvbs2_m1 dvbs2_ultra9
