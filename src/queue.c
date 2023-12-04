#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
        if (q == NULL) return 1;
	return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
        /* TODO: put a new process to queue [q] */
        if(proc == NULL) return;
        if(q->size >= MAX_QUEUE_SIZE){
                return;
        }
        q->proc[q->size] = proc;
        q->size++; 
}

struct pcb_t * dequeue(struct queue_t * q) {
        /* TODO: return a pcb whose prioprity is the highest
         * in the queue [q] and remember to remove it from q
         * */
        if(q->size <= 0) return NULL;
        // find process have highest priority
        int find_proc_id = 0;
        int highest_prio = MAX_PRIO;
        int current_prio;
        for(int i = 0; i < q->size; i++){
                current_prio = q->proc[i]->prio;
                if(current_prio < highest_prio){
                        highest_prio = current_prio;
                        find_proc_id = i;
                }
        }
        struct pcb_t* proc_return = q->proc[find_proc_id];
        // remove process from queue
        for(int i = find_proc_id; i < (q->size-1); i++){
                q->proc[i] = q->proc[i+1];
        }
        q->proc[q->size-1] = NULL;
        q->size--;
	return proc_return;
	return NULL;
}

