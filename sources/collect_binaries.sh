BENCHMARKS="STAMP-bayes STAMP-genome STAMP-intruder STAMP-kmeans STAMP-labyrinth STAMP-vacation STAMP-yada STAMP-ssca2"
while test $# -gt 0
do
	case "$1" in
		-clean)
			make clean
			echo Compiling....
			make -f Makefile
			;;
	esac
	shift
done
rm bin/*

echo "Moving binaries.... "
for BENCH in $BENCHMARKS
do
	cp ${BENCH}/bin/${BENCH}.original.V0 bin/${BENCH}.original.V0
	cp ${BENCH}/bin/${BENCH}.original.V1 bin/${BENCH}.original.V1
	cp ${BENCH}/bin/${BENCH}.original.V2 bin/${BENCH}.original.V2
done
echo Done !
