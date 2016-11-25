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

SHELL := /bin/sh

# Check existence of tools
CCACHE := $(shell which ccache 2> /dev/null)
DOXYGEN := $(shell which doxygen 2> /dev/null)
DOT := $(shell which dot 2> /dev/null)

MODULES := $(TOPDIR)/src/rtplf/$(PLF) $(TOPDIR)/src/rtfifo $(TOPDIR)/src/rthsm \
           $(TOPDIR)/src/rttest

# Path for make to search for source files
VPATH := $(foreach i,$(MODULES),$(i)/src) $(foreach i,$(MODULES),$(i)/test)

# Output libraries
LIBS := librtsys.a librttest.a

# Include paths for compilation
INCS := $(foreach i,$(MODULES),-I$(i)/include)

# List public header files
HDRS := $(foreach i,$(MODULES),$(wildcard $(i)/include/*.h))

# List of object files for various targets
LIBRTSYS_OBJS := rtplf.o rtfifo.o rthsm.o
LIBRTTEST_OBJS := rttest.o
RTTEST_MAIN_OBJ := rttestmain.o
RTTEST_TEST_OBJS := unittest1.o unittest2.o testme.o
RTSYS_TEST_OBJS := test-rtplf.o test-rtfifo.o test-rthsm.o


# Standard targets

all: librtsys.a librttest.a rttest_unit_tests rtsys_unit_tests doc

doc: $(BUILDDIR)/doc/html/index.html


# Rules to build object files, libraries, programs, etc.

define RUN_CC_P
set -eu; \
cmd="$(CC) $(CFLAGS_P) $(INCS) -o $(1) -c $(2)"; \
if [ $(V) == 1 ]; then \
	echo "$$cmd"; \
else \
	echo "CC_P  $(1)"; \
fi; \
$$cmd
endef

define RUN_CC
set -eu; \
cmd="$(CC) $(CFLAGS) $(INCS) -o $(1) -c $(2)"; \
if [ $(V) == 1 ]; then \
	echo "$$cmd"; \
else \
	echo "CC    $(1)"; \
fi; \
$$cmd
endef

define RUN_AR
set -eu; \
cmd="$(AR) crs $(1) $(2)"; \
if [ $(V) == 1 ]; then \
	echo "$$cmd"; \
else \
	echo "AR    $(1)"; \
fi; \
$$cmd
endef

define RUN_LINK
set -eu; \
cmd="$(CC) $(LINKFLAGS) -o $(1) $(2) -L. $(3)"; \
if [ $(V) == 1 ]; then \
	echo "$$cmd"; \
else \
	echo "LINK  $(1)"; \
fi; \
$$cmd
endef

rtplf.o: rtplf.c
	@$(call RUN_CC_P,$@,$<)

rtfifo.o: rtfifo.c
	@$(call RUN_CC_P,$@,$<)

rthsm.o: rthsm.c
	@$(call RUN_CC_P,$@,$<)

librtsys.a: $(LIBRTSYS_OBJS)

librttest.a: $(LIBRTTEST_OBJS)

rttest_unit_tests: $(RTTEST_TEST_OBJS) $(RTTEST_MAIN_OBJ) $(LIBS)
	@$(call RUN_LINK,$@,$^,-lrttest -lrtsys)

rtsys_unit_tests: $(RTSYS_TEST_OBJS) $(RTTEST_MAIN_OBJ) $(LIBS)
	@$(call RUN_LINK,$@,$^,-lrttest -lrtsys)


$(BUILDDIR)/doc/html/index.html: $(HDRS)
ifeq ($(DOXYGEN),)
	@echo "Doxygen not found, documentation will not be built"
else
	@set -eu; \
	if [ -z "$(DOT)" ]; then \
		cp $(TOPDIR)/Doxyfile doxy1; \
	else \
		cat $(TOPDIR)/Doxyfile | sed -e 's/HAVE_DOT.*/HAVE_DOT = YES/' > doxy1;\
	fi; \
	cat doxy1 | sed -e 's:INPUT[	 ]*=.*:INPUT = $^:' > doxy2; \
	cmd="doxygen doxy3"; \
	if [ $(V) == 1 ]; then \
		cp doxy2 doxy3; \
		echo "$$cmd"; \
	else \
		cat doxy2 | sed -e 's/QUIET.*/QUIET = YES/' > doxy3; \
		echo "DOXY  $@"; \
	fi; \
	$$cmd
endif

test: test_rttest test_rtsys

test_rttest: rttest_unit_tests
	@set -eu; \
	./$< > rttest.rtt; \
	if `cmp rttest.rtt $(TOPDIR)/src/rttest/test/expected.rtt > /dev/null`; \
	then \
		echo "TEST  rttest OK"; \
	else \
		echo "FAIL  rttest"; \
		exit 1; \
	fi

test_rtsys: rtsys_unit_tests
	@set -eu; \
	./$< > rtsys.rtt; \
	find $(TOPDIR) -name 'test-*.c' \
		| xargs $(TOPDIR)/src/rttest/scripts/rttest2text.py rtsys.rtt


lib%.a:
	@$(call RUN_AR,$@,$^)

%.o: %.c
	@$(call RUN_CC,$@,$<)

dbg:
	@echo "Platform = $(PLF)"
	@echo "VPATH = $(VPATH)"
	@echo "HDRS = $(HDRS)"
