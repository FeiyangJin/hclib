include ../common/make.inc

all: nqueens-rd.exe nqueens-origin.exe

nqueens-rd.exe: nqueens.cpp 
	$(INST) $^ -DRACE_DETECTION $(TEST_RD_OPTIONS)
	mv nqueens.exe nqueens-rd.exe

nqueens-origin.exe: nqueens.cpp
	${CXX} -o $@ $^ $(TEST_ORIGIN_OPTIONS)

.PHONY: clean
clean:
	rm *.exe *.bc *.ll
