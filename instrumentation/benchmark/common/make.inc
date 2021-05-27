INST=../../inst.sh
HCLIB_INSTALL=../../../hclib-install
HCLIB_ORIGIN_INSTALL=/home/fjin/hclib/hclib-install
RD_OPTIONS=-O3 -std=c++11 -I$(HCLIB_INSTALL)/include -L$(HCLIB_INSTALL)/lib -lhclib -lrt -ldl
ORIGIN_OPTIONS=-O3 -std=c++11 -I$(HCLIB_ORIGIN_INSTALL)/include -L$(HCLIB_ORIGIN_INSTALL)/lib -lhclib -lrt -ldl
CC=clang
CXX=clang++
