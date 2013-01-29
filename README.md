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
    * Floating point maths
    * BSD Scheduler - see Appendix B for more

Full details on pages 16-26 of spec, in the sections listed.

Who will do what?
-----------------
* Owen is doing Alarm clock, floating point maths stuff
* Dan is doing Priority scheduling
* Charlie is doing Priority Donation
* We can start work on Advanced Scheduler when we're done (this will probably be mostly Owen since Alarm clock should be quite short)

Branch off to do your appropriate work and pull back into master every now and then. We should use pull requests on github to get eachother's branches if we need them (I think).
