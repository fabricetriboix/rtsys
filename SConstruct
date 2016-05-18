import sys
import os
import autodetectplf


# Command-line options

AddOption("--target", dest='target', default="",
        help="Compilation target; eg: x64-linux, arm-linux (default: same as host)")

AddOption("--host", dest='host', default="",
        help="Compilation host; eg: x64-linux (default: auto-detect)")

AddOption("--prefix", dest='prefix', default="",
        help="Installation directory (default: under build directory)")

AddOption("--verbose", dest='verbose', action='store_true', default=False,
        help="Display full command lines")


# Manage cross-compilation

hostplf = GetOption('host')
if not hostplf:
    hostplf = autodetectplf.autodetectplf()
    print("--host not set, using auto-detected host: " + hostplf)

tgtplf = GetOption('target')
if not tgtplf:
    tgtplf = hostplf
    print("--target not set, using same as host: " + tgtplf)

tmp = os.path.join("src", "rtplf", tgtplf)
if not os.access(tmp, os.R_OK):
    print("ERROR: target platform not found: " + tmp)

sys.path.append(tmp)
import plfsettings


# Top-level environment

env = Environment(AR="dummy", CC="dummy", CXX="dummy", ENV={},
        HAS_DOXYGEN="no", HAS_DOT="no")

if not GetOption('verbose'):
    env['CCCOMSTR']     = "CC      $TARGET"
    env['ARCOMSTR']     = "AR      $TARGET"
    env['RANLIBCOMSTR'] = "RANLIB  $TARGET"
    env['LINKCOMSTR']   = "LINK    $TARGET"
    env['INSTALLSTR']   = "INSTALL $TARGET"


# Variants (NB: the first variant is the one built by default)

variantNames = ['release', 'debug']
settings = plfsettings.getPlfSettings(variantNames)
variants = {}
prefix = GetOption('prefix')

for v in variantNames:
    variants[v] = {}
    variants[v]['host'] = hostplf
    variants[v]['target'] = tgtplf

    tmp = os.path.abspath(os.path.join("build", tgtplf, v))
    variants[v]['build_root'] = tmp

    if prefix == "":
        tmp = os.path.join(tmp, "install")
    else:
        tmp = prefix
    variants[v]['install_root'] = tmp
    variants[v]['install_inc'] = os.path.join(tmp, "include")
    variants[v]['install_lib'] = os.path.join(tmp, "lib")
    variants[v]['install_bin'] = os.path.join(tmp, "bin")
    variants[v]['install_doc'] = os.path.join(tmp, "doc")

    variants[v]['env'] = env.Clone()
    variants[v]['env']['CC'] = settings[v]['cc']
    variants[v]['env']['AR'] = settings[v]['ar']
    variants[v]['env']['RANLIB'] = settings[v]['ranlib']
    variants[v]['env'].AppendENVPath('PATH', settings[v]['path'])
    variants[v]['env'].AppendENVPath('PATH',
            os.path.join("#rttest", "scripts"))
    variants[v]['env'].Append(CPPDEFINES = settings[v]['cppdefines'])
    variants[v]['env'].Append(CCFLAGS = settings[v]['ccflags'])
    variants[v]['env'].Append(CPPPATH = settings[v]['cpppath'])
    variants[v]['env'].Append(LINKFLAGS = settings[v]['linkflags'])
    variants[v]['env'].Append(LIBPATH = settings[v]['libpath'])
    variants[v]['env'].Append(LIBPATH = [variants[v]['build_root']])

    # Create another environment that is not c90-pedantic
    flags = []
    for f in settings[v]['ccflags']:
        if f != '-std=c90' and f != '-Wpedantic' and f != '-pedantic-errors':
            flags.append(f)
    variants[v]['envnp'] = variants[v]['env'].Clone()
    variants[v]['envnp'].Replace(CCFLAGS = flags)
    if not GetOption('verbose'):
        variants[v]['envnp']['CCCOMSTR'] = "CCNP    $TARGET"

    # Autoconf-like stuff
    if not GetOption('clean') and not GetOption('help'):
        conf = Configure(variants[v]['env'])

        hasDoxy = False
        hasDot = False
        if not conf.CheckProg("doxygen"):
            print("doxygen not found; doxygen documentation will not be generated")
        else:
            hasDoxy = True
            if not conf.CheckProg("dot"):
                print("dot not found; doxygen documentation will not have graphs")
            else:
                hasDot = True

        if not conf.CheckCC():
            print("C compiler not found: " + variants[v]['env']['CC'])
            Exit(1)

        variants[v]['env'] = conf.Finish()

        # Update the environment after calling `conf.Finish()`
        if hasDoxy:
            variants[v]['env']['HAS_DOXYGEN'] = "yes"
            variants[v]['envnp']['HAS_DOXYGEN'] = "yes"
            if hasDot:
                variants[v]['env']['HAS_DOT'] = "yes"
                variants[v]['envnp']['HAS_DOT'] = "yes"

# Include SConscript for each variant
for v in variantNames:
    SConscript(os.path.join("src", "SConscript"),
            variant_dir=variants[v]['build_root'],
            duplicate=0, exports={'variant': variants[v]})

# Manage targets
alltgt = []
for v in variantNames:
    Alias(v, variants[v]['build_root'])
    alltgt.append(variants[v]['build_root'])

Alias('all', alltgt)
Default(variantNames[0])
