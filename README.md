Task 3 Plan
===========

Who will do what?
-----------------
* Owen:
    * Supplemental page table and page fault handler
    * Page reclamation on process exit
* Dan:
    * Stack growth
* Charlie:
    * Frame table
    * Mapped files

After that: eviction, swapping, sharing. (Anything else?)

Current test status
-------------------

pass tests/filesys/base/syn-write  
pass tests/userprog/args-none  
pass tests/userprog/args-single  
pass tests/userprog/args-multiple  
pass tests/userprog/args-many  
pass tests/userprog/args-dbl-space  
pass tests/userprog/sc-bad-sp  
pass tests/userprog/sc-bad-arg  
pass tests/userprog/sc-boundary  
pass tests/userprog/sc-boundary-2  
pass tests/userprog/halt  
pass tests/userprog/exit  
pass tests/userprog/create-normal  
pass tests/userprog/create-empty  
pass tests/userprog/create-null  
pass tests/userprog/create-bad-ptr  
pass tests/userprog/create-long  
pass tests/userprog/create-exists  
pass tests/userprog/create-bound  
pass tests/userprog/open-normal  
pass tests/userprog/open-missing  
pass tests/userprog/open-boundary  
pass tests/userprog/open-empty  
pass tests/userprog/open-null  
pass tests/userprog/open-bad-ptr  
pass tests/userprog/open-twice  
pass tests/userprog/close-normal  
pass tests/userprog/close-twice  
pass tests/userprog/close-stdin  
pass tests/userprog/close-stdout  
pass tests/userprog/close-bad-fd  
pass tests/userprog/read-normal  
pass tests/userprog/read-bad-ptr  
pass tests/userprog/read-boundary  
pass tests/userprog/read-zero  
pass tests/userprog/read-stdout  
pass tests/userprog/read-bad-fd  
pass tests/userprog/write-normal  
pass tests/userprog/write-bad-ptr  
pass tests/userprog/write-boundary  
pass tests/userprog/write-zero  
pass tests/userprog/write-stdin  
pass tests/userprog/write-bad-fd  
pass tests/userprog/exec-once  
pass tests/userprog/exec-arg  
pass tests/userprog/exec-multiple  
pass tests/userprog/exec-missing  
pass tests/userprog/exec-bad-ptr  
pass tests/userprog/wait-simple  
pass tests/userprog/wait-twice  
pass tests/userprog/wait-killed  
pass tests/userprog/wait-bad-pid  
pass tests/userprog/multi-recurse  
pass tests/userprog/multi-child-fd  
pass tests/userprog/rox-simple  
pass tests/userprog/rox-child  
pass tests/userprog/rox-multichild  
pass tests/userprog/bad-read  
pass tests/userprog/bad-write  
pass tests/userprog/bad-read2  
pass tests/userprog/bad-write2  
pass tests/userprog/bad-jump  
pass tests/userprog/bad-jump2  
pass tests/vm/pt-grow-stack  
pass tests/vm/pt-grow-pusha  
pass tests/vm/pt-grow-bad  
pass tests/vm/pt-big-stk-obj  
pass tests/vm/pt-bad-addr  
pass tests/vm/pt-bad-read  
pass tests/vm/pt-write-code  
pass tests/vm/pt-write-code2  
pass tests/vm/pt-grow-stk-sc  
pass tests/vm/page-linear  
FAIL tests/vm/page-parallel  
pass tests/vm/page-merge-seq  
FAIL tests/vm/page-merge-par  
FAIL tests/vm/page-merge-stk  
FAIL tests/vm/page-merge-mm  
pass tests/vm/page-shuffle  
pass tests/vm/mmap-read  
pass tests/vm/mmap-close  
pass tests/vm/mmap-unmap  
pass tests/vm/mmap-overlap  
pass tests/vm/mmap-twice  
pass tests/vm/mmap-write  
pass tests/vm/mmap-exit  
pass tests/vm/mmap-shuffle  
pass tests/vm/mmap-bad-fd  
pass tests/vm/mmap-clean  
pass tests/vm/mmap-inherit  
pass tests/vm/mmap-misalign  
pass tests/vm/mmap-null  
pass tests/vm/mmap-over-code  
pass tests/vm/mmap-over-data  
pass tests/vm/mmap-over-stk  
pass tests/vm/mmap-remove  
pass tests/vm/mmap-zero  
pass tests/filesys/base/lg-create  
pass tests/filesys/base/lg-full  
pass tests/filesys/base/lg-random  
pass tests/filesys/base/lg-seq-block  
pass tests/filesys/base/lg-seq-random  
pass tests/filesys/base/sm-create  
pass tests/filesys/base/sm-full  
pass tests/filesys/base/sm-random  
pass tests/filesys/base/sm-seq-block  
pass tests/filesys/base/sm-seq-random  
pass tests/filesys/base/syn-read  
pass tests/filesys/base/syn-remove  
pass tests/filesys/base/syn-write  
4 of 109 tests failed.  
