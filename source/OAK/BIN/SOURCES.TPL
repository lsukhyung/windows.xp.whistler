    !!!!!!!!!!!!!!!!!!!!! WHAT YOU NEED TO DO !!!!!!!!!!!!!!!!!!!!!

    - replace the xxxxx in the MAJORCOMP= and MINORCOMP= macros below
      with your components major and minor component name
      (e.g. MAJORCOMP=xpos, MINORCOMP=ke etc.).

    - replace the xxxxx in the TARGETNAME= macro below with your target name
      (e.g. OS2, POSIX, PM, etc.).

    - edit the TARGETPATH= and TARGETTYPE= macros below to specify the location
      any type of the target.

    - edit the INCLUDES= macro below if you have a private include directory
      to search.

    - carefully edit the SOURCES= macro below so that it defines all the source
      files for this subcomponent

    - edit out these noisy instructions and all comments

    - save this file, exit the editor and execute the "build" command.

    - From now on, issue build or nmake to build the subcomponent.

!IF 0

Copyright (c) 2000  Microsoft Corporation

Module Name:

    sources.

Abstract:

    This file specifies the target component being built and the list of
    sources files needed to build that component.  Also specifies optional
    compiler switches and libraries that are unique for the component being
    built.


Author:

    Steve Wood (stevewo) 12-Apr-1990

NOTE:   Commented description of this file is in \xp\bak\bin\sources.tpl

!ENDIF


#
# The MAJORCOMP and MINORCOMP variables are defined
# so that $(MAJORCOMP)$(MINORCOMP)filename can be used in
# cross compiling to provide unique filenames in a flat namespace.
#

MAJORCOMP=xxxxx
MINORCOMP=xxxxx

#
# The TARGETNAME variable is defined by the developer.  It is the name of
# the target (component) that is being built by this makefile.  It
# should NOT include any path or file extension information.
#

TARGETNAME=xxxxx


#
# The TARGETPATH and TARGETTYPE variables are defined by the developer.
# The first specifies where the target is to be build.  The second specifies
# the type of target (either PROGRAM, DYNLINK, LIBRARY, UMAPPL_NOLIB or
# BOOTPGM).  UMAPPL_NOLIB is used when you're only building user-mode
# apps and don't need to build a library.
#

TARGETPATH=..\obj

# Pick one of the following and delete the others
TARGETTYPE=PROGRAM
TARGETTYPE=DYNLINK
TARGETTYPE=LIBRARY
TARGETTYPE=UMAPPL_NOLIB
TARGETTYPE=BOOTPGM


#
# The TARGETLIBS specifies additional libraries to link with you target
# image.  Each library path specification should contain an asterisk (*)
# where the machine specific subdirectory name should go.
#

TARGETLIBS=


#
# The INCLUDES variable specifies any include paths that are specific to
# this source directory.  Separate multiple directory paths with single
# semicolons.  Relative path specifications are okay.
#

INCLUDES=..\inc


#
# The SOURCES variable is defined by the developer.  It is a list of all the
# source files for this component.  Each source file should be on a separate
# line using the line continuation character.  This will minimize merge
# conflicts if two developers adding source files to the same component.
#

SOURCES=source1.c  \
        source2.c  \
        source3.c  \
        source4.c

i386_SOURCES=i386\source1.asm

i860_SOURCES=i860\source1.s

MIPS_SOURCES=mips\source1.s


#
# Next specify options for the compiler.
#

C_DEFINES=
#   USE_CL860_LARGE_MODEL=


#
# Next specify a kernel mode test (valid only in NTOS tree)
#

NTTEST=

#
# Next specify one or more user mode test programs and their type
# UMTEST is used for optional test programs.  UMAPPL is used for
# programs that always get built when the directory is built.
#

UMTYPE=nt
UMTEST=foo*baz
UMAPPL=foo*baz
UMBASE=0x1000000
UMLIBS=obj\*\foo.lib

#
# Defining either (or both) the variables NTTARGETFILE0 and/or NTTARGETFILES
# will cause MAKEFILE.DEF to include .\makefile.inc immediately after it
# specifies the top level targets (all, clean and loc) and their dependencies.
# MAKEFILE.DEF also expands NTTARGETFILE0 as the first dependent for the
# "all" target and NTTARGETFILES as the last dependent for the "all" target.
# Useful for specifying additional targets and dependencies that don't fit the
# general case covered by MAKEFILE.DEF
#

#   NTTARGETFILE0=
#   NTTARGETFILES=
