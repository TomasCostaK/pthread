#include "main.h"

#ifndef SHARED_H
#define SHARED_H


void openNextFile();

void storeFileNames(int nfileNames, char *fileNames[] );

int getDataChunk(int threadId, char buff[], struct PartialInfo *partialInfo);

void savePartialResults(int threadId, struct PartialInfo partialInfo);

void printProcessingResults();

#endif 