cmake_minimum_required(VERSION 3.10)  # or higher as needed
project(DebugWindowExample)

# Create the shared library.
add_library(DebugWindow SHARED
    DebugWindow.cpp
    DebugWindowDialog.cpp
    DebugWindow.rc       # Resource file
    DebugWindow.def      # Exports symbol table
    StdAfx.cpp           # Creates the PCH
)

set_property(TARGET DebugWindow PROPERTY MFC_FLAG 2)

# Precompile headers.
target_precompile_headers(DebugWindow PRIVATE
    stdafx.h
)

# Apply your pre-defined compile definitions, options, and link options.
target_compile_definitions(DebugWindow PRIVATE
    ${GNG_COMPILE_DEFINITIONS}
)
target_compile_options(DebugWindow PRIVATE
    ${GNG_COMPILE_OPTIONS}
)
target_link_options(DebugWindow PRIVATE
    ${GNG_LINK_OPTIONS}
)

target_compile_options(DebugWindow PRIVATE /MT)

# Place the DLL (and import library) under ..\..\run\
# Note: On Windows, .dll is treated as "RUNTIME" output, .lib as "LIBRARY".
set_target_properties(DebugWindow PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../../../run/"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../../../run/"
)
