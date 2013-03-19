#ifndef frame_H
#define frame_H

#include "threads/thread.h"
#include "threads/palloc.h"

void frame_init (void);
void *allocate_frame (enum palloc_flags flags);
void free_frame (void *);
void set_user_address (void*, uint32_t *, void *);

#endif /* vm/frame.h */
