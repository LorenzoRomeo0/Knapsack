#! /bin/bash

n=10
range=5

./gen.out $n $range 1 1 1000
mv test.in files/uncorr.in
#cat test.in | awk '{print $1}' | sed '1d;$d' > 1_10_uncorr.in

./gen.out $n $range 2 2 1000
mv test.in files/weak_corr.in

./gen.out $n $range 3 3 1000
mv test.in files/str_corr.in

./gen.out $n $range 4 4 1000
mv test.in files/inv_str_corr.in

./gen.out $n $range 5 5 1000
mv test.in files/alm_str_corr.in

exit