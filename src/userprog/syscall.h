#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <stdbool.h>

/* Program ID type. */
typedef int pid_t;

/* Map region identifier. */
typedef int mapid_t;

void syscall_init (void);
void syscall_done (void);
void exit (int status);
void lock_filesystem (void);
void release_filesystem (void);
bool is_mapped (void *addr);
struct mapid_node * addr_to_map (void *addr);

#define MAX_PUTBUF 512

#endif /* userprog/syscall.h */
