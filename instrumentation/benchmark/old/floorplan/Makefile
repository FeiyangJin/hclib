include ../common/make.inc

all: floorplan-rd.exe floorplan-origin.exe

floorplan-rd.exe: floorplan.cpp 
	$(INST) $^ -DRACE_DETECTION $(TEST_RD_OPTIONS)
	mv floorplan.exe floorplan-rd.exe

floorplan-origin.exe: floorplan.cpp
	${CXX} -o $@ $^ $(TEST_ORIGIN_OPTIONS)

.PHONY: clean
clean:
	rm *.exe *.bc *.ll
