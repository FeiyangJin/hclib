include ../common/make.inc

all: smith_waterman-rd.exe smith_waterman-origin.exe

smith_waterman-rd.exe: smith_waterman.cpp 
	$(INST) $^ -DRACE_DETECTION $(TEST_RD_OPTIONS)
	mv smith_waterman.exe smith_waterman-rd.exe

smith_waterman-origin.exe: smith_waterman.cpp
	${CXX} -o $@ $^ $(TEST_ORIGIN_OPTIONS)

.PHONY: clean
clean:
	rm *.exe *.bc *.ll
