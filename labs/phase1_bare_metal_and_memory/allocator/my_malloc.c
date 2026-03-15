#include "cmsis_compiler.h"
#include "my_malloc_internal.h"
#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>

#define ALIGNMENT (alignof(max_align_t))

extern char _sheap;
extern char _sstack;

static void *current_break = NULL;

const size_t meta_data_size = sizeof(block_header_t);

static block_header_t *head = NULL;
static block_header_t *tail = NULL;

static inline size_t align(size_t size);
static void *_sbrk_(size_t size);
static block_header_t *request_space(size_t total_size);
static block_header_t *find_free_block(size_t total_size);

// aligns the sizes to prevent memory corruption
static inline size_t align(size_t size){
    return (size + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);
}

static void *_sbrk_(size_t size){
    if (current_break == NULL)
        current_break = (void *)&_sheap;

    char *prev_break = (char*)current_break;
    char *next_break = (char *)prev_break + size;  
    
    if (next_break > (char *)&_sstack)
        return (void*)-1;

    current_break = (void *)next_break;

    return (void *)prev_break;
}

// asks for additional memory from mcpu when needed
static block_header_t *request_space(size_t total_size){
    block_header_t *block = (block_header_t *)_sbrk_(total_size);

    if (block == (void *)-1)
        return NULL;

    return block;
}

static block_header_t *find_free_block(size_t total_size){
    block_header_t *current = head;
    while (current != NULL){
        if (current->is_free && current->size_total >= total_size)
            return current;

        current = current->next;
    }

    return NULL;
}

void *my_malloc(size_t size){
    __disable_irq();

    if (current_break == NULL)
        current_break = (void *)&_sheap;
    
    void *result_p = NULL;

    size_t heap_size = (size_t)((char *)&_sstack - (char *)current_break);

    if (size > (heap_size - ALIGNMENT))
        goto exit_malloc;

    size_t aligned_size = align(size);
    if (aligned_size > (heap_size - sizeof(block_header_t)))
        goto exit_malloc;


    size_t total_size = aligned_size + sizeof(block_header_t);


    if (head == NULL){
        block_header_t *new_block = request_space(total_size);
        
        if (new_block == NULL)
            goto exit_malloc;

        new_block->size_total = total_size;
        new_block->is_free = 0;
        new_block->prev = head;
        new_block->next = tail;
        head = new_block;
        tail = new_block;
        
        result_p = new_block + 1;
        goto exit_malloc;
    }
    
    block_header_t *free_block = find_free_block(total_size);

    if (free_block != NULL){
        size_t new_size = free_block->size_total - total_size;
        
        if (new_size < (sizeof(block_header_t) + ALIGNMENT)){
            free_block->is_free = 0;
            result_p = free_block + 1;
            goto exit_malloc;
        }

        // splitting 
        block_header_t *remainder = (block_header_t *)((char *)free_block + total_size);
        remainder->size_total = new_size;
        remainder->is_free = 1;
        remainder->next = free_block->next;
        remainder->prev = free_block;
        
        free_block->size_total = total_size;
        free_block->is_free = 0;
        free_block->next = remainder;
        
        if (remainder->next != NULL)
            remainder->next->prev = remainder;
        else
            tail = remainder;


        result_p = free_block + 1;
        goto exit_malloc;
    }


    block_header_t *new_free_block = request_space(total_size);

    if (new_free_block == NULL)
        goto exit_malloc;

    new_free_block->size_total = total_size;
    new_free_block->is_free = 0;
    new_free_block->prev = tail;
    new_free_block->next = NULL;
    tail->next = new_free_block;

    tail = new_free_block;

    result_p = new_free_block + 1;
    goto exit_malloc;

exit_malloc:
    __enable_irq();
    return result_p;

}

void my_free(void *ptr){
    __disable_irq();

    if (!ptr)
        goto exit_free;

    block_header_t *current_ptr = (block_header_t *)ptr - 1;
    
    // double free check
    if (current_ptr->is_free)
        goto exit_free;

    current_ptr->is_free = 1;
    
    if (current_ptr->prev != NULL && current_ptr->prev->is_free == 1){
        if (tail == current_ptr)
            tail = current_ptr->prev;
        
        current_ptr->prev->size_total += current_ptr->size_total;
        current_ptr->prev->next = current_ptr->next;
        
        if (current_ptr->next != NULL)
            current_ptr->next->prev = current_ptr->prev;

        current_ptr = current_ptr->prev;

    }

    if (current_ptr->next != NULL && current_ptr->next->is_free == 1){
        if (tail == current_ptr->next)
            tail = current_ptr;

        current_ptr->size_total += current_ptr->next->size_total;

        current_ptr->next = current_ptr->next->next;

        if (current_ptr->next != NULL)
            current_ptr->next->prev = current_ptr;
    }

    if (current_ptr->prev == NULL)
        head = current_ptr;

exit_free:
    __enable_irq();
    return;
}

void *my_calloc(size_t nmemb, size_t size){
    if (size != 0 && nmemb > SIZE_MAX / size)
        return NULL;

    unsigned char *mem = my_malloc(nmemb * size);
    if (!mem)
        return NULL;

    for (size_t i = 0; i < nmemb * size; i++)
        *(mem + i)= 0;

    
    return (void *)mem;
}

void *my_realloc(void *ptr, size_t size){
    if (ptr == NULL)
        return my_malloc(size);

    if (size == 0){
        my_free(ptr);
        return NULL;
    }

    block_header_t *current = (block_header_t *)ptr - 1;

    size_t size_data = current->size_total - sizeof(block_header_t);

    if (size < size_data)
        size_data = size;

    void *mem = my_malloc(size);
    if (!mem)
        return NULL;

    unsigned char *src = (unsigned char *)ptr;
    unsigned char *dest = (unsigned char *)mem;
    for (size_t i = 0; i < size_data; i++)
        *dest++ = *src++;

    my_free(ptr);

    return mem;
}

void wipe_heap(void){
    __disable_irq();

    if (current_break != NULL){
        char *heap_start = (char *)&_sheap;

        while (heap_start < (char *)current_break)
            *heap_start++ = 0;
    }

    head = NULL;
    tail = NULL;
    
    current_break = (void *)&_sheap;

    __enable_irq();
}
