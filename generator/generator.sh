#! /bin/bash

n=100 #massimo senza segfault
#n=10
#range=10100 #massimo senza segfault
range=50000
#range=474
#range=30

./gen.out $n $range 1 1 1000
mv test.in files/uncorr
./derived_instance_generator files/uncorr 100

./gen.out $n $range 2 2 1000
mv test.in files/weak_corr
./derived_instance_generator files/weak_corr 100

./gen.out $n $range 3 3 1000
mv test.in files/str_corr
./derived_instance_generator files/str_corr 100

./gen.out $n $range 4 4 1000
mv test.in files/inv_str_corr
./derived_instance_generator files/inv_str_corr 100

./gen.out $n $range 5 5 1000
mv test.in files/alm_str_corr
./derived_instance_generator files/alm_str_corr 100

rm files/derived/*
cd files
files=$(find . -maxdepth 1 -name '*[[:digit:]]')
mv $files derived
cd ..


exit
#cat test.in | awk '{print $1}' | sed '1d;$d' > 1_10_uncorr.in