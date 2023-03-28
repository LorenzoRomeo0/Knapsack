#capacity=1000
# capacity=300
#capacity=20300 #massimo senza segfault per minCap con allocazione su stack
capacity=50000 

#capacity=500

files="./generator/files/derived/int/"
#gcc binaryKnapsack4.1.c -Ilibs -Llibs -lfminknap_npg -o binaryKnapsack -lm && ./binaryKnapsack $files $capacity > buffer.txt
#gcc binaryKnapsack4.1.c -Ilibs -Llibs -lfminknap_npg -o binaryKnapsack -lm && ./binaryKnapsack ./generator/files/derived/int/ 300 > buffer.txt
#gcc binaryKnapsack_ks2.c -Ilibs -Llibs -lfminknap_npg -o binaryKnapsack -lm && ./binaryKnapsack $files $capacity
gcc binaryKnapsack_ks2_mem_opt.c -Ilibs -Llibs -lfminknap_npg -o binaryKnapsack -lm && ./binaryKnapsack $files $capacity
#gcc binaryKnapsack_ks2_mem_opt.c -Ilibs -Llibs -lfminknap_npg -o binaryKnapsack -lm && ./binaryKnapsack ./generator/files/derived/int/ 20000