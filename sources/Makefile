# Copyright (C) Eta Scale AB. Licensed under the Eta Scale Open Source License. See the LICENSE file for details.

#BENCHMARKS = myBenchmark
#BENCHMARKS = STAMP-bayes STAMP-genome STAMP-intruder STAMP-kmeans STAMP-labyrinth STAMP-ssca2 STAMP-vacation STAMP-yada STAMP-ubench
#BENCHMARKS = STAMP-intruder 
#BENCHMARKS = STAMP-kmeans 
#BENCHMARKS = STAMP-labyrinth 
BENCHMARKS = STAMP-ssca2 
#BENCHMARKS = STAMP-vacation
#BENCHMARKS = STAMP-yada
#BENCHMARKS = STAMP-ubench
#BENCHMARKS = STAMP-genome
#BENCHMARKS = STAMP-bayes
LIBS-DIR = STAMP-include/lib

.SECONDEXPANSION:

DAE: BINDIR
	$(foreach bench, $(BENCHMARKS), \
	$(MAKE) -C $(bench)/src 2>&1 | tee $(bench)/bin/log.txt;)

BINDIR: $$(foreach bench, $$(BENCHMARKS), $$(bench)/bin)

%/bin:
	mkdir -p $@

clean:
	$(foreach bench, $(BENCHMARKS), \
	$(MAKE) -C $(bench)/src clean;)

veryclean: clean
	make clean -C $(LIBS-DIR)
