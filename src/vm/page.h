#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
#include "filesys/off_t.h"

/* Enumerate the optinos for where the data could be stored. We start at 1 and
   not 0 to make bitwise operations on the types easier */
enum sup_page_type
  {
    SWAP = 1,
    FILE = 2,
    FILEINSWAP = 3
  };

/* Struct that holds information about a page in the supplementary page table.
   This gets inserted into a thread's struct hash supp_pt member */
struct sup_page
  {
    void *user_addr;
    enum sup_page_type type;
    bool is_loaded;
    struct file * file;
    off_t offset;
    uint32_t read_bytes;
    uint32_t zero_bytes;
    bool writable;

    size_t swap_index;
    bool swap_writable;

    struct hash_elem pt_elem;
  };

struct sup_page* create_sup_page (struct file*, off_t offset,
                                  size_t zero_bytes, bool writable,
                                  uint8_t *addr, size_t read_bytes);
bool add_sup_page (struct hash *pt, struct sup_page *page);
struct sup_page* get_sup_page (struct hash *pt, uint8_t *addr);
void delete_sup_page (struct sup_page *page);
void reclaim_pages (struct hash *pt);
void print_sp (struct sup_page *page);

#endif /* vm/page.h */
