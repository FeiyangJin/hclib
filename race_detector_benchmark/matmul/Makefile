include ../common/make.inc

all: matmul-rd.exe matmul-origin.exe

matmul-rd.exe: matmul.cpp 
	$(INST) $^ -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)
	mv matmul.exe matmul-rd.exe

matmul-origin.exe: matmul.cpp
	${CXX} -o $@ $^ $(TEST_ORIGIN_OPTIONS)

.PHONY: clean
clean:
	rm -f *.exe *.bc *.ll
