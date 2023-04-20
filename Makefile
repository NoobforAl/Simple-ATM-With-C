ATM: sha256.c sha256.h main.c
	gcc -O3 -lsqlite3 -Wall -Wextra -pedantic main.c sha256.c -o atm