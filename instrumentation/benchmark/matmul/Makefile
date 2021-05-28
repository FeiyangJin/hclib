include ../common/make.inc

all: matmul-rd.exe matmul-origin.exe

matmul-rd.exe: matmul.cpp 
	$(INST) $^ -DRACE_DETECTION $(RD_OPTIONS)
	mv matmul.exe matmul-rd.exe

matmul-origin.exe: matmul.cpp
	${CXX} -o $@ $^ $(ORIGIN_OPTIONS)

.PHONY: clean
clean:
	rm *.exe *.bc *.ll
