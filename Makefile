run : main.c inc/*.h src/*.c
	gcc -Wall -Wunused -g  main.c src/*.c -o run -lm
