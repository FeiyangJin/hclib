include ../common/make.inc

all: health-rd.exe health-origin.exe

health-rd.exe: health.cpp 
	$(INST) $^ -DRACE_DETECTION $(RD_OPTIONS)
	mv health.exe health-rd.exe

health-origin.exe: health.cpp
	${CXX} -o $@ $^ $(ORIGIN_OPTIONS)

.PHONY: clean
clean:
	rm *.exe *.bc *.ll
