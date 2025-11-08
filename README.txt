HW3: Cache Simulator

BUILD:
make

Or:
g++ -std=c++11 -o cache_sim cache_simulator.cpp

RUN:
./cache_sim <num_entries> <associativity> <input_file>

EXAMPLE:
./cache_sim 4 2 test_hw_example.txt
cat cache_sim_output
