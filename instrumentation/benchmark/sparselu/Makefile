include ../common/make.inc

all: sparselu-rd.exe sparselu-origin.exe

sparselu-rd.exe: sparselu.cpp 
	$(INST) $^ -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)
	mv sparselu.exe sparselu-rd.exe

sparselu-origin.exe: sparselu.cpp
	${CXX} -o $@ $^ $(TEST_ORIGIN_OPTIONS)

.PHONY: clean
clean:
	rm *.exe *.bc *.ll
