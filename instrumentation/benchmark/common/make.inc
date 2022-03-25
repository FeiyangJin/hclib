HCLIB_INSTALL=../../../hclib-install
HCLIB_ORIGIN_INSTALL=../../../hclib-install-orig
INST=$(HCLIB_INSTALL)/bin/inst.sh
RD_OPTIONS=-O3 -std=c++11 -I$(HCLIB_INSTALL)/include -L$(HCLIB_INSTALL)/lib -lhclib -lrt -ldl
ORIGIN_OPTIONS=-O3 -std=c++11 -I$(HCLIB_ORIGIN_INSTALL)/include -L$(HCLIB_ORIGIN_INSTALL)/lib -lhclib -lrt -ldl

LIFE_RD_OPTIONS=-O1 -std=c++11 -I$(HCLIB_INSTALL)/include -L$(HCLIB_INSTALL)/lib -lhclib -lrt -ldl
LIFE_ORIGIN_OPTIONS=-O1 -std=c++11 -I$(HCLIB_ORIGIN_INSTALL)/include -L$(HCLIB_ORIGIN_INSTALL)/lib -lhclib -lrt -ldl

TEST_RD_OPTIONS=-O3 -std=c++11 -I$(HCLIB_INSTALL)/include -L$(HCLIB_INSTALL)/lib -lhclib -lrt -ldl -g
TEST_ORIGIN_OPTIONS=-O3 -std=c++11 -I$(HCLIB_ORIGIN_INSTALL)/include -L$(HCLIB_ORIGIN_INSTALL)/lib -lhclib -lrt -ldl -g

CC=clang
CXX=clang++
