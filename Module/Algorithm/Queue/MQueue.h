#pragma once
#include "stdlib.h"
#include "stdint.h"

typedef struct {
    int count;
    float *array;
    int front;
    int rear;
    int maxSize;
} Queue;

void queueInit(Queue *q, float a[], int m);
void queuePush(Queue *q, float element);
int queuePop(Queue *q);
void queuePrint(Queue *q);


