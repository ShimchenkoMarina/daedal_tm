#! /bin/bash
	
	
# Run all the STAMP benchs
trap "exit" INT
file=raw_results
	
cd bin
for i in `seq 1 10`;
do 
	>&2 echo
	>&2 echo bayes default
	echo bayes default
	>&2 echo bayes_default_V0
	sudo ./STAMP-bayes.original.V0 -t4
	>&2 echo bayes_default_V1
	sudo ./STAMP-bayes.original.V1 -t4
	>&2 echo bayes_default_V2
	sudo ./STAMP-bayes.original.V2 -t4
	
	>&2 echo
	>&2 echo bayes readme
	echo bayes readme
	>&2 echo bayes_complex_V0
	sudo ./STAMP-bayes.original.V0 -v32 -r4096 -n10 -p40 -i2 -e8 -s1 -t4
	>&2 echo bayes_complex_V1
	sudo ./STAMP-bayes.original.V1 -v32 -r4096 -n10 -p40 -i2 -e8 -s1 -t4
	>&2 echo bayes_complex_V2
	sudo ./STAMP-bayes.original.V2 -v32 -r4096 -n10 -p40 -i2 -e8 -s1 -t4
	
	>&2 echo genome default
	echo genome default
	>&2 echo genome_default_V0
	sudo ./STAMP-genome.original.V0 -t4
	>&2 echo genome_default_V1
	sudo ./STAMP-genome.original.V1 -t4
	>&2 echo genome_default_V2
	sudo ./STAMP-genome.original.V2 -t4
	
	
	>&2 echo
	>&2 echo genome readme
	echo genome readme
	>&2 echo genome_complex_V0
	sudo ./STAMP-genome.original.V0 -g16384 -s64 -n16777216 -t4
	>&2 echo genome_complex_V1
	sudo ./STAMP-genome.original.V1 -g16384 -s64 -n16777216 -t4
	>&2 echo genome_complex_V2
	sudo ./STAMP-genome.original.V2 -g16384 -s64 -n16777216 -t4
	
	
	>&2 echo
	>&2 echo intruder default
	echo intruder default
	>&2 echo intruder_default_V0
	sudo ./STAMP-intruder.original.V0 -t4
	>&2 echo intruder_default_V1
	sudo ./STAMP-intruder.original.V1 -t4
	>&2 echo intruder_default_V2
	sudo ./STAMP-intruder.original.V2 -t4
	
	
	>&2 echo
	>&2 echo intruder readme
	echo intruder readme
	>&2 echo intruder_complex_V0
	sudo ./STAMP-intruder.original.V0 -a10 -l128 -n262144 -s1 -t4
	>&2 echo intruder_complex_V1
	sudo ./STAMP-intruder.original.V1 -a10 -l128 -n262144 -s1 -t4
	>&2 echo intruder_complex_V2
	sudo ./STAMP-intruder.original.V2 -a10 -l128 -n262144 -s1 -t4
	
	
	
	>&2 echo
	>&2 echo kmeans low contention
	 echo kmeans low contention
	>&2 echo kmeans_low_contention.V0
	sudo ./STAMP-kmeans.original.V0 -m40 -n40 -t0.00001 -i ./../STAMP-kmeans/src/inputs/random-n65536-d32-c16.txt -p4
	>&2 echo kmeans_low_contention.V1
	sudo ./STAMP-kmeans.original.V1 -m40 -n40 -t0.00001 -i ./../STAMP-kmeans/src/inputs/random-n65536-d32-c16.txt -p4
	>&2 echo kmeans_low_contention.V2
	sudo ./STAMP-kmeans.original.V2 -m40 -n40 -t0.00001 -i ./../STAMP-kmeans/src/inputs/random-n65536-d32-c16.txt -p4
	
	
	
	>&2 echo
	>&2 echo kmeans high contention
	echo kmeans high contention
	>&2 echo kmeans_high_contension_V0
	sudo ./STAMP-kmeans.original.V0 -m15 -n15 -t0.00001 -i ./../STAMP-kmeans/src/inputs/random-n65536-d32-c16.txt   -p4
	>&2 echo kmeans_high_contension_V1
	sudo ./STAMP-kmeans.original.V1 -m15 -n15 -t0.00001 -i ./../STAMP-kmeans/src/inputs/random-n65536-d32-c16.txt   -p4
	>&2 echo kmeans_high_contension_V2
	sudo ./STAMP-kmeans.original.V2 -m15 -n15 -t0.00001 -i ./../STAMP-kmeans/src/inputs/random-n65536-d32-c16.txt   -p4
	
	
	
	>&2 echo
	>&2 echo labyrinth readme
	echo labyrinth readme
	>&2 echo labyrinth_complex_V0
	sudo ./STAMP-labyrinth.original.V0 -i ./../STAMP-labyrinth/src/inputs/random-x512-y512-z7-n512.txt -t4
	>&2 echo labyrinth_complex_V1
	sudo ./STAMP-labyrinth.original.V1 -i ./../STAMP-labyrinth/src/inputs/random-x512-y512-z7-n512.txt -t4
	>&2 echo labyrinth_complex_V2
	sudo ./STAMP-labyrinth.original.V2 -i ./../STAMP-labyrinth/src/inputs/random-x512-y512-z7-n512.txt -t4
	
	
	
	
	>&2 echo
	>&2 echo ssca2 default
	echo ssca2 default
	>&2 echo ssca2_default_V0
	sudo ./STAMP-ssca2.original.V0 -t4
	>&2 echo ssca2_default_V1
	sudo ./STAMP-ssca2.original.V1 -t4
	>&2 echo ssca2_default_V2
	sudo ./STAMP-ssca2.original.V2 -t4
	
	
	>&2 echo
	>&2 echo ssca2 readme
	echo ssca2 readme
	>&2 echo ssca2_complex_V0
	sudo ./STAMP-ssca2.original.V0 -s20 -i1.0 -u1.0 -l3 -p3 -t4
	>&2 echo ssca2_complex_V1
	sudo ./STAMP-ssca2.original.V1 -s20 -i1.0 -u1.0 -l3 -p3 -t4
	>&2 echo ssca2_complex_V2
	sudo ./STAMP-ssca2.original.V2 -s20 -i1.0 -u1.0 -l3 -p3 -t4
	
	>&2 echo
	>&2 echo yada readme
	echo yada readme
	>&2 echo yada_complex_V0
	sudo ./STAMP-yada.original.V0 -a15 -i ./../STAMP-yada/src/inputs/ttimeu1000000.2 -t4
	>&2 echo yada_complex_V1
	sudo ./STAMP-yada.original.V1 -a15 -i ./../STAMP-yada/src/inputs/ttimeu1000000.2 -t4
	>&2 echo yada_complex_V2
	sudo ./STAMP-yada.original.V2 -a15 -i ./../STAMP-yada/src/inputs/ttimeu1000000.2 -t4
done
	
'''	>&2 echo
	>&2 echo vacation low contention
	echo vacation low contention
	>&2 echo vocation_low_contention_V0
	sudo ./STAMP-vacation.original.V0 -n2 -q90 -u98 -r1048576 -t4096
	>&2 echo vocation_low_contention_V1
	sudo ./STAMP-vacation.original.V1 -n2 -q90 -u98 -r1048576 -t4096
	>&2 echo vocation_low_contention_V2
	sudo ./STAMP-vacation.original.V2 -n2 -q90 -u98 -r1048576 -t4096
	
	
	
	>&2 echo
	>&2 echo vacation high contention
	echo vacation high contention
	>&2 echo vacation_high_contension_V0
	sudo ./STAMP-vacation.original.V0 -n4 -q60 -u90 -r1048576 -t4194304
	>&2 echo vacation_high_contension_V1
	sudo ./STAMP-vacation.original.V1 -n4 -q60 -u90 -r1048576 -t4194304
	>&2 echo vacation_high_contension_V2
	sudo ./STAMP-vacation.original.V2 -n4 -q60 -u90 -r1048576 -t4194304'''
