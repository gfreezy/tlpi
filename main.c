#include "tlpi_hdr.h"
#include <math.h>
#include <assert.h>

#define MAX_MALLOCS 1000000
#define INCR_BLOCK_SIZE 0x100

#define block_size(ptr) ((size_t)*ptr)
#define set_block_size(ptr, size) *(ptr) = (void *)(size)
#define block_prev(ptr) ((void **)*(ptr+1))
#define set_block_prev(ptr, p) *((ptr)+1) = (void **)(p)
#define block_next(ptr) ((void **)*(ptr+2))
#define set_block_next(ptr, p) *((ptr)+2) = (void **)(p)
#define block_data_start(ptr) ((void **)(ptr+1))
#define block_data_end(ptr) ((void **)(ptr + block_size(ptr) + 1))

void **find_first_match(size_t);
void *malloc_(size_t);
void free_(void*);
void *split_block(void** p, size_t size);

void **freelist = NULL;

void* malloc_(size_t size) {
    printf("current programming break is %10p\n", sbrk(0));
    void **block = find_first_match(size);
    assert(block != NULL);
    if (block == NULL) {
        fatal("Can not find first match block\n");
    }
    return split_block(block, size);
}


void free_(void *ptr2) {
    void **p;
    void **ptr = (void **)ptr2;
    for (p = freelist; p != NULL; p = block_next(p)) {
        if ( ptr == block_data_end(p)) {
            assert(block_data_end(p) < block_next(p));
            set_block_size(p, block_size(p) + block_size(ptr));
        } else if (ptr > block_data_end(p) && block_data_end(ptr) < block_next(p)) {
            void **next_block = block_next(p);
            set_block_next(ptr, next_block);
            set_block_prev(ptr, p);
            
            set_block_next(p, ptr);
            set_block_prev(next_block, ptr);
        } else if (ptr > block_data_end(p) && block_data_end(ptr) == block_next(p)) {
            void **next_block = block_next(p);
            set_block_next(ptr, block_next(next_block));
            set_block_prev(ptr, p);
            set_block_next(p, ptr);
            set_block_size(ptr, block_size(ptr) + block_size(next_block) + 1);
        }
    }
}


void** find_first_match(size_t size) {
    void **p;
    for (p = freelist; p != NULL; p = block_next(p)) {
        if (block_size(p) >= size) {
            return p;
        }
    }
    
    void **last_block = p;
    int n_blocks = (int)(ceil(((double)size + 1) / INCR_BLOCK_SIZE));
    size_t allocked_size = INCR_BLOCK_SIZE * n_blocks;
    void *last_program_break = sbrk(allocked_size);
    if (last_program_break == (void *)-1) {
        errExit("sbrk");
    }
    
    if (last_block == NULL) {
        void **new_block = (void **)last_program_break;
        set_block_size(new_block, allocked_size - 1);
        set_block_next(new_block, NULL);
        set_block_prev(new_block, last_block);
        return new_block;
    }
    
    if ((void *)block_data_end(last_block) == last_program_break) {
        set_block_size(last_block, block_size(last_block) + allocked_size - 1);
        return last_block;
    } else {
        void **new_block = (void **)last_program_break;
        set_block_size(new_block, allocked_size - 1);
        set_block_next(new_block, NULL);
        set_block_prev(new_block, last_block);
        set_block_next(last_block, new_block);
        return new_block;
    }
}


void* split_block(void **p, size_t size) {
    void **prev_block = block_prev(p);
    void **next_block = block_next(p);
    
    if (block_size(p) == size) {
        set_block_next(prev_block, next_block);
        set_block_prev(next_block, prev_block);
        
        return (void *)p;
    }

    size_t orig_size = block_size(p);
    set_block_size(p, size);
    
    void **left_block = p + size + 1;
    set_block_size(left_block, orig_size - size);
    if (prev_block) {
        set_block_next(prev_block, left_block);
    }
    if (next_block) {
        set_block_prev(next_block, left_block);
    }
    
    return (void *)p;
}

int
main(int argc, char *argv[]) {
    char *ptr[MAX_MALLOCS];
    int freeStep, freeMin, freeMax, blockSize, numAllocs;
    
    printf("\n");
    if (argc < 3 || strcmp(argv[1], "--help") == 0) {
        usageErr("%s num-allocs block-size [step [min [max]]]\n", argv[0]);
    }
    
    numAllocs = getInt(argv[1], GN_ANY_BASE | GN_GT_0, "num-allocs");
    if (numAllocs > MAX_MALLOCS) {
        cmdLineErr("num-allocs > %z\n", MAX_MALLOCS);
    }
    
    blockSize = getInt(argv[2], GN_ANY_BASE | GN_GT_0, "block-size");
    freeStep = argc > 3 ? getInt(argv[3], GN_ANY_BASE | GN_GT_0, "step") : 1;
    freeMin = argc > 4 ? getInt(argv[4], GN_GT_0, "min") : 1;
    freeMax = argc > 5 ? getInt(argv[5], GN_GT_0, "max") : numAllocs;
    
    if (freeMax > numAllocs) {
        cmdLineErr("free-max > num-allocs\n");
    }
    
    printf("Initial program break:             %10p\n", sbrk(0));
    printf("Allocating %d*%d bytes\n", numAllocs, blockSize);
    
    int i;
    for (i = 0; i < numAllocs; i++) {
        ptr[i] = malloc_(blockSize);
        if (ptr[i] == NULL) {
            errExit("malloc");
        }
    }
    
    printf("Programming break is now:       %10p\n", sbrk(0));
    
    printf("Free blocks from %d to %d in steps of %d\n",
            freeMin, freeMax, freeStep);

    for (i = freeMin -1; i < freeMax; i+=freeStep) {
        free_(ptr[i]);
    }
    
    printf("After free, programming break is %10p\n", sbrk(0));
    
    exit(EXIT_SUCCESS);
}