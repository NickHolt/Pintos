#ifndef VM_FRAME_H
#define VM_FRAME_H

#include "threads/palloc.h"

void * allocate_frame (enum palloc_flags flags);
void free_frame (void *page);

#endif /* vm/frame.h */
