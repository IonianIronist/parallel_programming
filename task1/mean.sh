#!/bin/sh


for i in `seq 1 10`
do
  ./a.out >> output.txt
done

python3 calc.py
