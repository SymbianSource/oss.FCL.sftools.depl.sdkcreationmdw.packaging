# -*- Mode: makefile -*-
#
#  $Id: Makefile,v 1.9 2005/09/22 01:34:29 slava Exp $
#
#  Copyright (c) 2000 - 2005 Nokia Corporation.
#  This material, including documentation and any related
#  computer programs, is protected by copyright controlled by
#  Nokia Corporation. All rights are reserved. Copying,
#  including reproducing, storing, adapting or translating, any
#  or all of this material requires the prior written consent of
#  Nokia Corporation. This material also contains confidential
#  information which may not be disclosed to others without the
#  prior written consent of Nokia Corporation.
#

.PHONY: all clean 

#
# Include master config file
#

CONFIG_DIR = ../../build
include $(CONFIG_DIR)/Config.mak

#
# Directories
#

INC_DIR = include
SRC_DIR = src

#
# Compiler settings
#
INCLUDES = -I$(INC_DIR) -I$(SRC_DIR) $(JAVA_INCLUDES) $(SLIB_INCLUDES)

#
# Targets
#

RELEASE_LIB = libjni.a
DEBUG_LIB = libjnid.a
RESULTS = $(DEBUG_LIB) $(RELEASE_LIB) 

#
# Source files
#

SRC = jni_call.c jni_data.c jni_new.c jni_str.c jni_util.c

#
# Rules
#

all: debug release

clean:
	-rm -fr core *~ $(RESULTS) $(BUILD_DIR)

#
# Common rules
#

include $(CONFIG_DIR)/Rules.mak

#
# Dependencies
#

include $(DEPENDS)
