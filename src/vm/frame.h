#ifndef frame_H
#define frame_H

#include "threads/thread.h"
#include "threads/palloc.h"

void frame_init (void);
void *allocate_frame (enum palloc_flags flags);
void free_frame (void *);
void set_user_address (void*, uint32_t *, void *);
void pin_frame_by_page (void* kpage);
void unpin_frame_by_page (void* kpage);

#endif /* vm/frame.h */
