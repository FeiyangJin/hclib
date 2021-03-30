include $(HCLIB_ROOT)/../modules/system/inc/hclib_system.pre.mak
include $(HCLIB_ROOT)/include/hclib.mak
include $(HCLIB_ROOT)/../modules/system/inc/hclib_system.post.mak

EXE=graph cd channelReader promise_yield

all: $(EXE)

graph: graph.cpp
	$(CXX) -O3 $(HCLIB_CXXFLAGS) $(HCLIB_LDFLAGS) -o $@ $^ $(HCLIB_LDLIBS)

cd: cholesky_deadlock.cpp
	$(CXX) -O3 $(HCLIB_CXXFLAGS) $(HCLIB_LDFLAGS) -o $@ $^ $(HCLIB_LDLIBS)

# channel: Channel.cpp
# 	$(CXX) -O3 $(HCLIB_CXXFLAGS) $(HCLIB_LDFLAGS) -o $@ $^ $(HCLIB_LDLIBS)

channelReader: ChannelReader.cpp
	$(CXX) -O3 $(HCLIB_CXXFLAGS) $(HCLIB_LDFLAGS) -o $@ $^ $(HCLIB_LDLIBS)

promise_yield: promise_yield.cpp
	$(CXX) -O3 $(HCLIB_CXXFLAGS) $(HCLIB_LDFLAGS) -o $@ $^ $(HCLIB_LDLIBS)

clean:
	rm -rf $(EXE)
