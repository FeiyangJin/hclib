include ../common/make.inc

all: poisson-rd.exe poisson-origin.exe sparselu-rd.exe sparselu-origin.exe sort-rd.exe sort-origin.exe matmul-rd.exe matmul-origin.exe

poisson-rd.exe: poisson.cpp 
	$(INST) $^ -DRACE_DETECTION $(TEST_RD_OPTIONS)
	mv poisson.exe poisson-rd.exe

poisson-origin.exe: poisson.cpp
	${CXX} -o $@ $^ $(TEST_ORIGIN_OPTIONS)

sparselu-rd.exe: sparselu.cpp 
	$(INST) $^ -DRACE_DETECTION $(TEST_RD_OPTIONS)
	mv sparselu.exe sparselu-rd.exe

sparselu-origin.exe: sparselu.cpp
	${CXX} -o $@ $^ $(TEST_ORIGIN_OPTIONS)

sort-rd.exe: sort.cpp 
	$(INST) $^ -DRACE_DETECTION $(TEST_RD_OPTIONS)
	mv sort.exe sort-rd.exe

sort-origin.exe: sort.cpp
	${CXX} -o $@ $^ $(TEST_ORIGIN_OPTIONS)

matmul-rd.exe: matmul.cpp 
	$(INST) $^ -DRACE_DETECTION $(TEST_RD_OPTIONS)
	mv matmul.exe matmul-rd.exe

matmul-origin.exe: matmul.cpp
	${CXX} -o $@ $^ $(TEST_ORIGIN_OPTIONS)

.PHONY: clean
clean:
	rm *.exe *.bc *.ll
