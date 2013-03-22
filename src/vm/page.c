#include "vm/page.h"
#include "threads/pte.h"
#include "vm/frame.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "threads/malloc.h"
#include "filesys/file.h"
#include "string.h"
#include "userprog/syscall.h"
#include "vm/swap.h"

static void free_sup_pages (struct hash_elem *, void *aux);

/* Create a struct sup_page and fill in its properties */
struct sup_page*
create_sup_page (struct file *f, off_t offset, size_t zero_bytes,
                 bool writable, uint8_t *addr, size_t read_bytes)
{
  struct sup_page *page = malloc (sizeof (struct sup_page));
  if (page == NULL)
    PANIC ("Failed to allocate memory in create_full_page()");

  page->file = f;
  page->type = FILE;
  page->writable = writable;
  page->offset = offset;
  page->zero_bytes = zero_bytes;
  page->read_bytes = read_bytes;
  page->user_addr = addr;
  page->is_loaded = false;

  return page;
}

/* Returns null if PAGE is successfully added to PT */
bool
add_sup_page (struct hash *pt, struct sup_page *page)
{
  return hash_insert (pt, &page->pt_elem) == NULL;
}

/* Find the sup_page in PT which has the address ADDR. Return a null pointer
   if the sup_page is not found */
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

/* Remove the supplemental page table entry from the current thread's
   supplemental page table */
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

/* Clean up the hash table */
void
reclaim_pages (struct hash *pt)
{
  hash_destroy (pt, free_sup_pages);
}
