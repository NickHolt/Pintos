#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "devices/shutdown.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include <hash.h>

#define MAX_PUTBUF 512

static void syscall_handler (struct intr_frame *f);
static void halt (void);
static pid_t exec (const char *file);
static int wait (pid_t pid);
static bool create (const char *file, unsigned initial_size);
static bool remove (const char *file);
static int open (const char *file);
static int filesize (int fd);
static int read (int fd, void *buffer, unsigned length);
static int write (int fd, const void *buffer, unsigned size);
static void seek (int fd, unsigned position);
static unsigned tell (int fd);
static void close (int fd);

static struct hash fd_hash;
static int next_fd = 2;


struct fd_node
  {
    struct hash_elem hash_elem;
    unsigned fd;
    struct thread *thread;
    struct file *file;
  };

static unsigned
hash_func (const struct hash_elem *node_, void *aux UNUSED)
{
  struct fd_node *node = hash_entry (node_, struct fd_node, hash_elem);

  ASSERT (node != NULL);

  return node->fd;
}

static bool
less_func (const struct hash_elem *a_, const struct hash_elem *b_,
                void *aux UNUSED)
{
  struct fd_node *a = hash_entry (a_, struct fd_node, hash_elem);
  struct fd_node *b = hash_entry (b_, struct fd_node, hash_elem);

  ASSERT (a != NULL);
  ASSERT (b != NULL);

  return a->fd < b->fd;
}

/* Returns a file * for a given int fd. Terminates the process with an error
   code if the fd is not mapped, or is stdin/stdout. */
static struct file *
fd_to_file (int fd)
{
  struct fd_node node;
  node.fd = fd;

  struct hash_elem *e = hash_find (&fd_hash, &node.hash_elem);

  /* fd isn't mapped. Terminate.
     stdin/stdout failure cases are also caught here. */
  if (e == NULL)
    exit (-1);

  struct fd_node *entry = hash_entry (e, struct fd_node, hash_elem);
  return entry->file;
}

void
syscall_init (void)
{
  if (!hash_init (&fd_hash, hash_func, less_func, NULL))
    PANIC ("Failed to allocate memory for file descriptor map");

  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

/* Checks that a pointer points to a valid user memory address, and is therefore
   safe to be dereferenced. If it's not safe, we terminate the process.
   Returns true iff the ptr can safely be dereferenced. */
static bool
is_safe_user_ptr (const void *ptr)
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

  if (is_safe_user_ptr (stack_pointer))
    {
      int syscall_number = *stack_pointer;

      /* Maybe function pointers would be neater here? */
      switch (syscall_number)
        {
          case SYS_HALT:
            halt ();
            break;

          case SYS_EXIT:
            if (is_safe_user_ptr (stack_pointer + 1))
              exit (*(stack_pointer + 1));
            break;

          case SYS_EXEC:
            if (is_safe_user_ptr (stack_pointer + 1))
              f->eax = exec ((char *) *(stack_pointer + 1));
            break;

          case SYS_WAIT:
            if (is_safe_user_ptr (stack_pointer + 1))
              f->eax = wait (*(stack_pointer + 1));
            break;

          case SYS_CREATE:
            if (is_safe_user_ptr (stack_pointer + 1) &&
                is_safe_user_ptr (stack_pointer + 2))
              f->eax = create ((char *) *(stack_pointer + 1),
                               *(stack_pointer + 2));
            break;

          case SYS_REMOVE:
            if (is_safe_user_ptr (stack_pointer + 1))
              f->eax = remove ((char *) *(stack_pointer + 1));
            break;

          case SYS_OPEN:
            if (is_safe_user_ptr (stack_pointer + 1))
              f->eax = open ((char *) *(stack_pointer + 1));
            break;

          case SYS_FILESIZE:
            if (is_safe_user_ptr (stack_pointer + 1))
              f->eax = filesize (*(stack_pointer + 1));
            break;

          case SYS_READ:
            if (is_safe_user_ptr (stack_pointer + 1) &&
                is_safe_user_ptr (stack_pointer + 2) &&
                is_safe_user_ptr (stack_pointer + 3))
              f->eax = read (*(stack_pointer + 1),
                             (void *) *(stack_pointer + 2),
                             *(stack_pointer + 3));
            break;

          case SYS_WRITE:
            if (is_safe_user_ptr (stack_pointer + 1) &&
                is_safe_user_ptr (stack_pointer + 2) &&
                is_safe_user_ptr (stack_pointer + 3))
              f->eax = write (*(stack_pointer + 1),
                              (void *) *(stack_pointer + 2),
                              *(stack_pointer + 3));
            break;

          case SYS_SEEK:
            if (is_safe_user_ptr (stack_pointer + 1) &&
                is_safe_user_ptr (stack_pointer + 2))
              seek (*(stack_pointer + 1), *(stack_pointer + 2));
            break;

          case SYS_TELL:
            if (is_safe_user_ptr (stack_pointer + 1))
              f->eax = tell (*(stack_pointer + 1));
            break;

          case SYS_CLOSE:
            if (is_safe_user_ptr (stack_pointer + 1))
              close (*(stack_pointer + 1));
            break;

          default:
            NOT_REACHED ();
        }
    }
}

/* Terminates Pintos. */
static void
halt (void)
{
  hash_destroy (&fd_hash, NULL);
  shutdown_power_off ();
}

/* Terminates the current user program, sending its exit status to the kernel.
   If the process's parent waits for it, this is the status that will be
   returned. */
void
exit (int status)
{
  struct thread *exiting_thread = thread_current();

  /* Print the terminating message */
  printf("%s: exit(%d)\n", exiting_thread->name, status);

  /* Set some information about the child, for process_wait */
  struct thread *parent = exiting_thread->parent;
  struct child_info *info = get_child (parent, exiting_thread->tid);

  if (info != NULL)
    {
      lock_acquire (&parent->cond_lock);
      info->has_exited = true;
      info->return_status = status;
      lock_release (&parent->cond_lock);
    }
  else
    {
      /* Probably need some sort of error thingy here? */
    }

  /* Charlie: what about stuff that exiting_thread currently holds?
              Locks? Files? */

  /* TODO: call close() on each of thread's fds. Presumably either iterate
           through the hash map, or redesign it so that the fd->file map is
           inside the thread. Or let a thread have a list of fds, then use the
           hash map still. */

  thread_exit();
}

/* Runs the executable whose name is given in cmd_line, passing any given
   arguments, and returns the new process's pid, or -1 if loading the new
   process failed */
static pid_t
exec (const char *cmd_line)
{
  if (is_safe_user_ptr (cmd_line))
    {
      struct thread *current = thread_current ();

      current->child_status = LOADING;
      tid_t child_tid = process_execute (cmd_line);

      lock_acquire (&current->cond_lock);
      while (current->child_status == LOADING)
        cond_wait(&current->child_waiter, &current->cond_lock);
      lock_release (&current->cond_lock);

      if (current->child_status == FAILED)
        return -1;
      else
        return child_tid;
    }

  NOT_REACHED ();
}

/* Waits for a child process pid and retrieves the child's exit status. */
static int
wait (pid_t pid)
{
  return process_wait (pid);
}

/* Creates a new file called file initially initial_size bytes in size. Returns
   true iff successful. */
static bool
create (const char *file, unsigned initial_size)
{
  if (is_safe_user_ptr (file))
    return filesys_create (file, initial_size);

  NOT_REACHED ();
}

/* Deletes the file called file. Returns true iff successful. */
static bool
remove (const char *file)
{
  if (is_safe_user_ptr (file))
    return filesys_remove (file);

  NOT_REACHED ();
}

/* Opens the file called filename. Returns a non-negative integer handle, or
   -1 if the file could not be opened. */
static int
open (const char *filename)
{
  if (is_safe_user_ptr (filename))
    {
      struct file *file = filesys_open (filename);
      if (file == NULL)
        return -1;

      struct inode *inode = file_get_inode (file);
      if (inode == NULL)
        return -1;

      struct file *open_file = file_open (inode);
      if (open_file == NULL)
        return -1;

      /* Allocate an fd. */
      /* TODO: should these be heap-allocated? Can pass destructor argument to
               hash_destroy () to handle deallocation. */
      struct fd_node node;
      node.fd = next_fd++;
      node.thread = thread_current ();
      node.file = open_file;
      hash_insert (&fd_hash, &node.hash_elem);

      return node.fd;
    }

	NOT_REACHED ();
}

/* Returns the size, in bytes, of the file open as fd. */
static int
filesize (int fd)
{
  return file_length (fd_to_file (fd));
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
  /* Can't write to standard input. */
  if (fd == 0)
    exit (-1);
  else if (is_safe_user_ptr (buffer))
    {
      if (fd == 1)
        {
          if (size < MAX_PUTBUF)
            {
              putbuf (buffer, size);
              return size;
            }
          else
            {
              int offset = 0;
              while (size > MAX_PUTBUF)
                {
                  putbuf (buffer + offset, MAX_PUTBUF);
                  offset += MAX_PUTBUF;
                  size -= MAX_PUTBUF;
                }

              putbuf (buffer + offset, size);
              offset += size;
              return offset;
            }
        }
      else
        {
          return file_write (fd_to_file (fd), buffer, size);
        }
    }

  NOT_REACHED ();
}

/* Changes the next byte to be read/written in open file fd to position,
   expressed in bytes from the beginning of the file. */
static void
seek (int fd, unsigned position)
{
  file_seek (fd_to_file (fd), position);
}

/* Returns the position of the next byte to be read or written in open file fd,
   expressed in bytes from the beginning of the file. */
static unsigned
tell (int fd)
{
  return file_tell (fd_to_file (fd));
}

/* Closes file descriptor fd. Exiting or terminating a process implicitly closes
   all its open file descriptors, as if by calling this function for each
   one. */
static void
close (int fd)
{
  struct fd_node node;
  node.fd = fd;

  struct hash_elem *e = hash_find (&fd_hash, &node.hash_elem);

  /* fd isn't mapped. Terminate.
     stdin/stdout failure cases are also caught here. */
  if (e == NULL)
    exit (-1);

  struct fd_node *entry = hash_entry (e, struct fd_node, hash_elem);
  file_close (entry->file);

  /* Remove the fd from the map so it can't be closed twice. */
  hash_delete (&fd_hash, &node.hash_elem);
}
