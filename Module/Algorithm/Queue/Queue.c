//
// Created by liuxi on 2023/12/13.
//

#include "MQueue.h"
#include "stdio.h"


void queueInit(Queue *q, float a[], int m)
{
    q->maxSize = m;
    q->array   = a;
    q->count   = 0;
    q->front   = 0;
    q->rear    = -1;
}

/*
    向队列中加入
*/
void queuePush(Queue *q, float element)
{
    if (q->count == q->maxSize) {
        return;
    }
    q->rear++;
    q->rear %= q->maxSize;
    q->array[q->rear] = element;
    q->count++;
}

/*
    从队列中取出
*/
int queuePop(Queue *q)
{
    if (q->count == 0) {
        return -1;
    }
    int res            = q->array[q->front];
    q->array[q->front] = 0;
    q->front++;
    q->front %= q->maxSize;
    q->count--;
    return res;
}

/*
    遍历队列
*/
void queuePrint(Queue *q)
{
    int pointer = q->front;
    if (q->count == 0) {
        return;
    } else if (q->front <= q->rear) {
        while (pointer <= q->rear) {
            printf("%d ", q->array[pointer]);
            pointer++;
        }
    } else {
        for (; pointer <= q->maxSize - 1; pointer++) {
            printf("%d ", q->array[pointer]);
        }
        for (pointer = 0; pointer <= q->rear; pointer++) {
            printf("%d ", q->array[pointer]);
        }
    }
    printf("\n ");
}