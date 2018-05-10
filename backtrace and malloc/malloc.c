
#include "malloc.h"
#include <stddef.h> // for NULL
#include "strings.h"

extern int __bss_end__;

// Simple macro to round up x to multiple of n.
// The efficient but tricky bitwise approach it uses
// works only if n is a power of two -- why?
#define roundup(x,n) (((x)+((n)-1))&(~((n)-1)))

#define TOTAL_HEAP_SIZE 0x1000000 // 16 MB

struct header{ //will be used for the implicit list.
    unsigned int size: 31;
    unsigned int status: 1;
};

enum { IN_USE = 0, FREE = 1}; //to be used for the status of the header of block

static void* heap_start = (char *)&__bss_end__;
static void *heap_next = NULL, *heap_max = NULL;
static int remaining_size = TOTAL_HEAP_SIZE;
void *malloc(size_t nbytes)
{
    if(!heap_max) {
        heap_next = heap_start;
        heap_max = (char *)heap_next + TOTAL_HEAP_SIZE;
        nbytes = roundup(nbytes, 8);
        int total_needed = nbytes + sizeof(struct header);
        struct header *hdr = heap_next;
        hdr->size = nbytes;
        hdr->status = IN_USE;
        remaining_size -= total_needed;
        heap_next = (char*)heap_next + total_needed;
        struct header *neighbor_hdr = heap_next;
        neighbor_hdr->size = TOTAL_HEAP_SIZE - total_needed;
        neighbor_hdr->status = FREE;
        //        printf("hdr size is: %d and the status is: %d the address is: %x \n",hdr->size, hdr->status, hdr);
        //            printf("freespace size is: %d and the status is: %d the address is: %x \n",freespace->size, freespace->status, freespace);
        return (char *)hdr + sizeof(struct header);
        
    }
    nbytes = roundup(nbytes, 8);
    
    
    struct header *next_free_hdr = heap_start;
    while((next_free_hdr->status != FREE) | (next_free_hdr->size < nbytes)){ //upgrade to search freed blocks for use
        next_free_hdr = (struct header *)((char*)next_free_hdr + next_free_hdr->size + sizeof(struct header)); //does this get you to the next header
    }
    struct header *neighbor = (struct header *)((char*)next_free_hdr + next_free_hdr->size + sizeof(struct header));
    if(neighbor->size < remaining_size){
        int old_size = next_free_hdr->size;
        next_free_hdr->size = nbytes;
        next_free_hdr->status = IN_USE;
        if(old_size > nbytes){
            struct header *new_freespace = next_free_hdr + next_free_hdr->size + sizeof(struct header);
            new_freespace->size = old_size -nbytes -sizeof(struct header);
            new_freespace->status = FREE;
        }
        return (char*)next_free_hdr + sizeof(struct header);
    }
    
    if ((char *)heap_next + nbytes > (char *)heap_max)
        return NULL;
    int total_needed = nbytes + sizeof(struct header);
    struct header *hdr = heap_next;
    heap_next = (char *)heap_next + nbytes + sizeof(struct header);
    
    hdr->size = nbytes; //the header should already exist;
    hdr->status = IN_USE;
    remaining_size -= total_needed;
    
    neighbor = heap_next;
    neighbor->size = remaining_size;
    neighbor->status = FREE;
    
    return (char *)hdr + sizeof(struct header); //read malloc specification part of spec for return val
    
    
}

void free(void *ptr)
{
    if(!ptr) return;
    struct header *hdr_to_be_freed = (void*)((char *)ptr-1);
    void *neighbor = (void*)((char*)ptr + hdr_to_be_freed->size);
    struct header *neighbor_header = neighbor;
    hdr_to_be_freed->status = FREE;
    if(neighbor_header->status == FREE) hdr_to_be_freed->size += neighbor_header->size +sizeof(struct header); //includes the block being used by neighbors's header.
    
}

void *realloc (void *old_ptr, size_t new_size)
{
    if(new_size ==0){
        free(old_ptr);
        return NULL;
    }
    new_size = roundup(new_size, 8);
    if(!old_ptr) return NULL;
    struct header *old_ptr_hdr = (void*)((char *)(old_ptr)-1);
    int old_ptr_size = old_ptr_hdr->size;
    if(new_size == old_ptr_size) return old_ptr;
    void *neighbor = (void*)((char*)old_ptr + old_ptr_hdr->size);
    struct header *neighbor_hdr = neighbor;
    int old_neighbor_size = neighbor_hdr->size;
    int additional_size_needed = old_ptr_hdr->size - new_size;
    if(old_ptr_size < new_size){
        if(neighbor_hdr->status == FREE && neighbor_hdr->size >= additional_size_needed){
            old_ptr_hdr->size = new_size;
            neighbor = (void *)(old_ptr_hdr + old_ptr_hdr->size +1);
            neighbor_hdr  = neighbor;
            neighbor_hdr->size = old_ptr_size - additional_size_needed;
            neighbor_hdr->status = FREE;
            return old_ptr;
        }
        
        void *new_ptr = malloc(new_size);
        if (!new_ptr) return NULL;
        memcpy(new_ptr, old_ptr, new_size);
        free(old_ptr);
        return new_ptr;
    }
    old_ptr_hdr->size = new_size;
    int size_diff = old_ptr_size - new_size;
    neighbor_hdr = (struct header *)((char*)neighbor_hdr - size_diff);
    neighbor_hdr->size = old_neighbor_size + size_diff;
    return old_ptr;
    
}
