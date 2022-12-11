include ../../common/make.inc
SHELL:=/bin/bash

all: matmul-rd.exe matmul-origin.exe

matmul-rd.exe: matmul.cpp 
	$(INST) -o $@ $^ -- -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)

matmul-origin.exe: matmul.cpp
	${CXX} -o $@ $^ $(TEST_ORIGIN_OPTIONS)

.PHONY: clean
clean:
	rm -f *.exe *.bc *.ll
