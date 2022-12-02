#! /bin/bash

#gcc binaryKnapsack.c -o binaryKnapsack
#gcc binaryKnapsack2.c -o binaryKnapsack
#gcc binaryKnapsack2.1.c -o binaryKnapsack
#gcc binaryKnapsack_alloc.c -o binaryKnapsack
gcc binaryKnapsack3.c -Ilibs -Llibs -lfminknap_npg -o binaryKnapsack -lm
for i in generator/files/*.in; do
            echo $i
            ./binaryKnapsack $i 100
            echo ''
  done
rm binaryKnapsack