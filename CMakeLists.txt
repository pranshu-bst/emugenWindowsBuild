#/***********************************************************************
# Copyright (C) 2020 BlueStack Systems, Inc.
# All Rights Reserved
#
# THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF BLUESTACK SYSTEMS, INC.
# The copyright notice above does not evidence any actual or intended
# publication of such source code.
#************************************************************************/
set (TARGET_NAME emugenBuild_client)
project(${TARGET_NAME})

set(HEADERS
    getopt.h
)

set(SOURCES
    getopt.cpp
    ApiGen.cpp
    EntryPoint.cpp
    main.cpp
    strUtils.cpp
    TypeFactory.cpp

)

add_library(${TARGET_NAME}
    STATIC
    ${SOURCES}
)

# target_include_directories(${TARGET_NAME} PUBLIC
#     include
# )