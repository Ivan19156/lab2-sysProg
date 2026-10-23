CXX = g++
SRC = src/matrix_transpose.cpp

all: no_opt opt

no_opt:
	$(CXX) -O0 -g -o transpose_no_opt $(SRC)

opt:
	$(CXX) -O3 -march=native -g -o transpose_opt $(SRC)

flamegraph_no_opt: no_opt
	sudo perf record -g ./transpose_no_opt
	sudo perf script | ~/FlameGraph/stackcollapse-perf.pl | ~/FlameGraph/flamegraph.pl > flamegraphs/flamegraph_no_opt.svg

flamegraph_opt: opt
	sudo perf record -g ./transpose_opt
	sudo perf script | ~/FlameGraph/stackcollapse-perf.pl | ~/FlameGraph/flamegraph.pl > flamegraphs/flamegraph_opt.svg

clean:
	rm -f transpose_no_opt transpose_opt perf.data
