#include "file.h"

/*
================================================================================
appendToUserProfile

Appends the file name to the user's profile directory (user's home directory).
================================================================================
*/
char* appendToUserProfile(const char* file)
{
    char* path = (char*)malloc(250 * sizeof(char));
    size_t returnValue = 0;

    getenv_s(&returnValue, path, 250, "USERPROFILE");

    strcat_s(path, 250, file);
    
    return path;
}

/*
================================================================================
checkIfFileExists

Checks if a file exists. The function will return a 1 if the file exists 
or a 0 if it does not exist.
================================================================================
*/
unsigned int checkIfFileExists(const char* file)
{
    int fileExists = TRUE;
    OFSTRUCT buffer;

    HFILE hFile = OpenFile(file, &buffer, OF_EXIST);

    if (hFile == HFILE_ERROR) 
    {
        fileExists = FALSE;
    }

    CloseHandle(&hFile);
    return fileExists;
}

/*
================================================================================
writeWPassyFile

Creates and overwrites the file at the given file path with the wpassy struct
passed in. The file parameter is assumed to have the full file path. 

The newly created file is created with the WinAPI FILE_ATTRIBUTE_HIDDEN.
================================================================================
*/
unsigned int writeWPassyFile(const char* file, WPassy* wpassy)
{
    HANDLE hFile;
    DWORD dwBytesWritten;
    unsigned int writeSuccessful = FALSE;

    // Create the file; Hide the file 
    hFile = CreateFile(file, GENERIC_WRITE, 0, NULL, 
        CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
        return FALSE;
    }

    // TODO: Add error checking with writeSuccessful below

    // Try and write the individual parts to the file
    writeSuccessful = WriteFile(hFile, &(wpassy->listSize),
        (DWORD)sizeof(size_t), &dwBytesWritten, NULL);

    writeSuccessful = WriteFile(hFile, wpassy->hashedPassword,
        (DWORD)sizeof(wpassy->hashedPassword), &dwBytesWritten, NULL);

    // Write all the services to the file
    if (wpassy->service != NULL)
    {
        Service* temp = wpassy->service;

        // At least write the first service
        writeSuccessful = WriteFile(hFile, temp, (DWORD)sizeof(struct SERVICE), 
            &dwBytesWritten, NULL);

        while (temp->next != NULL) 
        {
            temp = temp->next;

            writeSuccessful = WriteFile(hFile, temp, (DWORD)sizeof(struct SERVICE), 
                &dwBytesWritten, NULL);
        }
    }

    CloseHandle(hFile);
    return writeSuccessful;
}

/*
================================================================================
readWPassyFile

Reads the WPASSY struct from the fileLocation and writes it to the wpassy 
struct passed in.

Returns 0 if the function failed, and 1 if it success.
================================================================================
*/
unsigned int readWPassyFile(const char* fileLocation, WPassy* wpassy) 
{
    FILE* file;
    fopen_s(&file, fileLocation, "rb"); // all windows files are "rb"

    if (file == NULL) 
    {
        return FALSE;
    }

    fread(&(wpassy->listSize), sizeof(size_t), 1, file);
    fread(wpassy->hashedPassword, sizeof(wpassy->hashedPassword), 1, file);
    
    // By default, the service is null; then proceed to build the services LL
    wpassy->service = NULL;

    if (wpassy->listSize > 0)
    {
        Service* head = malloc(sizeof(struct SERVICE));
        fread(head, sizeof(struct SERVICE), 1, file);

#if DEBUG
        debugPrint("reading first service: %s\n", head->serviceName);
#endif

        Service* prevService = head;
        // Only attempt to read a service if we know a service exists
        // i starts at one because we've already read the head in
        for (unsigned int i = 1; i < wpassy->listSize; i++)
        {
            Service* currentService = malloc(sizeof(struct SERVICE));
            fread(currentService, sizeof(struct SERVICE), 1, file);

#if DEBUG
            debugPrint("reading subsequent service: %s\n", currentService->serviceName);
#endif

            // Link the new service to the previous one
            prevService->next = currentService;
            // Set the current service to the previous one
            prevService = currentService;
        }

        // Once the linked list has been built, set wpassy's service
        // to the start of the wpassy's service
        // to the start of the list
        wpassy->service = head;
    }

    fclose(file);

    return TRUE;
}

