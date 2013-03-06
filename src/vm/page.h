#ifndef VM_PAGE_H
#define VM_PAGE_H

struct sup_page
  {
    struct file* file;
    bool writable;
    off_t offset;
    size_t zero_bytes;
  }

struct sup_page* create_zero_page (void);
struct sup_page* create_full_page (struct file*, off_t offset, bool writeable);
struct sup_page* create_partial_page (struct file*, off_t offset,
                                      size_t zero_bytes, bool writeable);

#endif
