#!/bin/bash

# experiment name in $1
# experiment args in $2

set -e

cd ../java
javac *.java

echo Test Name: $1
echo Arguments: $2

echo Stats:
java -Xmx1g -Dmode=OFF -Dmetric=STATS Test$1 $2

echo Baseline Execution Time:
java -Xmx1g -Dmode=OFF -Dmetric=TIME Test$1 $2

echo Baseline Memory Usage:
java -Xmx1g -Dmode=OFF -Dmetric=MEMORY Test$1 $2

echo Verified Execution Time:
java -Xmx1g -Dmode=DETECT_DEADLOCKS -Dmetric=TIME Test$1 $2

echo Verified Memory Usage:
java -Xmx1g -Dmode=DETECT_DEADLOCKS -Dmetric=MEMORY Test$1 $2
echo "========"
