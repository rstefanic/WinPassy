#pragma once
#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#define DEBUG 0

#if DEBUG
#include <stdarg.h>
#endif

#define PASSWORD_BUFFER_SIZE 50
#define SERVICE_NAME_BUFFER_SIZE 100
#define ACCOUNT_BUFFER_SIZE 100
#define PASSWORD_BYTE_LIMIT 512

// Service is a linked list
typedef struct SERVICE {
    char serviceName[SERVICE_NAME_BUFFER_SIZE];
    char account[ACCOUNT_BUFFER_SIZE];
    DWORD blobSize;
    BYTE password[PASSWORD_BYTE_LIMIT];
    struct SERVICE* next;
} Service;

// The Layout of the WPASSY file 
typedef struct WPASSY {
    size_t listSize;
    char hashedPassword[PASSWORD_BYTE_LIMIT];
    Service* service;
} WPassy;

void getPassword(char* passwordBuffer, int bufferLength);
Service* createService(const char* service, const char* account, 
    DWORD blobSize, BYTE* password);
void createNewFilePrompt(const char* pathToWPassy);
void addServicePrompt(WPassy* wpassy, const char* masterPassword, 
    const char* pathToWPassy);
void appendServiceToServices(Service** head, Service* newService);
unsigned int findRequestedService(Service* head, 
    const char* requestedService, Service** pRequestedService);
void listAllServices(Service* head);
unsigned int deleteServiceByName(Service** head, const char* requestedService);
void copyToClipboard(const char* str);
void printUsage();

#if DEBUG
void debugPrint(const char* format, ...);
#endif
