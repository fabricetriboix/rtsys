Import('variant')

import os

incs = []
objs = []
rttestobjs = []
rttestmainobj = ""
testsrcs = []
testrttobjs = []

subdirs = [os.path.join("rtplf", variant['target']),
           "rttest", "rtfifo", "rthsm"]

for subdir in subdirs:
    artefacts = SConscript(os.path.join(subdir, "SConscript"),
            exports={'variant': variant})
    variant['env'].Append(CPPPATH = [os.path.join("#" + subdir, "include")])
    variant['envnp'].Append(CPPPATH = [os.path.join("#" + subdir, "include")])

    for key, value in artefacts.items():
        if key == 'incs':
            incs.extend([os.path.join(subdir, inc) for inc in value])

        elif key == 'objs':
            objs.extend(value)

        elif key == 'rttestobjs':
            rttestobjs.extend(value)

        elif key == 'rttestmainobj':
            rttestmainobj = value

        elif key == 'testsrcs':
            for src in value:
                testsrcs.append(os.path.join(subdir, src))

        elif key == 'testrttobjs':
            testrttobjs.extend(value)

lib = variant['env'].StaticLibrary('rtsys', objs)
rttestlib = variant['env'].StaticLibrary('rttest', rttestobjs)

# NB: We have to do a bit of gymnastics to get scons to install files
# or directories that are not the results of some build process. This
# is because scons will use the relative paths from the variant
# directory, not the source directory. The way to solve that is to use
# absolute paths for any file/directory in the source directories.
# NB2: Don't use `os.getcwd()` here, it will give you the variant
# directory.
for inc in incs:
    variant['env'].Install(variant['install_inc'], "#" + inc)

# Build doxygen documentation
if variant['env']['HAS_DOXYGEN'] == "yes":
    cmd = os.path.join(".", "run_doxygen.py")
    cmd += " "
    if variant['env']['HAS_DOT'] == "yes":
        cmd += "--hasdot "
    doxyDir = os.path.join(variant['build_root'], "doxy")
    cmd += doxyDir + " "
    cmd += " ".join(incs)
    variant['env'].Command("doc", incs, cmd)
    variant['env'].Install(variant['install_doc'], doxyDir)

# Install final libraries
variant['env'].Install(variant['install_lib'], lib)
variant['env'].Install(variant['install_lib'], rttestlib)
installed_rttestmainobj = variant['env'].Install(variant['install_lib'], rttestmainobj)

# Build test program
if len(testsrcs) > 0:
    testobjs = [variant['envnp'].StaticObject(src) for src in testsrcs]
    testobjs.append(installed_rttestmainobj)
    variant['env'].Program("rtsys_unit_tests", testobjs,
            LIBS=['rttest', 'rtsys'])

# Build test program for rttest
if len(testrttobjs) > 0:
    variant['env'].Program("rttest_unit_tests", testrttobjs,
            LIBS=['rttest', 'rtsys'])