#! /bin/bash

gcc derived_instance_generator_int.c -o derived_instance_generator_int -lm

n=20
#range=20
#range=100
#range=4300
#range=20300
range=30000
#range=50000
instanceNr=100

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


exit
#cat test.in | awk '{print $1}' | sed '1d;$d' > 1_10_uncorr.in