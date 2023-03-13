#! /bin/bash

#capacity=30
capacity=500
#capacity=20000
#capacity=10300 #massimo senza segfault

#gcc binaryKnapsack.c -o binaryKnapsack && ./binaryKnapsack
#gcc binaryKnapsack2.c -o binaryKnapsack
#gcc binaryKnapsack2.1.c -o binaryKnapsack
#gcc binaryKnapsack_alloc.c -o binaryKnapsack
#gcc binaryKnapsack_timed.c -o binaryKnapsack
#gcc binaryKnapsack3.c -Ilibs -Llibs -lfminknap_npg -o binaryKnapsack -lm && ./binaryKnapsack
#gcc binaryKnapsack3.c -Ilibs -Llibs -lfminknap_npg -o binaryKnapsack -lm
#gcc binaryKnapsack3_test.c -Ilibs -Llibs -lfminknap_npg -o binaryKnapsack -lm
gcc binaryKnapsack3.1.c -Ilibs -Llibs -lfminknap_npg -o binaryKnapsack -lm

#gcc binaryKnapsack3_test.c -Ilibs -Llibs -lfminknap_npg -o binaryKnapsack -lm && binaryKnapsack "generator/files/str_corr" 200 "generator/files/derived/int/str_corr"
#gcc binaryKnapsack4.c -Ilibs -Llibs -lfminknap_npg -o binaryKnapsack -lm && ./binaryKnapsack ./generator/files/derived/int/ 500 > buffer.txt
#gcc binaryKnapsack4.1.c -Ilibs -Llibs -lfminknap_npg -o binaryKnapsack -lm && ./binaryKnapsack ./generator/files/derived/int/ 500 > buffer.txt
run(){
  for i in generator/files/derived/*; do
  #for i in generator/files/*; do
      echo $i
      ./binaryKnapsack $i $capacity
      echo ''
  done
}

while getopts "cCfFR" opt; do
  case $opt in
    c) 
        echo "Compiled"
        exit
      ;;
    f)
        ./binaryKnapsack "generator/files/str_corr.in" $capacity
        exit
      ;;
    C) 
        clear
        run
        exit
        ;;
    F) 
        clear
        ./binaryKnapsack "generator/files/str_corr.in" $capacity
        exit
        ;;
    R)  clear
        gcc -c  -ggdb -lm libs/fminknap_npg.c -o libs/fminknap_npg.o
        ar rcs libs/libfminknap_npg.a libs/fminknap_npg.o
        gcc binaryKnapsack3.c -Ilibs -Llibs -lfminknap_npg -o binaryKnapsack -lm
        ./binaryKnapsack "generator/files/str_corr" $capacity
        exit
        ;;
    \?)
        exit
        ;; 
  esac
done

run
rm binaryKnapsack

exit