include ../common/make.inc

all: life-rd.exe life-origin.exe

life-rd.exe: life.cpp 
	$(INST) $^ -DRACE_DETECTION $(TEST_RD_OPTIONS)
	mv life.exe life-rd.exe

life-origin.exe: life.cpp
	${CXX} -o $@ $^ $(TEST_ORIGIN_OPTIONS)

.PHONY: clean
clean:
	rm *.exe *.bc *.ll
