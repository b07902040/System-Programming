#!/bin/sh

./main < OS_PJ1_Test/TIME_MEASUREMENT.txt > output/TIME_MEASUREMENT_stdout.txt
dmesg | grep Project1 > output/TIME_MEASUREMENT_dmesg.txt
dmesg -c
for i in FIFO RR SJF PSJF 
do
	for j in 1 2 3 4 5
	do
	  ./main < OS_PJ1_Test/${i}_${j}.txt > output/${i}_${j}_stdout.txt
	  dmesg | grep Project1 > output/${i}_${j}_dmesg.txt
	  dmesg -c
	  #echo ${i}_${j}.txt ${i}_${j}_dmesg.txt
	done
done
