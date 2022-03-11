include ../common/make.inc
SHELL:=/bin/bash

all: example-rd.exe example-origin.exe

example-rd.exe: example.cpp
	$(INST) $^ -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)
	mv example.exe example-rd.exe

example-origin.exe: example.cpp
	${CXX} -o $@ $^ $(TEST_ORIGIN_OPTIONS)

.PHONY: clean
clean:
	rm *.exe *.bc *.ll
