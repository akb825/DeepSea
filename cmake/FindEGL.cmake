# - Try to Find EGL
# Once done, this will define
#
#  EGL_FOUND - system has EGL installed.
#  EGL_INCLUDE_DIRS - directories which contain the EGL headers.
#  EGL_LIBRARIES - libraries required to link against EGL.
#  EGL_DEFINITIONS - Compiler switches required for using EGL.
#
# Copyright (C) 2012 Intel Corporation. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1.  Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
# 2.  Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND ITS CONTRIBUTORS ``AS
# IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR ITS
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


find_package(PkgConfig)

if (NOT CMAKE_CROSSCOMPILING)
	pkg_check_modules(PC_EGL egl)
endif()

if (PC_EGL_FOUND)
    set(EGL_DEFINITIONS ${PC_EGL_CFLAGS_OTHER})
endif ()

if (ANDROID)
    set(ANDROID_INCLUDE_DIRS ${ANDROID_NDK}/sysroot/usr/include
		NO_DEFAULT_PATH NO_CMAKE_SYSTEM_PATH)
else()
    set(ANDROID_INCLUDE_DIRS)
endif()

find_path(EGL_INCLUDE_DIRS NAMES EGL/egl.h
    HINTS ${PC_EGL_INCLUDEDIR} ${PC_EGL_INCLUDE_DIRS} ${ANDROID_INCLUDE_DIRS}
    NO_CMAKE_FIND_ROOT_PATH
)

if (ANDROID)
    # Don't set full path on Android since the build system will copy over the library, causing
    # initialization to fail.
    set(EGL_LIBRARIES EGL)
else()
    set(EGL_NAMES ${EGL_NAMES} egl EGL)
    find_library(EGL_LIBRARIES NAMES ${EGL_NAMES}
        HINTS ${PC_EGL_LIBDIR} ${PC_EGL_LIBRARY_DIRS} ${ANDROID_LIBRARY_DIRS}
        NO_CMAKE_FIND_ROOT_PATH
    )
endif()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(EGL DEFAULT_MSG EGL_INCLUDE_DIRS EGL_LIBRARIES)

mark_as_advanced(EGL_INCLUDE_DIRS EGL_LIBRARIES)
