cmake_minimum_required(VERSION 3.20)

#------------------------------------------------------------------------------
# Project name. We'll call it "Compression" to match the library name in the vcxproj.
#------------------------------------------------------------------------------
project(Compression LANGUAGES C CXX)

#------------------------------------------------------------------------------
# The vcxproj has three configurations: Debug, Internal, Release.
# For multi-config generators (e.g. Visual Studio), we set them here.
#------------------------------------------------------------------------------
set(CMAKE_CONFIGURATION_TYPES Debug Internal Release CACHE STRING "Configs" FORCE)

#------------------------------------------------------------------------------
# Add a STATIC library target with the sources from <ItemGroup><ClCompile>.
#------------------------------------------------------------------------------
add_library(Compression STATIC
    # EAC
    EAC/btreeabout.cpp
    EAC/btreedecode.cpp
    EAC/btreeencode.cpp
    EAC/huffabout.cpp
    EAC/huffdecode.cpp
    EAC/huffencode.cpp
    EAC/refabout.cpp
    EAC/refdecode.cpp
    EAC/refencode.cpp

    # LZHCompress
    LZHCompress/CompLibSource/Huff.cpp
    LZHCompress/CompLibSource/Lz.cpp
    LZHCompress/CompLibSource/Lzhl.cpp
    LZHCompress/CompLibSource/Lzhl_tcp.cpp
    LZHCompress/NoxCompress.cpp

    # ZLib (these are .c files, but we can still compile them in CMake)
    ZLib/adler32.c
    ZLib/compress.c
    ZLib/crc32.c
    ZLib/deflate.c
    ZLib/gzio.c
    ZLib/infblock.c
    ZLib/infcodes.c
    ZLib/inffast.c
    ZLib/inflate.c
    ZLib/inftrees.c
    ZLib/infutil.c
    ZLib/maketree.c
    ZLib/trees.c
    ZLib/uncompr.c
    ZLib/zutil.c

    # Top-level
    CompressionManager.cpp
)

#------------------------------------------------------------------------------
# Include directories. The vcxproj sets "..\..\include" for all three configs.
#------------------------------------------------------------------------------
target_include_directories(Compression PRIVATE
    $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
    $<$<CONFIG:Internal>:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
    $<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
)


#------------------------------------------------------------------------------
# Per-configuration preprocessor definitions & compiler flags:
# (We'll unify some common ones, replicate differences with generator expressions.)
#------------------------------------------------------------------------------
target_compile_definitions(Compression PRIVATE
  # All configs define _CRT_SECURE_NO_WARNINGS
  _CRT_SECURE_NO_WARNINGS

  # Internal => also _DISABLE_STRING_ANNOTATION;_DISABLE_VECTOR_ANNOTATION
  $<$<CONFIG:Internal>:_DISABLE_STRING_ANNOTATION _DISABLE_VECTOR_ANNOTATION>
)

target_compile_options(Compression PRIVATE
  $<$<C_COMPILER_ID:MSVC>:
    # Debug => /Od (Optimization=Disabled), /MDd (MultiThreadedDebugDLL)
    $<$<CONFIG:Debug>:/Od /MDd /RTC1 /Zi>

    # Internal => /O2 (MaxSpeed), /Ob1 (OnlyExplicitInline), /MD, string pooling => /GF, function-level link => /Gy
    $<$<CONFIG:Internal>:/O2 /Ob1 /MD /GF /Gy>

    # Release => /O2 (MaxSpeed), /Ob2 (AnySuitable), /MD, string pooling => /GF, function-level link => /Gy
    $<$<CONFIG:Release>:/O2 /Ob2 /MD /GF /Gy>
  >
)

#------------------------------------------------------------------------------
# 5) Output .lib names. The .vcxproj has:
#   - Debug => "..\..\..\Libraries\Lib\CompressionDebug_x64.lib"
#   - Internal => "..\..\..\Libraries\Lib\CompressionInternal_x64.lib"
#   - Release => "..\..\..\Libraries\Lib\Compression_x64.lib"
# We'll replicate with ARCHIVE_OUTPUT_DIRECTORY_<CONFIG> and OUTPUT_NAME_<CONFIG>.
#------------------------------------------------------------------------------
set_target_properties(Compression PROPERTIES
  OUTPUT_NAME_Debug      "CompressionDebug_x64"
  OUTPUT_NAME_Internal   "CompressionInternal_x64"
  OUTPUT_NAME_Release    "Compression_x64"
)

#------------------------------------------------------------------------------
# The vcxproj sets a .pch path, but we won't replicate PCH exactly unless you want to
# use CMake's target_precompile_headers(...) or custom MSVC properties. We'll skip for now.
#
# Additional items (ResourceCompile, etc.) are not relevant here because it's a static library
# with no .rc files.
#------------------------------------------------------------------------------

message(STATUS "Compression library CMake conversion loaded. Configs = ${CMAKE_CONFIGURATION_TYPES}")
