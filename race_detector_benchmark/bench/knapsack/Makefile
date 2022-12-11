include ../../common/make.inc
SHELL:=/bin/bash

all: knapsack-rd.exe knapsack-origin.exe

knapsack-rd.exe: knapsack.cpp 
	$(INST) -o $@ $^ -- -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)

knapsack-origin.exe: knapsack.cpp
	${CXX} -o $@ $^ $(TEST_ORIGIN_OPTIONS)

.PHONY: clean
clean:
	rm *.exe *.bc *.ll
