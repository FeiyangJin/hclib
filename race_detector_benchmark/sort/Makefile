include ../common/make.inc

all: sort-rd.exe sort-origin.exe

sort-rd.exe: sort.cpp 
	$(INST) $^ -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)
	mv sort.exe sort-rd.exe

sort-origin.exe: sort.cpp
	${CXX} -o $@ $^ $(TEST_ORIGIN_OPTIONS)

.PHONY: clean
clean:
	rm *.exe *.bc *.ll
