include $(HCLIB_ROOT)/../modules/system/inc/hclib_system.pre.mak
include $(HCLIB_ROOT)/include/hclib.mak
include $(HCLIB_ROOT)/../modules/system/inc/hclib_system.post.mak

EXE=fib matmul_future strassen sort graph_promise finish_demo

all: $(EXE)

fib: fib.cpp
	$(CXX) -O3 $(HCLIB_CXXFLAGS) $(HCLIB_LDFLAGS) -o $@ $^ $(HCLIB_LDLIBS)

sort: sort.cpp
	$(CXX) -O3 $(HCLIB_CXXFLAGS) $(HCLIB_LDFLAGS) -o $@ $^ $(HCLIB_LDLIBS)

matmul_future: matmul_future.cpp
	$(CXX) -O3 $(HCLIB_CXXFLAGS) $(HCLIB_LDFLAGS) -o $@ $^ $(HCLIB_LDLIBS)

strassen: strassen_new.cpp
	$(CXX) -O3 $(HCLIB_CXXFLAGS) $(HCLIB_LDFLAGS) -o $@ $^ $(HCLIB_LDLIBS)

graph_promise: graph_promise.cpp
	$(CXX) -O3 -ldl $(HCLIB_CXXFLAGS) $(HCLIB_LDFLAGS) -o $@ $^ $(HCLIB_LDLIBS)

finish_demo: finish_demo.cpp
	$(CXX) -O3 $(HCLIB_CXXFLAGS) $(HCLIB_LDFLAGS) -o $@ $^ $(HCLIB_LDLIBS)


clean:
	rm -rf $(EXE)
