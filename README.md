RTSYS
=====


Introduction
------------

rtsys is dual-licensed under the GPLv3 and a commercial license. If
you require the commercial license, please contact me
"Fabrice Triboix" <ftriboix@gmail.com>.

This set of software modules is meant to help software developer write
software for high-integrity and safety-critical applications.
They are primarily meant to run bare-metal, altough they can easily be
used in the context of an RTOS or even Linux.

Everything in rtsys has been design to run on very small platforms.
Even an 8-bit PIC should be able to run it.


Getting started (under Linux)
-----------------------------

    $ scons --help
    $ scons --prefix=/usr/local     # Build and install under /usr/local
    $ ./run_unit_tests.py           # Run the unit tests

Read the doxygen documentation to learn how to use rtsys.


No warranties
-------------

I wrote these pieces of code on my spare time in the hope that they
will be useful. I make no warranty at all on their suitability for use
in software application, whether safety-critical or not.


Copyright
---------

This software is dual-licensed under the GPLv3 and a commercial
license. The text of the GPLv3 is available in the [LICENSE](LICENSE)
file.

A commercial license can be provided if you do not wish to be bound by
the terms of the GPLv3, or for other reasons. Please refer to the
introduction above.


The Modules
===========


rtplf
-----

This module provides some interface to your architecture, be it
hardware (eg: BSP for bare metal execution) or software (eg: running
as a task in an RTOS or Linux).

Please refer to [the rtplf readme file](src/rtplf/README.md) for more
information.


rtfifo
------

A simple FIFO implementation.

Please refer to [the rtfifo readme file](src/rtfifo/README.md) for more
information.


rthsm
-----

This module implements a hierarchical state machine engine. This is
based on UML2.0, but has a signficant number of restrictions.

Please refer to [the rthsm readme file](src/rthsm/README.md) for more
information.


rttest
------

This module is a test harness. It can be used to run unit tests on
very small systems.

Please refer to [the rttest readme file](src/rttest/README.md) for
more information.

