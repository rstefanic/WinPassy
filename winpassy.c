#include "winpassy.h"
#include "encrypt.h"
#include "file.h"
#include "hash.h"

// using main over wmain because the LPSTR characters 
// from wmain mess up the char** argv argument in main
int main(int argc, char** argv) 
{
    unsigned int wPassyFileExists = 0;
    unsigned int copyToClipboardFlag = 0;
    unsigned int deleteFlag = 0;
    unsigned int requestedServiceGiven = 0;
    char requestedService[SERVICE_NAME_BUFFER_SIZE];
    char masterPassword[PASSWORD_BUFFER_SIZE] = { 0 };

    // check if wpassy file exists
    char* pathToWPassy = appendToUserProfile("\\.wpassy");
    wPassyFileExists = checkIfFileExists(pathToWPassy);

    // If a new file must be created, then walk them
    // through the create new file prompts and exit
    if (!wPassyFileExists) 
    {
        printf("No wpassy file was found for this user.");
        createNewFilePrompt(pathToWPassy);
        return 0;
    }
    
    // Build the WPassy struct in memory
    WPassy wpassy;
    if (readWPassyFile(pathToWPassy, &wpassy) == FALSE) 
    {
        printf("Error reading WPassy file.\n");
        return -1;
    }

    // Exit the program if the user didn't give us a command
    if (argc < 2) 
    {
        printf("No command given.");
        printUsage();
        return 0;
    }

    // Get the user's master password
    printf("Please enter your password: ");
    getPassword(masterPassword, PASSWORD_BUFFER_SIZE);
    printf("\n");

    const char* hashedPassword = generateHash(masterPassword);

#if DEBUG 
    // Double check the password hashes
    debugPrint("User entered hashed pw: %s\n", hashedPassword);
    debugPrint("WPASSY hashed pw: %s\n", wpassy.hashedPassword);
#endif

    // Check the user's password to make sure the hashes match
    if (strcmp(hashedPassword, wpassy.hashedPassword) != 0) 
    {
        printf("Passwords do not match.\n");
        return 0;
    }

    // Parse arguments and flags
    for (int i = 1; i < argc; i++) 
    {
        if (strcmp("-h", argv[i]) == 0)
        {
            printUsage();
            return 0;
        }

        if (strcmp("-a", argv[i]) == 0)
        {
            addServicePrompt(&wpassy, masterPassword, pathToWPassy);
            return 0;
        }

        if (strcmp("-c", argv[i]) == 0) 
        {
            copyToClipboardFlag = 1;
            continue;
        }

        if (strcmp("-l", argv[i]) == 0) 
        {
            printf("All available services:\n\n");
            listAllServices(wpassy.service);
            printf("\n");
            return 0;
        }

        if (strcmp("-d", argv[i]) == 0)
        {
            deleteFlag = 1;
            continue;
        }

        // If none of the above flags are set, take the argument as a service
        // Only one service can be taken as an argument
        if (requestedServiceGiven == 0) 
        {
            strcpy_s(requestedService, SERVICE_NAME_BUFFER_SIZE, argv[i]);
            requestedServiceGiven = 1;
        }
        else 
        {
            printf("More than one service has been requested.\n");
            printf("Ignoring the following argument: %s\n", argv[i]);
        }
    }

    // Make sure a service was actually passed
    if (requestedService[0] == '\0')
    {
        printf("No service passed as an argument");
        printUsage();
        return -1;
    }

    // Process deletion if requested and exit the program
    if (deleteFlag) 
    {
        unsigned int deletionSuccess = deleteServiceByName(
            &(wpassy.service), 
            requestedService
        );

        if (deletionSuccess == -1) 
        {
            printf("There are no services to delete.\n");
        }
        else if (deletionSuccess == -2)
        {
            printf("Service %s was not found.\n", requestedService);
            printf("Perhaps you misspelled the service name?\n");
            printf("Try running 'winpassy -l' to list all ");
            printf("the available services.\n");
        }
        else 
        {
            printf("Service %s successfully removed.\n", requestedService);
        }

#if DEBUG
        listAllServices(wpassy.service);
#endif

        // Only rewrite the file if there were no errors
        if (deletionSuccess == 0)
        {
            // If it was successfully deleted, decrement the size count
            // and rewrite the file
            wpassy.listSize--; 
            writeWPassyFile(pathToWPassy, &wpassy);
        }
           
        return deletionSuccess;
    }

    // Process the requested service
    Service* s;
    if (findRequestedService(wpassy.service, requestedService, &s) == 0)
    {
        printf("Service not found.\n");
    }
    else
    {
        // Decrypt the password, and show it to the user
        const char* passwordDecrypted = decrypt(s->password, masterPassword, s->blobSize);

        printf("Service Name: %s\n", s->serviceName);
        printf("Username: %s\n", s->account);
        printf("Password : %s\n", passwordDecrypted);

        if (copyToClipboardFlag)
        {
            copyToClipboard(passwordDecrypted);
        }
    }

    // Clean up
    free(pathToWPassy);

    return 0;
}

/*
================================================================================
getPassword

Prompts the user for their password, and masks the user's input.
Stores the password that the user entered into the passwordBuffer.
================================================================================
*/
void getPassword(char* passwordBuffer, int bufferLength) 
{
    char c;

    for (int i = 0; i < bufferLength; i++)
    {
        c = _getch();

        // Enter key pressed
        if (c == 13)
            break;

        // Backspace key pressed
        if (c == 8)
        {
            i--;
            continue;
        }

        passwordBuffer[i] = c;
        _putch('*');
    }
}

/*
================================================================================
createService

Builds a new service and returns a pointer to the newly created service.
================================================================================
*/
Service* createService(const char* serviceName, const char* account, 
    DWORD blobSize, BYTE* password)
{
    Service* service = malloc(sizeof(Service));
    strcpy_s(service->serviceName, SERVICE_NAME_BUFFER_SIZE, serviceName);
    strcpy_s(service->account, 100, account);
    service->blobSize = blobSize;
    memcpy(service->password, password, PASSWORD_BYTE_LIMIT);
    service->next = NULL;

    return service;
}

/*
================================================================================ 
createNewFilePrompt

Prompts the user for a password and asks them to confirm it. After the new
password has been confirmed, the password is hashed and a new WPASSY struct
is written to the given path. If the passwords do not match, then no new 
file is written.
================================================================================ 
*/
void createNewFilePrompt(const char* pathToWPassy)
{
    char password[PASSWORD_BUFFER_SIZE] = { 0 };
    char confirmPassword[PASSWORD_BUFFER_SIZE] = { 0 };

    printf("\nEnter new password >> ");
    getPassword(password, PASSWORD_BUFFER_SIZE);

    printf("\nPlease confirm your new password >> ");
    getPassword(confirmPassword, PASSWORD_BUFFER_SIZE);

    printf("\n");

    if (strcmp(password, confirmPassword) == 0) 
    {
        // Create new Passy struct, and hash this password
        WPassy newWPassy;

        // Set the values for the new wpassy file; No services yet added
        newWPassy.listSize = 0;
        newWPassy.service = NULL;

        const char* hashedPassword = generateHash(password);

        strncpy_s(newWPassy.hashedPassword,
            PASSWORD_BYTE_LIMIT,
            hashedPassword,
            PASSWORD_BYTE_LIMIT
        );

        writeWPassyFile(pathToWPassy, &newWPassy);
    }
    else
    {
        printf("\nPasswords do not match.");
        printf("\nNo wpassy file was created.");
        printf("\n");
    }
}

/*
================================================================================
addServicePrompt

Prompts the user for information in order to add a new service to wpassy. 
Adds the service to the end of the services linked list on wpassy, 
encrypts the service's password with the masterPassword that's given,
and overwrites the wpassy file at the path given.
================================================================================
*/
void addServicePrompt(WPassy* wpassy, const char* masterPassword, const char* pathToWPassy)
{
    char serviceName[SERVICE_NAME_BUFFER_SIZE] = { 0 };
    char account[ACCOUNT_BUFFER_SIZE] = { 0 };
    char password[PASSWORD_BUFFER_SIZE] = { 0 };
    char confirmPassword[PASSWORD_BUFFER_SIZE] = { 0 };

    DWORD encryptionSize;

    printf("\nEnter new service name >> ");
    fgets(serviceName, SERVICE_NAME_BUFFER_SIZE, stdin);

    printf("\nEnter new account name >> ");
    fgets(account, ACCOUNT_BUFFER_SIZE, stdin);

    printf("\nEnter new master password >> ");
    getPassword(password, PASSWORD_BUFFER_SIZE);

    printf("\nPlease confirm your new password >> ");
    getPassword(confirmPassword, PASSWORD_BUFFER_SIZE);

    printf("\n");

    // Remove the newline character from each input
    size_t len = strlen(serviceName);
    serviceName[--len] = '\0';

    len = strlen(account);
    account[--len] = '\0';

    // The 'getPassword()' already omits the new line character, so
    // both password and confirmPassword just need to be null terminated
    len = strlen(password);
    password[len] = '\0';

    len = strlen(confirmPassword);
    confirmPassword[len] = '\0';

    if (strcmp(password, confirmPassword) == 0) 
    {
        // Encrypt the service's password using the masterPassword
        BYTE* encryptedPassword = encrypt(password, masterPassword, &encryptionSize);
        Service* newService = malloc(sizeof(struct SERVICE));
        newService = createService(serviceName, account, encryptionSize, encryptedPassword);
        appendServiceToServices(&(wpassy->service), newService);

        // Increment the wpassy counter
        wpassy->listSize = wpassy->listSize + 1;

        // Re-write the file
        writeWPassyFile(pathToWPassy, wpassy);
    }
}

/*
================================================================================
appendServiceToServices

Takes a pointer to a pointer of services and a service to be appended,
and adds the service to the end of the services linked lists.
================================================================================
*/
void appendServiceToServices(Service** head, Service* serviceToAppend)
{
    // Check to make sure the head isn't NULL
    if (*head == NULL)
    {
        // If it is null, then just set the head
        // to the new service
        *head = serviceToAppend;
        return;
    }

    Service* temp = *head;
    while (temp->next != NULL)
    {
        temp = temp->next;
    }

    temp->next = serviceToAppend;
}

/*
================================================================================
findRequestedService

Finds the requested service from the start of the linked list. If the service
is found, it assigns it to the the pRequestedService parameter. Returns a 1 if 
a match was found, 0 if there was no match found, or a -1 if there was an error.
================================================================================
*/
unsigned int findRequestedService(
    Service* head, 
    const char* requestedService, 
    Service** pRequestedService)
{
    *pRequestedService = NULL;

    if (head == NULL)
    {
        return -1; 
    }

    // Walk through the services until a match is found
    Service* currentService = head;
    while (currentService != NULL)
    {
        // Does the service requested match the current service?
        if (strcmp(currentService->serviceName, requestedService) == 0)
        {
            *pRequestedService = currentService;
            break;
        }

        currentService = currentService->next;
    }

    // If pRequestedService is still null, then there was no match
    if (*pRequestedService == NULL)
    {
        return 0;
    }

    // Otherwise, report that a match was found
    return 1;
}

/*
================================================================================
listAllServices

Walks through all of the services and prints the service names.
================================================================================
*/
void listAllServices(Service* head)
{
    if (head == NULL) 
    {
        printf("There are no services to list.\n");
        return;
    }

    printf("- %s\n", head->serviceName);

    Service* nextService = head->next;
    while (nextService != NULL) 
    {
        printf("- %s\n", nextService->serviceName);
        nextService = nextService->next;
    }
}

/*
================================================================================
deleteServiceByName

Walks the linked list of services and deletes the requestService by name.
If the service is found, then it will be removed from the list 
by assigning the previous service node to the following service's node. 
Returns a -1 if the list was null, 0 if the removal was a success,
and a -2 if the service was not found.
================================================================================
*/
unsigned int deleteServiceByName(Service** head, const char* requestedService)
{
    // Return an error if there are no services
    if (*head == NULL) 
    {
        return -1;
    }

    Service* previousService = *head;
    Service* nextService = (*head)->next; 

    // Check if we can just return the tail of the services
    if (strcmp(requestedService, previousService->serviceName) == 0)
    {
        *head = nextService;
        return 0;
    }

    // Otherwise proceed to check the remaining services
    while (nextService != NULL) 
    {
        if(strcmp(requestedService, nextService->serviceName) == 0)
        {
            // If this is the service to be removed, just omit it from the list
            previousService->next = nextService->next;
            return 0;
        }

        previousService = nextService;
        nextService = nextService->next;
    }

    return -2;
}

/*
================================================================================
copyToClipboard

Copies a string to the Windows clipboard.
================================================================================
*/
void copyToClipboard(const char* str)
{
    const size_t len = strlen(str) + 1;
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);

    memcpy(GlobalLock(hMem), str, len);
    GlobalUnlock(hMem);
    OpenClipboard(0);
    EmptyClipboard();
    SetClipboardData(CF_TEXT, hMem);
    CloseClipboard();
}

/*
================================================================================
printUsage

Prints the program's usage.
================================================================================
*/
void printUsage()
{
    printf("usage: winpassy [ -a | -l ] [ -c ] [ -d ] <service name>\n");
    printf("\t -h : Print this usage message\n");
    printf("\t -a : Add - adds a service to your wpassy file\n");
    printf("\t -l : list - lists all the services in your wpassy file\n");
    printf("\t -d : delete - deletes a service in your wpassy file\n");
    printf("\t -c : copy - copies a service's decrypted password to your clipboard\n");
    printf("\t -d : delete - deletes a service in your wpassy file\n");
    printf("\t\t a <service name> must be specified \n");
}

#if DEBUG
/*
================================================================================
debugPrint

If the DEBUG symbol is defined, then debug print will be available.
debugPrint takes any amount of arguments, and a format for them to be printed
in, and prefixes the word "DEBUG" to the beginning of the formated string
to be printed.
================================================================================
*/
void debugPrint(const char* format, ...)
{
    // DEBUG print prefix 
    printf("[DEBUG]: ");

    va_list argp;
    va_start(argp, format);
    vfprintf(stderr, format, argp);
    va_end(argp);
}
#endif
