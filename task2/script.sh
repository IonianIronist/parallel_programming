#!/bin/bash

echo "threds, length, time" >> out.csv

U="_"
for ((threads = 2 ; threads <= 16; threads *= 2))
do
    for ((length = 10000; length <= 10000000; length *= 10))
	    do
            gcc -O2 -Wall -pthread quicksort-simple.c -o ./bin/parallel_"$length$U$threads".o  -DTHREADS=$threads -DN=$length
            for ((repeat = 0; repeat < 10; repeat++))
                do
                    echo $threads , $length , >> out.csv
                    ./bin/parallel_"$length$U$threads".o >> out.csv
                done
        done
done
