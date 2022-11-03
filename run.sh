#! /bin/bash

#gcc binaryKnapsack.c -o binaryKnapsack2a
#gcc binaryKnapsack2.c -o binaryKnapsack2a
#gcc binaryKnapsack2.1.c -o binaryKnapsack2a
gcc binaryKnapsack_timed_alloc.c -o binaryKnapsack2a

for i in generator/files/*.in; do
    echo $i
    ./binaryKnapsack2a $i 100
    echo ''
done

rm binaryKnapsack2a