#ifndef VM_FRAME_H
#define VM_FRAME_H

#include "threads/palloc.h"
#include <stdint.h>
#include "threads/thread.h"

void frame_init (void);
void frame_done (void);
void *allocate_frame (enum palloc_flags flags);
void free_frame (void *page);
void frame_done (void);
void set_page_table_entry (void* page, uint8_t *user_addr, uint8_t *pt_entry);

#endif /* vm/frame.h */
