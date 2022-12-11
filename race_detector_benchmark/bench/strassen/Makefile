include ../../common/make.inc
SHELL:=/bin/bash

all: strassen-rd.exe strassen-origin.exe

strassen-rd.exe: strassen.cpp 
	$(INST) -o $@ $^ -- -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)

strassen-origin.exe: strassen.cpp
	${CXX} -o $@ $^ $(TEST_ORIGIN_OPTIONS)

.PHONY: clean
clean:
	rm -f *.exe *.bc *.ll
