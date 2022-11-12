include ../common/make.inc
SHELL:=/bin/bash

tests = test1.exe test2.exe

all: $(tests) test2-origin.exe

$(tests): %.exe: %.cpp
	$(INST) $^ -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)

test2-origin.exe: test2.cpp
	${CXX} -o $@ $^ $(TEST_ORIGIN_OPTIONS)

.PHONY: clean
clean:
	rm -f *.exe *.bc *.ll