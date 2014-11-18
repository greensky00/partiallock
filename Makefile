LDFLAGS = -lpthread

CXXFLAGS = \
	-g -O2 -I./src -I./tests \

CFLAGS = $(CXXFLAGS)

PLOCK = src/partiallock.o src/list.o
PLOCK_TEST = tests/partiallock_test.o
PLOCK_TEST_RW = tests/partiallock_test_rwlock.o
PLOCK_BENCH = tests/partiallock_bench.o

PROGRAMS = \
	partiallock_test \
    partiallock_test_rwlock \
	bench \

all: $(PROGRAMS)

partiallock_test: $(PLOCK) $(PLOCK_TEST)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

partiallock_test_rwlock: $(PLOCK) $(PLOCK_TEST_RW)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

bench: $(PLOCK) $(PLOCK_BENCH)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf $(PROGRAMS) ./src/*.o ./tests/*.o
