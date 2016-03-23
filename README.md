RTSYS
=====


Introduction
------------

This set of software modules is meant to help software developer write
software for high-integrity and safety-critical applications.
They are primarily meant to run bare-metal, altough they can easily be
used in the context of an RTOS or even Linux.

Everything in rtsys has been design to run on very small platforms.
Even an 8-bit PIC should be able to run it.


Getting started (under Linux)
-----------------------------

    $ cd fbuild
    $ make                          # Build FSW
    $ make doc                      # Make the doxygen documentation
    $ make ftest                    # Build the test harness
    $ make test                     # Build the unit tests
    $ cd ../ftest/tools
    $ make                          # Build a test harness output converter
    $ cd ../../fbuild/test
    $ ./ftest_farch.exe > test.out  # Run farch unit tests
    $ ../../ftest/tools/ftestoutput2text ftest_farch.map test.out


No warranties
-------------

I wrote these pieces of code on my spare time in the hope that they
will be useful. I make no warranty at all on their suitability for use
in software application, whether safety-critical or not.


Copyright
---------

Please refer to the [LICENSE](LICENSE) file.
This software is dual-licensed. A commercial license can be provided,
please contact me (Fabrice Triboix) for more information.


The Modules
===========


farch
-----

This module provides some interface to your architecture, be it
hardware (eg: BSP for bare metal execution) or software (eg: running
as a task in an RTOS or Linux).

Please refer to [the farch readme file](farch/README.md) for more
information.


fbuild
------

This is just the build system.
The fsw modules are built as a single library: libf.a. The test
harness is built as a separate library: libftest.a.
Please note that it is not allowed to have two source files with the
same name across the whole of the fsw modules.

All the output files (object files, intermediary source files, etc.)
are created under separate directories:
 - fbuild/build: objects and libraries for fsw and the test harness
 - fbuild/test: objects, executable and other stuff related to unit
   tests
 - fbuild/build/docs: built documentation (i.e. doxygen output)

You can use this build system to easily get your modules to build in
the libf.a library. Just follow an existing modules, such as fhsm.


fhsm
----

This module implements a hierarchical state machine engine. This is
based on UML2.0, but has a signficant number of restrictions.

Please refer to [the fhsm readme file](fhsm/README.md) for more
information.


fsched
------

This module implements a very basic time-triggered scheduler.
The idea here is that all tasks run to completion on each tick of a
timer (so in interrupt context). Obviously, tasks must complete
whatever they need to do as quickly as possible and are not allowed to
block.

If you are unfamiliar with that way of doing things, it might look
like some crazy heretic design, but in actuality it is very common in
safety-critical applications, such as aerospace.

But compared to a traditional approach of using an RTOS, or even
event-driven programming (maybe with a superloop), it's actually
a better way to produce reliable software, that can easily pass
safety-critical certifications. In addition, the software is easier
to debug and maintain (no need for mutexes or IPCs, no more priority
inversion problems or anything like that).

Combined with hierarchical state-machines, this is a great way to
write complex software in a clean way.

Please refer to [the fsched readme file](fsched/README.md) for more
information.


ftemplate
---------

This module is not part of libf.a and is just meant to show how to
write code in the fsw way.


ftest
-----

This module is a test harness. It can be used to run unit tests on
very small systems.

Please refer to [the ftest readme file](ftest/README.md) for more
information.

