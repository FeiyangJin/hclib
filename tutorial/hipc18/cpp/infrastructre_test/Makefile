include $(HCLIB_ROOT)/../modules/system/inc/hclib_system.pre.mak
include $(HCLIB_ROOT)/include/hclib.mak
include $(HCLIB_ROOT)/../modules/system/inc/hclib_system.post.mak

CXX=clang++

EXE=test7 life
#EXE=test1 test2 test3 test4 update_test test5 test6 

all: $(EXE)

# test1: test1.cpp
# 	$(CXX) -O0 $(HCLIB_CXXFLAGS) $(HCLIB_LDFLAGS) -o $@ $^ $(HCLIB_LDLIBS)

# test2: test2.cpp
# 	$(CXX) -O0 $(HCLIB_CXXFLAGS) $(HCLIB_LDFLAGS) -o $@ $^ $(HCLIB_LDLIBS)

# test3: test3.cpp
# 	$(CXX) -O0 $(HCLIB_CXXFLAGS) $(HCLIB_LDFLAGS) -o $@ $^ $(HCLIB_LDLIBS)

# test4: test4.cpp
# 	$(CXX) -O0 $(HCLIB_CXXFLAGS) $(HCLIB_LDFLAGS) -o $@ $^ $(HCLIB_LDLIBS)

# update_test: update_test.cpp
# 	$(CXX) -O0 $(HCLIB_CXXFLAGS) $(HCLIB_LDFLAGS) -o $@ $^ $(HCLIB_LDLIBS)

# test5: test5.cpp
# 	$(CXX) -O0 $(HCLIB_CXXFLAGS) $(HCLIB_LDFLAGS) -o $@ $^ $(HCLIB_LDLIBS)

# test6: test6.cpp
# 	$(CXX) -O0 $(HCLIB_CXXFLAGS) $(HCLIB_LDFLAGS) -o $@ $^ $(HCLIB_LDLIBS)

test7: test7.cpp
	$(CXX) -O0 $(HCLIB_CXXFLAGS) $(HCLIB_LDFLAGS) -o $@ $^ $(HCLIB_LDLIBS)

life: life.cpp
	$(CXX) -O0 $(HCLIB_CXXFLAGS) $(HCLIB_LDFLAGS) -o $@ $^ $(HCLIB_LDLIBS)
	
clean:
	rm -rf $(EXE)
