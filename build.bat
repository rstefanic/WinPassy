IF NOT EXIST ".\bin" md .\bin
clang -target x86_64-pc-windows-gnu -Wall --std=c99 winpassy.c hash.c encrypt.c file.c -Iinc --output ./bin/winpassy.exe
