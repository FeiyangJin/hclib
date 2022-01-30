include ../common/make.inc

all: knapsack-rd.exe knapsack-origin.exe

knapsack-rd.exe: knapsack.cpp 
	$(INST) $^ -DRACE_DETECTION $(TEST_RD_OPTIONS)
	mv knapsack.exe knapsack-rd.exe

knapsack-origin.exe: knapsack.cpp
	${CXX} -o $@ $^ $(TEST_ORIGIN_OPTIONS)

.PHONY: clean
clean:
	rm *.exe *.bc *.ll
