include ../../common/make.inc
SHELL:=/bin/bash

all: sparselu-rd.exe sparselu-origin.exe

sparselu-rd.exe: sparselu.cpp 
	$(INST) -o $@ $^ -- -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)

sparselu-origin.exe: sparselu.cpp
	${CXX} -o $@ $^ $(TEST_ORIGIN_OPTIONS)

.PHONY: clean
clean:
	rm -f *.exe *.bc *.ll
