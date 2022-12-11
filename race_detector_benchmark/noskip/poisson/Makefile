include ../../common/make.inc
SHELL:=/bin/bash

all: poisson-rd.exe poisson-origin.exe

poisson-rd.exe: poisson.cpp 
	$(INST) -o $@ $^ -- -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)

poisson-origin.exe: poisson.cpp
	${CXX} -o $@ $^ $(TEST_ORIGIN_OPTIONS)

.PHONY: clean
clean:
	rm -f *.exe *.bc *.ll
