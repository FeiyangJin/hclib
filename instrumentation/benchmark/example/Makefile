include ../common/make.inc

all: example-rd.exe example-origin.exe

example-rd.exe: example.cpp 
	$(INST) $^ -DRACE_DETECTION $(TEST_RD_OPTIONS)
	mv example.exe example-rd.exe

example-origin.exe: example.cpp
	${CXX} -o $@ $^ $(TEST_ORIGIN_OPTIONS)

.PHONY: clean
clean:
	rm *.exe *.bc *.ll
