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


rtplf
-----

This module provides some interface to your architecture, be it
hardware (eg: BSP for bare metal execution) or software (eg: running
as a task in an RTOS or Linux).

Please refer to [the rtplf readme file](rtplf/README.md) for more
information.


rtfifo
------

A simple FIFO implementation.

Please refer to [the rtfifo readme file](rtfifo/README.md) for more
information.


rthsm
-----

This module implements a hierarchical state machine engine. This is
based on UML2.0, but has a signficant number of restrictions.

Please refer to [the rthsm readme file](rthsm/README.md) for more
information.


rttest
------

This module is a test harness. It can be used to run unit tests on
very small systems.

Please refer to [the rttest readme file](rttest/README.md) for more
information.

