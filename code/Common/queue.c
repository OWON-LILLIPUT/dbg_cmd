
/*
 * file encoding utf-8
 *
 * queue.c
 *
 * Copyright (C) 2021 Inker.Dong
 *
 * Change Activity:
 *
 */

#include "queue.h"

void init_queue(struct queue_s *queue_name)
{
    queue_name->front = 0;
    queue_name->rear  = 0;
}

/* rear == front  is empty */
char is_queue_empty(struct queue_s *queue_name)
{
    return (queue_name->front == queue_name->rear) ? (QUEUE_EMPTY) : (QUEUE_NORMAL);
}

/* (rear + 1) == front  is full*/
char is_queue_full(struct queue_s *queue_name)
{
    unsigned char tmp = queue_name->rear + 1;
    if(tmp >= queue_name->queue_max_element) {
        tmp = 0;
    }
    return (tmp == queue_name->front) ? (QUEUE_FULL) : (QUEUE_NORMAL);
}

char out_queue(struct queue_s *queue_name, char *out_data)
{
    if (is_queue_empty(queue_name) == QUEUE_NORMAL) {
        *out_data = queue_name->queue_buf[queue_name->front];
        queue_name->front++;
        if(queue_name->front >= queue_name->queue_max_element) {
            queue_name->front = 0;
        }
        return QUEUE_NORMAL;
    }
    return QUEUE_EMPTY;
}
char in_queue(struct queue_s *queue_name, char in_data)
{
    if (is_queue_full(queue_name) == QUEUE_NORMAL) {
        queue_name->queue_buf[queue_name->rear] = in_data;
        queue_name->rear++;
        if(queue_name->rear >= queue_name->queue_max_element) {
            queue_name->rear = 0;
        }
        return QUEUE_NORMAL;
    }
    return QUEUE_FULL;
}
