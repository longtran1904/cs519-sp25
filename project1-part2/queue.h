#ifndef QUEUE_H
#define QUEUE_H

// Node structure for each element in the queue.
typedef struct Node {
    int data;
    struct Node *next;
} Node;

// Queue structure maintaining pointers to the front and rear.
typedef struct Queue {
    Node *front;
    Node *rear;
} Queue;

// Creates an empty queue.
Queue* createQueue(void);

// Enqueues an element to the rear of the queue.
void enqueue(Queue* q, int data);

// Dequeues an element from the front of the queue.
void dequeue(Queue* q);

// Prints all elements in the queue.
void printQueue(Queue* q);

#endif // QUEUE_H