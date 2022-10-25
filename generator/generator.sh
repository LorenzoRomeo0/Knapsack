#! /bin/bash

./gen.out 10 100 1 1 1000
mv test.in files/1_10_uncorr.in
#cat test.in | awk '{print $1}' | sed '1d;$d' > 1_10_uncorr.in

./gen.out 10 100 2 2 1000
mv test.in files/2_10_weak_corr.in

./gen.out 10 100 3 3 1000
mv test.in files/3_10_str_corr.in

./gen.out 10 100 4 4 1000
mv test.in files/4_10_inv_str_corr.in

./gen.out 10 100 5 5 1000
mv test.in files/5_10_alm_str_corr.in

exit