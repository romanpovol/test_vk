CXX 	 = g++
CXXFLAGS = -std=c++17 -pthread -Wall -Werror -Wextra -fsanitize=undefined

scheduler = src/scheduler.cpp
test 	  = test/test.cpp test/doctest_main.cpp

.PHONY: test clean

test:
	rm -rf bin
	mkdir bin
	$(CXX) $(CXXFLAGS) $(scheduler) $(test) -o bin/test
	./bin/test

clean:
	rm -rf bin
