include ../../common/make.inc
SHELL:=/bin/bash

all: sort-rd.exe sort-origin.exe

sort-rd.exe: sort.cpp 
	$(INST) -o $@ $^ -- -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)

sort-origin.exe: sort.cpp
	${CXX} -o $@ $^ $(TEST_ORIGIN_OPTIONS)

.PHONY: clean
clean:
	rm -f *.exe *.bc *.ll
