#include "vm/frame.h"
#include <debug.h>
#include "threads/palloc.h"

void *
allocate_frame (enum palloc_flags flags)
{
  void *page = palloc_get_page (flags);

  if (page != NULL)
    {
      return page;
    }
  else
    {
      /* Eventually, eviction and swapping will take place here. */
      PANIC ("Out of frames.");
    }
}

void
free_frame (void *page)
{
  palloc_free_page (page);
}
