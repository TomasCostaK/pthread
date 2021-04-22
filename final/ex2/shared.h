#ifndef SHARED_H
#define SHARED_H


void openNextFile();

void storeFileNames(int nfileNames, char *fileNames[] );

void savePartialResults(int threadID, struct PartialInfo partialInfo);

#endif 