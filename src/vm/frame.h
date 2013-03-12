#ifndef VM_FRAME_H
#define VM_FRAME_H

#include "threads/palloc.h"

void frame_init (void);
void * allocate_frame (enum palloc_flags flags);
void free_frame (void *page);
void frame_done (void);

#endif /* vm/frame.h */
