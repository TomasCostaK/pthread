#ifndef SHARED_H
#define SHARED_H


void openNextFile();

void storeFileNames(int nfileNames, char *fileNames[] );

int getDataChunk(int threadId, char buff[]);

#endif 