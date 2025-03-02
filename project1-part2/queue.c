#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

// Internal helper: creates a new node with the given data.
static Node* newNode(int data) {
    Node* temp = (Node*)malloc(sizeof(Node));
    if (!temp) {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }
    temp->data = data;
    temp->next = NULL;
    return temp;
}

// Creates an empty queue.
Queue* createQueue(void) {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    if (!q) {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }
    q->front = q->rear = NULL;
    return q;
}

// Enqueues an element to the rear of the queue.
void enqueue(Queue* q, int data) {
    Node* temp = newNode(data);

    // If the queue is empty, new node becomes both front and rear.
    if (q->rear == NULL) {
        q->front = q->rear = temp;
        return;
    }

    // Add the new node at the end and update rear.
    q->rear->next = temp;
    q->rear = temp;
}

// Dequeues an element from the front of the queue.
void dequeue(Queue* q) {
    if (q->front == NULL)
        return; // Queue is empty, nothing to dequeue.

    // Store the current front and move front to the next node.
    Node* temp = q->front;
    q->front = q->front->next;

    // If the queue becomes empty, update rear to NULL.
    if (q->front == NULL)
        q->rear = NULL;

    free(temp);
}

// Prints all the elements in the queue.
void printQueue(Queue* q) {
    Node* temp = q->front;
    while (temp != NULL) {
        printf("%d ", temp->data);
        temp = temp->next;
    }
    printf("\n");
}