LDFLAGS = -lpthread

CXXFLAGS = \
	-g -O2 -I./src -I./tests \

CFLAGS = $(CXXFLAGS)

PLOCK = src/partiallock.o src/list.o
PLOCK_TEST = tests/partiallock_test.o
PLOCK_BENCH = tests/partiallock_bench.o

PROGRAMS = \
	partiallock_test \
	bench \

all: $(PROGRAMS)

partiallock_test: $(PLOCK) $(PLOCK_TEST)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

bench: $(PLOCK) $(PLOCK_BENCH)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf $(PROGRAMS) ./src/*.o ./tests/*.o
