#include "cross_relation.h"

#ifndef SHARED_H
#define SHARED_H

void storeFileNames(int nfileNames, char *fileNames[] );

void openNextFile();

int processConvPoint(int threadId, int fileId, int n, double * x, double * y, int point);

double computeValue(int n, double * x, double * y, int point);

void savePartialResults(int threadId, int fileId, int point, double val);

void storeResults();

#endif 