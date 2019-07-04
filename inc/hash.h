#pragma once
#pragma comment(lib, "crypt32")
#pragma comment(lib, "advapi32.lib")

#include <stdio.h>
#include <windows.h>
#include <Wincrypt.h>

const char* generateHash(const char* phrase);
void handleHashError(const char* message);
