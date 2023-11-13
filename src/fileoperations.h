#pragma once
#include "fileoperations.h"
#include "FS.h"

void createDir(fs::FS &fs, const char * path);

void appendFile(fs::FS &fs, const char * path, const char * message);

void deleteFile(fs::FS &fs, const char * path);