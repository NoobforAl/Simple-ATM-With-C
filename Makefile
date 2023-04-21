tests: test/main.c ATM/atm.c SHA-256/sha256.c
	gcc -O3 -Wall -Wextra -pedantic -lsqlite3 -o atm_test ./test/main.c SHA-256/sha256.c ATM/atm.c -I.

ATM: main.c ATM/atm.c SHA-256/sha256.c
	gcc -O3 -Wall -Wextra -pedantic -lsqlite3 -o atm main.c SHA-256/sha256.c ATM/atm.c -I.