#include <stdio.h>
#include "thread_worker_types.h"

// #define unsigned int worker_t

typedef struct Node {
    worker_t data;
    tcb* control;
    struct Node* next;
} Node;

typedef struct {
    Node* front;
    Node* back;
} LinkedList;

// Function to initialize an empty linked list
void initialize(LinkedList* list) {
    list->front = NULL;
    list->back = NULL;
}

// Function to check if the list is empty
int isempty(LinkedList* list) {
    return list->front == NULL;
}

// Function to find a node with a certain value
Node* findNode(LinkedList* list, worker_t value) {
    if (isempty(list)) {
        // fprintf(stderr, "Cannot search an empty list\n");
        return NULL;
    }

    Node* current = list->front;
    while (current != NULL) {
        if (current->data == value) {
            return current; // Node found
        }
        current = current->next;
    }

    fprintf(stderr, "Node with value %d not found\n", value);
    return NULL;
}

// Function to insert a new element at the back of the list
void insert(LinkedList* list, worker_t newData, tcb* control) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    newNode->data = newData;
    newNode->control = control;
    newNode->next = NULL;

    if (isempty(list)) {
        // If the list is empty, the new node becomes both front and back
        list->front = newNode;
        list->back = newNode;
    } else {
        // If the list is not empty, insert the new node at the back
        list->back->next = newNode;
        list->back = newNode;
    }
}

// Function to delete the front element 
void delete_first(LinkedList* list) {
    if (isempty(list)) {
        fprintf(stderr, "Cannot delete from an empty list\n");
        return;
    }

    Node* temp = list->front;
    list->front = list->front->next;

    // If the list becomes empty after deletion, update the back pointer as well
    if (list->front == NULL) {
        list->back = NULL;
    }

    free(temp);
}

void removeNode(LinkedList* list, worker_t value) {
    if (isempty(list)) {
        fprintf(stderr, "Cannot remove from an empty list\n");
        return;
    }

    Node* current = list->front;
    Node* prev = NULL;

    // Traverse the list to find the node with the specified value
    while (current != NULL && current->data != value) {
        prev = current;
        current = current->next;
    }

    // If the value is not found
    if (current == NULL) {
        fprintf(stderr, "Node with value %d not found\n", value);
        return;
    }

    // If the node to be removed is the front
    if (prev == NULL) {
        list->front = current->next;
    } else {
        prev->next = current->next;
    }

    // If the node to be removed is the back
    if (current == list->back) {
        list->back = prev;
    }

    // free(current);
}

Node* getNext(LinkedList* list) {
    if (isempty(list)) {
        perror("Run Queue is empty");
        return NULL;
    }

    Node* temp = list->front;
    delete_first(list);
    return temp;
}

// Function to print out the entire list
void display(LinkedList* list) {
    if (isempty(list)) {
        printf("List is empty\n");
        return;
    }

    Node* current = list->front;
    while (current != NULL) {
        // Modify this part based on the structure of worker_t
        printf("ID: %d\n", current->data);
        current = current->next;
    }
}

// Function to free the memory allocated for the linked list
void destroyList(LinkedList* list) {
    while (!isempty(list)) {
        delete_first(list);
    }
}
