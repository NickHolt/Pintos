#include "vm/page.h"

struct sup_page* create_zero_page (void)
{
    // TODO
}

struct sup_page* create_full_page (struct file *f, off_t offset, bool writeable)
{
    // TODO
}

struct sup_page* create_partial_page (struct file *F, off_t offset,
                                      size_t zero_bytes, bool writeable)
{
    // TODO
}
