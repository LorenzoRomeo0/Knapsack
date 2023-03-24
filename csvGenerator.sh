# testnr=1
# for i in `seq 1 $testnr`
# do
#     sh ./runv4.sh > ./csv/csv2.csv
#     echo $i
# done

sh ./runv4.sh > ./csv/csv3.csv
cp ./csv/* /mnt/c/Users/loren/Desktop/Tesi/jupyter/csv/