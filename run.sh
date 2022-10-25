#! /bin/bash

gcc binaryKnapsack2.1.c -o binaryKnapsack2a

for i in generator/files/*.in; do
    echo $i
    ./binaryKnapsack2a $i 100
    echo ''
done

rm binaryKnapsack2a