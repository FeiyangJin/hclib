include ../common/make.inc

all: sort-rd.exe sort-origin.exe

sort-rd.exe: sort.cpp 
	$(INST) $^ -DRACE_DETECTION $(RD_OPTIONS)
	mv sort.exe sort-rd.exe

sort-origin.exe: sort.cpp
	${CXX} -o $@ $^ $(ORIGIN_OPTIONS)

.PHONY: clean
clean:
	rm *.exe *.bc *.ll
