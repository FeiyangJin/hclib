include ../common/make.inc
SHELL:=/bin/bash

BIN_DIR := bin
RD_TESTS=$(patsubst %.cpp,$(BIN_DIR)/%-rd.exe,$(wildcard test*.cpp))

.PHONY: all_rd
all_rd: $(RD_TESTS)

$(BIN_DIR)/%-rd.exe: %.cpp
	$(INST) -o $@ $^ -- -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)

.PHONY: clean
clean:
	rm -f $(BIN_DIR)/*.exe *.bc *.ll
