#include "hash.h"

/*
================================================================================
generateHash

Generates a hash from a given phrase. A pointer to the hashed phrase is 
returned. It's the responsibility of the caller to free the returned hash 
memory allocated by this function.
================================================================================
*/
const char* generateHash(const char* phrase) 
{
    // Memory allocation for the string to be returned
    char* strHash = malloc(500);
    char* hex = "0123456789abcdef";
    BYTE hash[64];
    DWORD cbHash = 64;

    HCRYPTPROV hCryptProv;
    HCRYPTHASH hHash;
    DWORD dwLength;

    // Get the length of the phrase to hash
    dwLength = (DWORD)strlen(phrase);

    // Get a handle for a key container
    if (!CryptAcquireContext(&hCryptProv, NULL, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, 0))
    {
        handleHashError("Error during CryptAcquireContext!");
    }

    // Create the hash using the handle
    if (!CryptCreateHash(hCryptProv, CALG_SHA_512, 0, 0, &hHash))
    {
        handleHashError("Error during CryptCreateHash!");
    }

    // Hash the data with the hash that was just created
    if (!CryptHashData(hHash, (BYTE*)phrase, dwLength, 0))
    {
        handleHashError("Error during CryptHashData!");
    }

    // Get the hash value out
    if (!CryptGetHashParam(hHash, HP_HASHVAL, hash, &cbHash, 0))
    {
        handleHashError("Error during CryptGetHashParam!");
    }

    // Convert the hash blob data into a hex string to be returned
    for (int i = 0; i < cbHash; i++)
    {
        strHash[i * 2] = hex[hash[i] >> 4];
        strHash[(i * 2) + 1] = hex[hash[i] & 0XF];
    }

    strHash[cbHash * 2] = '\0';

    // Function Cleanup
    if (hHash) 
    {
        if(!(CryptDestroyHash(hHash)))
        {
            handleHashError("Error during CryptDestroyHash");
        }
    }

    if(hCryptProv)
    {
        if(!(CryptReleaseContext(hCryptProv, 0))) 
        {
            handleHashError("Error during CryptReleaseContext");
        }
    }

    return strHash;
}

/*
================================================================================
handleHashError

This function is used to report an error that may have occurred while
hashing, and then exit the program. The program will be terminated once this
function is called.
================================================================================
*/
void handleHashError(const char* message)
{
    printf("An error occured when hashing a phrase.\n");
    printf("%s\n", message);
    printf("Error number: %lx\n", GetLastError());
    exit(1);
}
