CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra

all: cache_sim

cache_sim: cache_simulator.cpp
	$(CXX) $(CXXFLAGS) -o cache_sim cache_simulator.cpp

clean:
	rm -f cache_sim cache_sim_output

test: cache_sim
	./cache_sim 8 2 test_input.txt
	@echo "\n--- Output (cache_sim_output) ---"
	@cat cache_sim_output

.PHONY: all clean test
