#! /bin/bash
gcc binaryKnapsack_ks2_mem_opt.c -Ilibs -Llibs -lfminknap_npg -o binaryKnapsack -lm
# testmodes:
#         0: full (esecuzione di tutti gli algoritmi risolutivi)
#         1: optimized (esecuzione di tutti gli algoritmi con ottimizzazione delle colonne e fminknap)
#         2: column optimization (confronto della riduzione del numero delle colonne degli algoritmi di ottimizzazione delle colonne)
#         3: approximated (confronto con gli algoritmi approssimati con divisione dei pesi o esclusione delle colonne)
#         4: approximated column optimization (confronto della riduzione delle colonne con gli algoritmi approssimati)

# n=200
# range=20000
# instanceNr=100
# capacity=20000 

# VALORI USATI PER ESEMPIO MINCAP
# n=5  
# range=3
# instanceNr=1
# capacity=10
#for range in {10..1000..10}
#for cut in {10..100..10}
for a in {1..1..1}
do
    echo ${cut}
    # n=${i}
    n=100
    range=5000
    instanceNr=100
    capacity=5000

    # n=10  
    # range=50
    # instanceNr=1
    # capacity=20

    mode=4
    division=10
    cut=10

    filenames="./generator/files/derived/int/"

    #confronto colonne 
    #newfilename="./csv/csv_c${capacity}_n${n}_r${range}_fn${instanceNr}_mode${mode}_div${division}_cut${cut}.csv"
    
    #confronto tempi colonne
    #newfilename="./csv/opt_col/csv_c${capacity}_n${n}_r${range}_fn${instanceNr}_mode${mode}_div${division}_cut${cut}.csv"
    #newfilename="./csv/columns/csv_c${capacity}_n${n}_r${range}_fn${instanceNr}_mode${mode}_div${division}_cut${cut}.csv"
    #newfilename="./csv/ks/csv_c${capacity}_n${n}_r${range}_fn${instanceNr}_mode${mode}_div${division}_cut${cut}.csv"
    #newfilename="./csv/columns_t/csv_c${capacity}_n${n}_r${range}_fn${instanceNr}_mode${mode}_div${division}_cut${cut}.csv"
    newfilename="./csv/approx/csv_c${capacity}_n${n}_r${range}_fn${instanceNr}_mode${mode}_div${division}_cut${cut}.csv"

    #newfilename="./csv/columns/csv_c${capacity}_n${n}_r${range}_fn${instanceNr}_mode${mode}_div${division}_cut${cut}.csv"
    # newfilename="./csv/csv_test1.csv"
    #newfilename="./csv/csv_test2.csv"
    # newfilename="./csv/buffer.txt"
    #newfilename="./csv/csv_test3.csv"
    #newfilename="./csv/mincap_test.csv"

    #### generator
    cd generator
    gcc derived_instance_generator_int.c -o derived_instance_generator_int -lm

    ./gen.out $n $range 1 1 1000
    mv test.in files/uncorr
    ./derived_instance_generator_int files/uncorr $instanceNr

    ./gen.out $n $range 2 2 1000
    mv test.in files/weak_corr
    ./derived_instance_generator_int files/weak_corr $instanceNr

    ./gen.out $n $range 3 3 1000
    mv test.in files/str_corr
    ./derived_instance_generator_int files/str_corr $instanceNr

    ./gen.out $n $range 4 4 1000
    mv test.in files/inv_str_corr
    ./derived_instance_generator_int files/inv_str_corr $instanceNr

    ./gen.out $n $range 5 5 1000
    mv test.in files/alm_str_corr
    ./derived_instance_generator_int files/alm_str_corr $instanceNr

    rm files/derived/int/*
    cd files
    files=$(find . -maxdepth 1 -name '*[[:digit:]]')
    mv $files derived/int
    cd ..
    cd ..
    ####

    #### RUNV4
    echo "gcc binaryKnapsack_ks2_mem_opt.c -Ilibs -Llibs -lfminknap_npg -o binaryKnapsack -lm && ./binaryKnapsack ${filenames} ${capacity} ${mode} ${cut} ${division}"
    echo "generating ${newfilename}"
    echo "please wait..."

    #gcc binaryKnapsack_ks2_mem_opt.c -Ilibs -Llibs -lfminknap_npg -o binaryKnapsack -lm && ./binaryKnapsack $filenames $capacity > $newfilename
    ./binaryKnapsack $filenames $capacity $mode $cut $division > $newfilename
    ####

    echo "done."
    #exit

    # sh ./runv4.sh > ./csv/csv_c50000_n20_r30000_fn100.csv
done

echo "copying..."
cp -r ./csv/* /mnt/c/Users/loren/Desktop/Tesi/jupyter/csv/
echo "ok."
exit
