
all: heat.log qsort.log randomized.log strassen.log streamcluster.log streamcluster2.log smithwaterman.log conway.log sieve.log
	./ingest_logs.pl

heat.log:
	./experiment.sh Heat "50 40000 5000" 2>&1 | tee $@

qsort.log:
	./experiment.sh QSort "1000000 100 20" 2>&1 | tee $@

randomized.log:
	./experiment.sh Randomized "2000 5000 3 .8 5127820" 2>&1 | tee $@

strassen.log:
	./experiment.sh Strassen "7 8192" 2>&1 | tee $@

streamcluster.log:
	./experiment.sh StreamCluster 2>&1 | tee $@

streamcluster2.log:
	./experiment.sh StreamCluster2 2>&1 | tee $@

smithwaterman.log:
	./experiment.sh SmithWaterman "../experiments/data/sw_large1.txt ../experiments/data/sw_large2.txt 25 25" 2>&1 | tee $@

conway.log:
	./experiment.sh Conway "200 10 2000" 2>&1 | tee $@

sieve.log:
	./experiment.sh Sieve 100000 2>&1 | tee $@
