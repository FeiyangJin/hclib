include ../../common/make.inc
SHELL:=/bin/bash

all: example-rd.exe example-origin.exe

example-rd.exe: example.cpp
	$(INST) -o $@ $^ -- -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)

example-origin.exe: example.cpp
	${CXX} -o $@ $^ $(TEST_ORIGIN_OPTIONS)

.PHONY: clean
clean:
	rm *.exe *.bc *.ll
