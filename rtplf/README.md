rtplf
=====

The rtplf module is used by all other modules and encapsulate all
aspects related to the platform the software is running on.

Some architectures are provided for fairly common platforms. If yours
is not listed (or if you need to customise one), you will need to
create a new directory with the appropriate headers and source files,
so that all the macros, types and functions that the farch module must
provide are defined.

To cross-compile for a certain platform, please provide the `--target`
option to `scons` when building the software. By default, the host
platform is used.
