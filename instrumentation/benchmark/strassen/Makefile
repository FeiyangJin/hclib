include ../common/make.inc

all: strassen-rd.exe strassen-origin.exe

strassen-rd.exe: strassen.cpp 
	$(INST) $^ -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)
	mv strassen.exe strassen-rd.exe

strassen-origin.exe: strassen.cpp
	${CXX} -o $@ $^ $(TEST_ORIGIN_OPTIONS)

.PHONY: clean
clean:
	rm *.exe *.bc *.ll
