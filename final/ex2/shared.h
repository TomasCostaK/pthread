#ifndef SHARED_H
#define SHARED_H


void openNextFile();

void storeFileNames(int nfileNames, char *fileNames[] );

double computeValue(int n, double * x, double * y, int point);

void storeResults();

void savePartialResults(int threadId, int fileId, int point, double val);

#endif 