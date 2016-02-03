/*
 * Chris Fahlin
 * TCSS 422
 * Homework 4
 */
#include "page_manager.h"
#include <stdlib.h>
#include <math.h>

static memory_config config;

typedef struct node {
    void * data;
    struct node * next;
} node;

typedef struct {
    int size;
    node * head;
    node * tail;
} queue;

typedef struct {
    unsigned int physical_page_number;
    unsigned char valid;
}pageEntries;

pageEntries ** pt;
queue q;

/*Begin Aldens Code*/
void queue_init(queue * queue) {
    queue->size = 0;
    queue->head = NULL;
    queue->tail = NULL;
}

void queue_destroy(queue * queue) {
    while (queue->head != NULL) {
        node * toDie = queue->head;
        queue->head = queue->head->next;
        free(toDie);
    }
}

void queue_add(queue * queue, void * item) {
    if (queue->tail == NULL)
        queue->head = queue->tail = (node*)malloc(sizeof(node));
    else {
        queue->tail->next = (node*)malloc(sizeof(node));
        queue->tail = queue->tail->next;
    }
    queue->tail->data = item;
    queue->tail->next = NULL;
    queue->size++;
}

void * queue_remove(queue * queue) {
    if (queue->size == 0) {
        return NULL;
    }
    void * data = queue->head->data;
    node * toDie = queue->head;
    queue->head = queue->head->next;
    free(toDie);
    if (queue->head == NULL)
        queue->tail = NULL;
    queue->size--;
    return data;
}

int queue_isEmpty(queue * queue) {
    return queue->size == 0;
}
/*End Aldens Code*/


void initialize_page_manager(memory_config mc) {
    config = mc;
    queue_init(&q);
    
    
    //Find the number of bits to represent the index
    int virtualPages = (int) exp2(config.virtual_address_space) / config.page_size;
    int bitsOfIndex = log2(virtualPages);
    
    //Find the number of bits that represent the offset
    int bitsOfOffset = config.virtual_address_space - bitsOfIndex;
    
    //find the number of bits to represent a physical page
    int bitsOfPage = config.physical_address_space - bitsOfOffset;
    
    //find the number of physical pages
    int num_of_phys = (int) exp2(bitsOfPage);
    
    
    //Allocate the page table
    pt = malloc(sizeof(pageEntries) * config.processes);
    int i, j;
    for(i = 0; i < config.processes; i++)
        pt[i] = malloc(sizeof(pageEntries) * virtualPages);
    
    //Set the ready bits
    for(i = 0; i < config.processes; i++)
    {
        for(j = 0; j < virtualPages; j++)
        {
            pt[i][j].valid = 1;
            pt[i][j].physical_page_number = 0;
        }
    }
    
    //Add the physical pages to the queue
        for(i = 0; i < num_of_phys; i++){
            queue_add(&q, (void*)i);
        }
    
}

access_result access_memory(unsigned int pid, unsigned int virtual_address) {
    access_result result;
    
    //Find the number of bits to represent the index
    int bitsOfIndex = (int) exp2(config.virtual_address_space) / config.page_size;
    bitsOfIndex = log2(bitsOfIndex);
    
    //Find the number of bits for the offset
    int bitsOfOffset = config.virtual_address_space - bitsOfIndex;
    
    //Find the location of the virtual memory
    int virtual_page_index = virtual_address >> bitsOfOffset;
    
    //Get the valid bit
    int valid_bit = pt[pid][virtual_page_index].valid;
    
    result.virtual_page_number = virtual_page_index;
    result.page_fault = valid_bit;
    
    //Page fault
    if(valid_bit == 1)
    {
        
        //need more physical address
        //All your base belong to us
        int bits = (int)queue_remove(&q);
        pt[pid][virtual_page_index].physical_page_number = bits;
        queue_add(&q, (void*)bits);
        
        
        //Get the physical page number
        int physical_page_number = pt[pid][virtual_page_index].physical_page_number;
        result.physical_page_number = physical_page_number;
        
        //Calculate the physical address
        int physical_address = physical_page_number << bitsOfOffset;
        physical_address = physical_address + (virtual_address & (int) exp2(bitsOfOffset) - 1);
        
        
        result.physical_address = physical_address;
        
        //Set the bit as valid
        pt[pid][virtual_page_index].valid = 0;
        
        
    } else {
        queue helper;
        queue_init(&helper);
        //Get the physcial page number
        int physical_page_number = pt[pid][virtual_page_index].physical_page_number;
        result.physical_page_number = physical_page_number;
        
        // I never was able to get the second policy working, I Tried various
        // My theory though would be that since each process has their own page table
        // they would also need their own queue, sadly I ran out of time on this assignment
        
        
        //Calculate the physical address
        int physical_address = physical_page_number << bitsOfOffset;
        physical_address = physical_address + (virtual_address & (int)(exp2(bitsOfOffset) - 1));
        
        result.physical_address = physical_address;
        
    }
    
    //queue_destroy(&q);
    return result;
}

