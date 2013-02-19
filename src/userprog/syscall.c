#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"
#include "userprog/process.h"

static void syscall_handler (struct intr_frame *);
static void halt (void);
static void exit (int status);
static int  write (int fd, const void *buffer, unsigned size);
static int  wait (pid_t pid);

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

/* Switch on the system call numbers defined in lib/syscall-nr.h, and call the
   appropriate system call. If the system call returns something, then put that
   value in f->eax. */
static void
syscall_handler (struct intr_frame *f)
{
  ASSERT (f != NULL);
  ASSERT (f->esp != NULL);

  int *stack_pointer = f->esp;
  int syscall_number = *stack_pointer;

  /* Maybe function pointers would be neater here? */
  switch (syscall_number)
    {
      case SYS_HALT:
        halt ();
        break;

      case SYS_EXIT:
        exit (*(stack_pointer + 1));
        break;

      case SYS_WRITE:
        f->eax = write (*(stack_pointer + 1), (void *) *(stack_pointer + 2),
                        *(stack_pointer + 3));
        break;

      case SYS_WAIT:
        f->eax = wait (*(stack_pointer + 1));
        break;
    }
}

/* Terminates Pintos. */
static void
halt (void)
{
  shutdown_power_off ();
}

/* Terminates the current user program, sending its exit status to the kernel.
   If the process's parent waits for it, this is the status that will be
   returned. */
static void
exit (int status)
{
  struct thread *exiting_thread = thread_current();
  printf("%s: exit(%d)\n", exiting_thread->name, status);

  /* I think there might need to be stuff to do with parents here, which will
     involve adding a list of children to the thread struct probably. */

  /* Charlie: what about stuff that exiting_thread currently holds?
              Locks? Files? */

  thread_exit();
}

/* Writes size bytes from buffer to the open file fd. Returns the number of
   bytes actually written, which may be less than size.
   fd == 1 writes to the console. */
static int
write (int fd, const void *buffer, unsigned size)
{
  /* TODO: check if pointer to buffer is valid */
  ASSERT (buffer != NULL); /* Is this enough? */

  if (fd == 1)
    {
      /* TODO: if buffer is greater than `a few hundred bytes', break it up into
               multiple putbuf() calls. */
      putbuf (buffer, size);
      return size;
    }
  else
    {
      /* Write to the file. Need to lock the file system, open the file, write
         to the file, and release the lock. We can probably use file_open and
         file_write from filesys/file.h for this. */

      return 0; /* Needs to return number of bytes written to file. */
    }
}

/* Waits for a child process pid and retreives the child's exit status. */
static int
wait (pid_t pid)
{
  return process_wait (pid);
}
