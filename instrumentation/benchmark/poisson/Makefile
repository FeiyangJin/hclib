include ../common/make.inc

all: poisson-rd.exe poisson-origin.exe

poisson-rd.exe: poisson.cpp 
	$(INST) $^ -DRACE_DETECTION $(RD_OPTIONS)
	mv poisson.exe poisson-rd.exe

poisson-origin.exe: poisson.cpp
	${CXX} -o $@ $^ $(ORIGIN_OPTIONS)

.PHONY: clean
clean:
	rm *.exe *.bc *.ll
