#include "vm/page.h"
#include "threads/malloc.h"
#include "lib/debug.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include <stdio.h>

struct sup_page*
create_zero_page (uint8_t *addr)
{
    struct sup_page *zero_page = malloc (sizeof (struct sup_page));
    if (zero_page == NULL)
        PANIC ("Failed to allocate memory in create_zero_page()");

    zero_page->zero_bytes = PGSIZE;
    zero_page->read_bytes = 0;
    zero_page->user_addr = addr;

    return zero_page;
}

struct sup_page*
create_full_page (struct file *f, off_t offset, bool writable, uint8_t *addr)
{
    struct sup_page *full_page = malloc (sizeof (struct sup_page));
    if (full_page == NULL)
        PANIC ("Failed to allocate memory in create_full_page()");

    full_page->file = f;
    full_page->writable = writable;
    full_page->offset = offset;
    full_page->zero_bytes = 0;
    full_page->read_bytes = PGSIZE;
    full_page->user_addr = addr;

    return full_page;
}

struct sup_page*
create_partial_page (struct file *f, off_t offset, size_t zero_bytes,
                     bool writable, uint8_t *addr, size_t read_bytes)
{
    struct sup_page *partial_page = malloc (sizeof (struct sup_page));
    if (partial_page == NULL)
        PANIC ("Failed to allocate memory in create_full_page()");

    partial_page->file = f;
    partial_page->writable = writable;
    partial_page->offset = offset;
    partial_page->zero_bytes = zero_bytes;
    partial_page->read_bytes = read_bytes;
    partial_page->user_addr = addr;

    return partial_page;
}

bool
add_sup_page (struct sup_page *page)
{
    return hash_insert (&thread_current ()->supp_pt, &page->pt_elem) == NULL;
}

struct sup_page*
get_sup_page (struct hash *pt, uint8_t *addr)
{
    struct sup_page temp;
    temp.user_addr = addr;

    struct hash_elem *temp_elem
        = hash_find (pt, &temp.pt_elem);

    if (temp_elem == NULL)
        return NULL;
    else
        return hash_entry (temp_elem, struct sup_page, pt_elem);
}

static void
free_sup_pages (struct hash_elem *page_elem, void *aux UNUSED)
{
    struct sup_page *page = hash_entry (page_elem, struct sup_page, pt_elem);
    free (page);
}

void
reclaim_pages (struct hash *pt)
{
    hash_destroy (pt, free_sup_pages);
}
