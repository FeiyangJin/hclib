include ../common/make.inc

all: fib-rd.exe fib-origin.exe

fib-rd.exe: fib.cpp 
	$(INST) $^ -DRACE_DETECTION $(RD_OPTIONS)
	mv fib.exe fib-rd.exe

fib-origin.exe: fib.cpp
	${CXX} -o $@ $^ $(ORIGIN_OPTIONS)

.PHONY: clean
clean:
	rm *.exe *.bc *.ll
