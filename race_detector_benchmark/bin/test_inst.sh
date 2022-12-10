#!/bin/bash

ROOT=$(readlink -f $(dirname $0)/..)
BENCHMARKS="fib health matmul nqueens poisson sort sparselu strassen"
ORACLE_FOLDER="bc"

for bench in $BENCHMARKS; do    
  echo "====================== Test $bench ==============================="
  BC_FILE="${bench}-inst.bc"
  pushd $ROOT/$bench > /dev/null
  make -B > /dev/null 2>&1
  DIFF_OUTPUT=$(diff $ROOT/$ORACLE_FOLDER/$BC_FILE $BC_FILE)
  if [ -z "$DIFF_OUTPUT" ]; then
      echo "Inconsistent BC file"
  else
      echo "Test succeed"
  fi
  popd > /dev/null
done
