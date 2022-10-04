include ../common/make.inc
SHELL:=/bin/bash

tests = test1-rd.exe test2-rd.exe test3-rd.exe test4-rd.exe test5-rd.exe test6-rd.exe test7-rd.exe test8-rd.exe \
		test9-rd.exe test10-rd.exe test11-rd.exe test12-rd.exe test13-rd.exe test14-rd.exe test15-rd.exe test16-rd.exe \
		test17-rd.exe test18-rd.exe test19-rd.exe test20-rd.exe test21-rd.exe test22-rd.exe test23-rd.exe

all: $(tests)

$(tests): %.exe: %.cpp
	$(INST) $^ -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)

# all: test1-rd.exe test2-rd.exe test3-rd.exe test4-rd.exe test5-rd.exe test6-rd.exe test7-rd.exe test8-rd.exe test9-rd.exe test10-rd.exe test11-rd.exe test12-rd.exe test13-rd.exe test14-rd.exe test15-rd.exe test16-rd.exe

# test1-rd.exe: test1.cpp
# 	$(INST) $^ -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)
# 	mv test1.exe test1-rd.exe

# test2-rd.exe: test2.cpp
# 	$(INST) $^ -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)
# 	mv test2.exe test2-rd.exe

# test3-rd.exe: test3.cpp
# 	$(INST) $^ -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)
# 	mv test3.exe test3-rd.exe

# test4-rd.exe: test4.cpp
# 	$(INST) $^ -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)
# 	mv test4.exe test4-rd.exe

# test5-rd.exe: test5.cpp
# 	$(INST) $^ -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)
# 	mv test5.exe test5-rd.exe

# test6-rd.exe: test6.cpp
# 	$(INST) $^ -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)
# 	mv test6.exe test6-rd.exe

# test7-rd.exe: test7.cpp
# 	$(INST) $^ -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)
# 	mv test7.exe test7-rd.exe

# test8-rd.exe: test8.cpp
# 	$(INST) $^ -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)
# 	mv test8.exe test8-rd.exe

# test9-rd.exe: test9.cpp
# 	$(INST) $^ -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)
# 	mv test9.exe test9-rd.exe

# test10-rd.exe: test10.cpp
# 	$(INST) $^ -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)
# 	mv test10.exe test10-rd.exe

# test11-rd.exe: test11.cpp
# 	$(INST) $^ -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)
# 	mv test11.exe test11-rd.exe

# test12-rd.exe: test12.cpp
# 	$(INST) $^ -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)
# 	mv test12.exe test12-rd.exe

# test13-rd.exe: test13.cpp
# 	$(INST) $^ -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)
# 	mv test13.exe test13-rd.exe

# test14-rd.exe: test14.cpp
# 	$(INST) $^ -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)
# 	mv test14.exe test14-rd.exe

# test15-rd.exe: test15.cpp
# 	$(INST) $^ -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)
# 	mv test15.exe test15-rd.exe

# test16-rd.exe: test16.cpp
# 	$(INST) $^ -DRACE_DETECTION -DDRDP_ENABLED $(TEST_RD_OPTIONS)
# 	mv test16.exe test16-rd.exe

.PHONY: clean
clean:
	rm *.exe *.bc *.ll
