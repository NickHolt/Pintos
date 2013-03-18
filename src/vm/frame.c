#include "vm/frame.h"
#include <debug.h>
#include <hash.h>
#include "threads/malloc.h"
#include "threads/synch.h"
#include "vm/swap.h"
#include "userprog/pagedir.h"
#include "vm/page.h"
#include <string.h>
#include "threads/vaddr.h"

struct frame {
  struct hash_elem elem; /* Hash table element. */
  void *page;            /* Page occupying this frame. */
  struct thread* thread; /* Owner of this frame. */
  uint8_t *user_addr;    /* Stored to associate frames and sup_pt entries */
  bool pinned;           /* Is the frame pinned? */
  bool from_file;        /* Is the frame holding file data? */
};

static struct lock frame_lock;
static struct hash frame_table;
static struct frame* evict_frame (void);
static struct frame* select_frame_to_evict (void);
static void pin_frame (struct frame* f);
static void unpin_frame (struct frame *f);

static unsigned
frame_hash (const struct hash_elem *f_, void *aux UNUSED)
{
  struct frame *f = hash_entry (f_, struct frame, elem);
  return hash_bytes (&f->page, sizeof f->page);
}

static bool
frame_less (const struct hash_elem *a_, const struct hash_elem *b_,
            void *aux UNUSED)
{
  struct frame *a = hash_entry (a_, struct frame, elem);
  struct frame *b = hash_entry (b_, struct frame, elem);

  ASSERT (a != NULL);
  ASSERT (b != NULL);

  return a->page < b->page;
}

static void
frame_destroy (struct hash_elem *f_, void *aux UNUSED)
{
  struct frame *f = hash_entry (f_, struct frame, elem);
  free (f);
}

void
frame_init (void)
{
  lock_init (&frame_lock);
  hash_init (&frame_table, frame_hash, frame_less, NULL);
}

void
frame_done (void)
{
  hash_destroy (&frame_table, frame_destroy);
}

void *
allocate_frame (enum palloc_flags flags)
{
  lock_acquire (&frame_lock);
  void *page = palloc_get_page (flags);
  lock_release (&frame_lock);
  struct frame *f;

  if (page != NULL)
    {
      f = malloc (sizeof (struct frame));
      if (f == NULL)
        PANIC ("Failed to allocate memory for frame.");

      f->page = page;
      f->thread = thread_current ();

      lock_acquire (&frame_lock);
      hash_insert (&frame_table, &f->elem);
      lock_release (&frame_lock);
      return page;
    }
  else
    {
      f = evict_frame ();
      ASSERT (f != NULL);
      f->thread = thread_current ();
      f->user_addr = NULL;
      return f->page;
    }
}

/* Set the user page as USER_ADDR on the frame with page PAGE. */
void
set_user_address (void *page, uint8_t *user_addr)
{
  struct frame temp;
  struct hash_elem *e;

  temp.page = page;
  e = hash_find (&frame_table, &temp.elem);
  struct frame *res = hash_entry (e, struct frame, elem);

  res->user_addr = user_addr;
}

static void pin_frame (struct frame* f)
{
  f->pinned = true;
}

static void unpin_frame (struct frame *f)
{
  f->pinned = false;
}

void pin_by_addr (void *page)
{
  struct frame temp;
  struct hash_elem *e;

  temp.page = page;
  e = hash_find (&frame_table, &temp.elem);
  struct frame *res = hash_entry (e, struct frame, elem);
  res->pinned = true;
}

void unpin_by_addr (void *page)
{
  struct frame temp;
  struct hash_elem *e;

  temp.page = page;
  e = hash_find (&frame_table, &temp.elem);
  struct frame *res = hash_entry (e, struct frame, elem);
  res->pinned = false;
}

static struct frame*
evict_frame (void)
{
  /* Choose a frame to evict and clear it from the page directory of its owner
     thread */
  struct frame *choice = select_frame_to_evict ();
  struct thread *owner = choice->thread;

  if (owner == NULL || owner->pagedir == NULL)
    {
      free_frame (choice->page);
      return evict_frame ();
    }

  /* Get the supplemental page table entry associated with the chosen frame */
  struct sup_page *sp = get_sup_page (&owner->supp_pt, choice->user_addr);

  if (sp == NULL)
    {
      sp = malloc (sizeof (struct sup_page));
      if (sp == NULL)
        PANIC ("Failed to allocate memory in create_full_page()");

      sp->user_addr = choice->user_addr;
    }

  if (pagedir_is_dirty (owner->pagedir, choice->user_addr))
    {
      pin_frame (choice);
      size_t index = pick_slot_and_swap (choice->page);
      sp->is_swapped = true;
      sp->swap_index = index;
      unpin_frame (choice);
    }

  memset (choice->page, 0, PGSIZE);

  lock_acquire (&owner->pd_lock);
  pagedir_clear_page (owner->pagedir, choice->user_addr);
  lock_release (&owner->pd_lock);

  sp->loaded = false;
  return choice;

}

static struct frame*
select_frame_to_evict (void)
{
  printf("%i\n", hash_size (&frame_table));
  struct hash_iterator i;
  struct frame *choice = NULL;

  /* Second chance page replacement algorithm. We always ignore pinned
     frames */
  int j = 0;
  for (; j < 2; ++j)
    {
      /* The first loop finds any perfect candidates for eviction - one that
         is neither accessed nor dirty. */
      hash_first (&i, &frame_table);
      while (hash_next (&i))
        {
          choice = hash_entry (hash_cur (&i), struct frame, elem);
          if (choice->pinned)
            continue;

          struct thread* owner = choice->thread;
          lock_acquire (&owner->pd_lock);
          if (!pagedir_is_dirty (owner->pagedir, choice->user_addr) &&
              !pagedir_is_accessed (owner->pagedir, choice->user_addr))
            {
              /* A perfect frame to evict */
              lock_release (&owner->pd_lock);
              printf("picked %p perfect\n", choice);
              return choice;
            }

          lock_release (&owner->pd_lock);
        }

      /* The second loop looks for slightly worse candidates - ones that are
         not accessed but dirty. If they do not match this criteria, they are
         given a second chance - their accessed bit is set, and we execute the
         for loop again. */
      hash_first (&i, &frame_table);
      while (hash_next (&i))
        {
          choice = hash_entry (hash_cur (&i), struct frame, elem);
          if (choice->pinned)
            continue;

          struct thread* owner = choice->thread;
          lock_acquire (&owner->pd_lock);
          if (pagedir_is_dirty (owner->pagedir, choice->user_addr) &&
              !pagedir_is_accessed (owner->pagedir, choice->user_addr))
            {
              lock_release (&owner->pd_lock);
              printf("picked %p imperfect\n", choice);
              return choice;
            }

          /* Frames that reach here will be hit earlier the next time round,
             in the first while loop or the second depending on whether they
             are dirty or not */
          pagedir_set_accessed (owner->pagedir, choice->user_addr, false);
          lock_release (&owner->pd_lock);
        }
    }

  /* This will only get hit if every frame in existence is pinned */
  return NULL;
}

void
free_frame (void *page)
{
  palloc_free_page (page);

  struct frame f;
  f.page = page;
  hash_find (&frame_table, &f.elem);

  lock_acquire (&frame_lock);
  struct hash_elem *e = hash_delete (&frame_table, &f.elem);
  lock_release (&frame_lock);
  frame_destroy (e, NULL);
}
