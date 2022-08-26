SHELL:=/bin/bash

TEST_RD_OPTIONS=-O3 -std=c++11 -I$(HCLIB_INSTALL)/include -L$(HCLIB_INSTALL)/lib -lhclib -lrt -ldl -g
TEST_ORIGIN_OPTIONS=-O3 -std=c++11 -I$(HCLIB_ORIGIN_INSTALL)/include -L$(HCLIB_ORIGIN_INSTALL)/lib -lhclib -lrt -ldl -g

CC=clang
CXX=clang++

all: test1.exe
# test2 test3 test4 update_test test5 test6 

test1.exe: test1.cpp
	$(INST) $^ -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)

test2: test2.cpp
	$(CXX) -O0 $(HCLIB_CXXFLAGS) $(HCLIB_LDFLAGS) -o $@ $^ $(HCLIB_LDLIBS)

test3: test3.cpp
	$(CXX) -O0 $(HCLIB_CXXFLAGS) $(HCLIB_LDFLAGS) -o $@ $^ $(HCLIB_LDLIBS)

test4: test4.cpp
	$(CXX) -O0 $(HCLIB_CXXFLAGS) $(HCLIB_LDFLAGS) -o $@ $^ $(HCLIB_LDLIBS)

test5: test5.cpp
	$(CXX) -O0 $(HCLIB_CXXFLAGS) $(HCLIB_LDFLAGS) -o $@ $^ $(HCLIB_LDLIBS)

test6: test6.cpp
	$(CXX) -O0 $(HCLIB_CXXFLAGS) $(HCLIB_LDFLAGS) -o $@ $^ $(HCLIB_LDLIBS)

test7: test7.cpp
	$(CXX) -O0 $(HCLIB_CXXFLAGS) $(HCLIB_LDFLAGS) -o $@ $^ $(HCLIB_LDLIBS)
	
update_test: update_test.cpp
	$(CXX) -O0 $(HCLIB_CXXFLAGS) $(HCLIB_LDFLAGS) -o $@ $^ $(HCLIB_LDLIBS)

clean:
	rm *.exe *.bc *.ll
