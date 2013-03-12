#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
#include "filesys/off_t.h"

/* Struct that holds information about a page in the supplementary page table.
   This gets inserted into a thread's struct hash supp_pt member */
struct sup_page
  {
    struct file *file;
    bool writable;
    off_t offset;
    size_t zero_bytes;
    size_t read_bytes;
    uint8_t *user_addr;

    struct hash_elem pt_elem;
  };

struct sup_page* create_zero_page (uint8_t *addr);
struct sup_page* create_full_page (struct file*, off_t offset, bool writable,
                                   uint8_t *addr);
struct sup_page* create_partial_page (struct file*, off_t offset,
                                      size_t zero_bytes, bool writable,
                                      uint8_t *addr, size_t read_bytes);
bool add_sup_page (struct sup_page *page);
struct sup_page* get_sup_page (struct hash *pt, uint8_t *addr);
void reclaim_pages (struct hash *pt);

#endif
