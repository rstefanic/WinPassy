#pragma once

#include <stdio.h>
#include <windows.h>
#include <Wincrypt.h>

BYTE* encrypt(const char* strToEncrypt, const char* password, DWORD* dwSize);
const char* decrypt(BYTE* encryptedBytes, const char* password, DWORD blobSize);
void byteToString(DWORD cb, void* pv, LPSTR sz);
void handleEncryptError(char* message);
