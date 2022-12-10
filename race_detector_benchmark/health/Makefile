include ../common/make.inc

all: health-rd.exe health-origin.exe

health-rd.exe: health.cpp 
	$(INST) $^ -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)
	mv health.exe health-rd.exe

health-origin.exe: health.cpp
	${CXX} -o $@ $^ $(TEST_ORIGIN_OPTIONS)

.PHONY: clean
clean:
	rm *.exe *.bc *.ll
