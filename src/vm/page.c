#include "vm/page.h"
#include "threads/malloc.h"
#include "lib/debug.h"
#include "threads/thread.h"
#include <stdio.h>
#include "threads/vaddr.h"

struct sup_page*
create_sup_page (struct file *f, off_t offset, size_t zero_bytes,
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
  partial_page->is_swapped = false;
  partial_page->loaded = false;

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

void
delete_sup_page (struct sup_page *page)
{
  hash_delete (&thread_current ()->supp_pt, &page->pt_elem);
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

void
print_sp (struct sup_page *page)
{
  printf("Page at %p:\n", page);
  printf("File: %p\n", page->file);
  printf("zero_bytes: %i\n", page->zero_bytes);
  printf("read_bytes: %i\n", page->read_bytes);
  printf("addr: %p\n", page->user_addr);
  printf("swapped: %i\n", page->is_swapped);

  ASSERT(page->read_bytes <= PGSIZE);
  ASSERT(page->zero_bytes <= PGSIZE);
}
