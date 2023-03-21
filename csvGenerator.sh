echo "typename, columOpt, fminknap" > ./csv/csv1.csv

for i in `seq 1 100` 
do 
    sh ./runv4.sh >> ./csv/csv1.csv
    echo $i
done