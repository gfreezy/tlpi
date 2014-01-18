#include "tlpi_hdr.h"

#define MAX_MALLOCS 1000000

void 
*malloc_(size_t size) {
    printf("current programming break is %10p\n", sbrk(0));
    return malloc(size);
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
        free(ptr[i]);
    }
    
    printf("After free, programming break is %10p\n", sbrk(0));
    
    exit(EXIT_SUCCESS);
}