#include <stdint.h>
#include <stddef.h>

typedef union block_header{
    struct{
        size_t size_total;
        int is_free;
        union block_header *next;
        union block_header *prev;
    };

    max_align_t _align_;
}block_header_t;
