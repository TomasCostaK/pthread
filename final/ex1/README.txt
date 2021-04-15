gcc -Wall -o ex1 main.c shared.c helperfuncs.c -lpthread

./ex1 -t 4 -i texts/text1.txt -i texts/text2.txt -i texts/text3.txt
