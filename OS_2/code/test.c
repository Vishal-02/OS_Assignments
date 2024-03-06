#include <stdio.h>
#include "linked_list.h"
#include "thread_worker_types.h"

int main() {
    LinkedList* list = (LinkedList*)malloc(sizeof(LinkedList));
    tcb* thing = (tcb*)malloc(sizeof(tcb));
    insert(list, 1, thing);
    insert(list, 2, thing);
    insert(list, 3, thing);
    removeNode(list, 2);

    display(list);
    

    return 0;
}