include ../common/make.inc
SHELL:=/bin/bash

RD_TESTS=$(patsubst %.cpp,%-rd.exe,$(wildcard test*.cpp))

.PHONY: all_rd
all_rd: $(RD_TESTS)

%-rd.exe: %.cpp
	$(INST) -o $@ $^ -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)

.PHONY: clean
clean:
	rm *.exe *.bc *.ll
