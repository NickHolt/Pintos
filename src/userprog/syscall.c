#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "devices/shutdown.h"

static void syscall_handler (struct intr_frame *);
static void halt (void);
static void exit (int status);
static int wait (pid_t pid);
static int write (int fd, const void *buffer, unsigned size);

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

/* Checks that a pointer points to a valid user memory address, and is therefore
   safe to be dereferenced. If it's not safe, we terminate the process.
   Returns true iff the ptr can safely be dereferenced. */
static bool
is_safe_user_ptr (void *ptr)
{
  struct thread *t = thread_current();
  if (ptr == NULL || !is_user_vaddr (ptr) ||
      pagedir_get_page (t->pagedir, ptr) == NULL)
    {
      /* Destroy thread. */
      exit (-1);
    	return false;
    }
  else
    {
      return true;
    }
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

      case SYS_WAIT:
        f->eax = wait (*(stack_pointer + 1));
        break;

      case SYS_WRITE:
        f->eax = write (*(stack_pointer + 1), (void *) *(stack_pointer + 2),
                        *(stack_pointer + 3));
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

/* Runs the executable whose name is given in cmd_line, passing any given
   arguments, and returns the new process's pid. */
static pid_t
exec (const char *file UNUSED)
{
  return -1;
}

/* Waits for a child process pid and retrieves the child's exit status. */
static int
wait (pid_t pid UNUSED)
{
  return process_wait (pid);
}

/* Creates a new file called file initially initial_size bytes in size. Returns
   true iff successful. */
static bool
create (const char *file UNUSED, unsigned initial_size UNUSED)
{
  return false;
}

/* Deletes the file called file. Returns true iff successful. */
static bool
remove (const char *file UNUSED)
{
  return false;
}

/* Opens the file called file. Returns a non-negative integer handle, or -1 if
   the file could not be opened. */
static int
open (const char *file UNUSED)
{
	return -1;
}

/* Returns the size, in bytes, of the file open as fd. */
static int
filesize (int fd UNUSED)
{
	return -1;
}

/* Reads size bytes from the file open as fd into buffer. Returns the number of
   bytes actually read, or -1 if the file could not be read.
   fd == 0 reads from the keyboard using input_getc(). */
static int
read (int fd UNUSED, void *buffer UNUSED, unsigned length UNUSED)
{
	return -1;
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

/* Changes the next byte to be read/written in open file fd to position,
   expressed in bytes from the beginning of the file. */
static void
seek (int fd UNUSED, unsigned position UNUSED)
{

}

/* Returns the position of the next byte to be read or written in open file fd,
   expressed in bytes from the beginning of the file. */
static unsigned
tell (int fd UNUSED)
{
	return 0;
}

/* Closes file descriptor fd. Exiting or terminating a process implicitly closes
   all its open file descriptors, as if by calling this function for each
   one. */
static void
close (int fd UNUSED)
{

}
