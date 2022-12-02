#! /bin/bash

#gcc binaryKnapsack.c -o binaryKnapsack
#gcc binaryKnapsack2.c -o binaryKnapsack
#gcc binaryKnapsack2.1.c -o binaryKnapsack
#gcc binaryKnapsack_alloc.c -o binaryKnapsack
gcc binaryKnapsack3.c -Ilibs -Llibs -lfminknap_npg -o binaryKnapsack -lm

run(){
  for i in generator/files/*.in; do
            echo $i
            ./binaryKnapsack $i 100
            echo ''
        done
}

while getopts "cCfFR" opt; do
  case $opt in
    c) 
        echo "Compiled"
      ;;
    f)
        ./binaryKnapsack "generator/files/str_corr.in" 100
      ;;
    C) 
        clear
        run
        ;;
    F) 
        clear
        ./binaryKnapsack "generator/files/str_corr.in" 100
        ;;
    R)  clear
        gcc -c  -ggdb -lm libs/fminknap_npg.c -o libs/fminknap_npg.o
        ar rcs libs/libfminknap_npg.a libs/fminknap_npg.o
        gcc binaryKnapsack3.c -Ilibs -Llibs -lfminknap_npg -o binaryKnapsack -lm
        ./binaryKnapsack "generator/files/str_corr.in" 100
        ;;
    \?)
        run
      ;;

  esac
done

rm binaryKnapsack