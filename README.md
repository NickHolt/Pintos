Task 1 Plan
===========

Things to do:
-------------
* Alarm clock - 3.2.2
    * `timer.sleep()` defined in `devices/timer.c`
    * Passes all tests except `alarm-priority`, which I think needs Priority Scheduling to work first
* Priority Scheduling - 3.2.3
* Priority Donation - 3.2.4
    * `thread_set_priority()` in `threads/thread.c`
    * `thread_get_priority()` also in `threads/thread.c`
* Advanced Scheduler - 3.2.5
    * Floating point maths - seems to work
    * BSD Scheduler - see Appendix B for more
        * Most of the logic is done, just need to implement the scheduling part

Full details on pages 16-26 of spec, in the sections listed.

Who will do what?
-----------------
* Owen is doing Alarm clock, floating point maths stuff, advanced scheduler
* Dan is doing Priority scheduling
* Charlie is doing Priority Donation

Branch off to do your appropriate work and pull back into master every now and then. We should use pull requests on github to get eachother's branches if we need them (I think).

Test status
-----------

pass tests/threads/alarm-single  
pass tests/threads/alarm-multiple  
pass tests/threads/alarm-simultaneous  
pass tests/threads/alarm-priority  
pass tests/threads/alarm-zero  
pass tests/threads/alarm-negative  
pass tests/threads/priority-change  
FAIL tests/threads/priority-donate-one  
FAIL tests/threads/priority-donate-multiple  
FAIL tests/threads/priority-donate-multiple2  
FAIL tests/threads/priority-donate-nest  
FAIL tests/threads/priority-donate-sema  
FAIL tests/threads/priority-donate-lower  
pass tests/threads/priority-fifo  
pass tests/threads/priority-preempt  
FAIL tests/threads/priority-sema  
FAIL tests/threads/priority-condvar  
FAIL tests/threads/priority-donate-chain  
pass tests/threads/mlfqs-load-1  
pass tests/threads/mlfqs-load-60  
pass tests/threads/mlfqs-load-avg  
pass tests/threads/mlfqs-recent-1  
pass tests/threads/mlfqs-fair-2  
pass tests/threads/mlfqs-fair-20  
pass tests/threads/mlfqs-nice-2  
pass tests/threads/mlfqs-nice-10  
FAIL tests/threads/mlfqs-block  
