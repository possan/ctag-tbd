#include <stdlib.h>

extern void *memalign(size_t blocksize, size_t bytes);

struct mallinfo_t {
    size_t arena;
    size_t ordblks;
    size_t smblks;
    size_t hblks;
    size_t hblkhd;
    size_t usmblks;
    size_t fsmblks;
    size_t uordblks;
    size_t fordblks;
    size_t keepcost;
};

#ifdef __cplusplus
extern "C" {
#endif
    extern struct mallinfo_t mallinfo();
#ifdef __cplusplus
} 
#endif
