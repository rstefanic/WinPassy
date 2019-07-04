#pragma once
#include "winpassy.h"

char* appendToUserProfile(const char* file);
unsigned int checkIfFileExists(const char* file);
unsigned int writeWPassyFile(const char* file, WPassy* wpassy);
unsigned int readWPassyFile(const char* fileLocation, WPassy* wpassy);
