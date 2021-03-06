############################################################################
#
#   Microsoft Windows
#   Copyright (C) Microsoft Corporation, 2000 - 1993.
#   All rights reserved.
#
############################################################################

!include types.mk

BUILDMSG        =

TDLTYPES        = $(OLEAUTO_TDLTYPES)

PROXYTYPES      = $(OLEAUTO_PROXYTYPES)

NONPROXYTYPES   = $(OLEAUTO_NONPROXYTYPES)

BASE_TYPES      = $(OLEAUTO_BASE_TYPES)

BASE_HFILES     = $(OLEAUTO_BASE_HFILES)

TDLFILES        = $(TDLTYPES:.type=.tdl)

TYPES           = $(PROXYTYPES) $(NONPROXYTYPES)

IDLFILES        = $(TYPES:.type=.idl) $(BASE_TYPES:.type=.idl)

BASE_HFILES     = $(BASE_TYPES:.type=.h)

USE_OLE_MC      = 1

IDLFLAGS        = /import defined_single

ITFS_BASE       = oleauto

ITFS_DIR        = $(HCOPY)

CLEANFILES      = $(ITFS_DIR)\oleauto.h

!include ..\makefile.inc
!include $(DEPENDFILE)

all: $(MIDL_INCS)

