# Copyright (c) 2016  Fabrice Triboix
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.


###  BEGIN CONFIGURATION PARAMETERS  ###

# **VERY IMPORTANT**: This makefile assumes all source files have different
# names. This is because all object files go to a single build directory. Also
# ensure that no directory or file name has blank in them.

# Set V to 1 to increase verbosity
V = 0

# Set VARIANT to "release" to make a release build
# Supported variants: "debug" (default) and "release"
VARIANT = debug

# Set PLF to the platform you want to build to
# This must be one of the platform directory listed under "src/rtplf"
PLF := $(shell ./autodetectplf.py)

CC = gcc
AR = ar
CFLAGS = -Wall -Wextra -Werror -Wno-unused-parameter -std=c90 -pthread
LDFLAGS = -pthread

ifneq ($(VARIANT),debug)
CFLAGS += -O3
else
CFLAGS += -O0 -g
endif

# Pendantic flags
CFLAGS_P = $(CFLAGS) -Wpedantic -pedantic-errors

###  END CONFIGURATION PARAMETERS  ###



###  DO NOT MODIFY ANYTHIG BELOW THIS LINE  ###

BUILDDIR := build/$(PLF)/$(VARIANT)
TOPDIR := ../../..

# Export all variables
export

all install:
	@mkdir -p $(BUILDDIR) && $(MAKE) -C $(BUILDDIR) -f $(TOPDIR)/make.mk

# TODO: run_unit_tests.py can be improved
test: all
	@./run_unit_tests.py

clean:
	rm -rf $(BUILDDIR)
