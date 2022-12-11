include ../../common/make.inc
SHELL:=/bin/bash

all: health-rd.exe health-origin.exe

health-rd.exe: health.cpp 
	$(INST) -o $@ $^ -- -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)

health-origin.exe: health.cpp
	${CXX} -o $@ $^ $(TEST_ORIGIN_OPTIONS)

.PHONY: clean
clean:
	rm *.exe *.bc *.ll
