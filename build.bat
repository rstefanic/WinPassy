cls
clang -Wall -o ./bin/winpassy.exe --std=c99 winpassy.c hash.c encrypt.c file.c -luser32.lib -Iinc
