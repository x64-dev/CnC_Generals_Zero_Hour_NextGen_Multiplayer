cmake_minimum_required(VERSION 3.15)

# Tell CMake that we have these four configurations available.
# This ensures that Visual Studio will show Debug, Internal, Profile, and Release.
set(CMAKE_CONFIGURATION_TYPES "Debug;Internal;Profile;Release" CACHE STRING "Configs" FORCE)

project(WWDebug LANGUAGES CXX)

# List your source and header files:
set(SOURCES
    wwdebug.cpp
    wwmemlog.cpp
    wwprofile.cpp
)

set(HEADERS
    wwdebug.h
    wwhack.h
    wwmemlog.h
    wwprofile.h
)

# Create a static library target:
add_library(WWDebug STATIC
    ${SOURCES}
    ${HEADERS}
)

# Match the AdditionalIncludeDirectories in the vcxproj:
target_include_directories(WWDebug PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../wwlib"
)

# Replicate the per‐configuration preprocessor definitions:
target_compile_definitions(WWDebug PRIVATE
    # Debug
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Debug>:WINVER=0x400>
    $<$<CONFIG:Debug>:_WINDOWS>
    $<$<CONFIG:Debug>:WIN32_LEAN_AND_MEAN>

    # Internal
    $<$<CONFIG:Internal>:NDEBUG>
    $<$<CONFIG:Internal>:WINVER=0x400>
    $<$<CONFIG:Internal>:_WINDOWS>
    $<$<CONFIG:Internal>:WIN32_LEAN_AND_MEAN>
    $<$<CONFIG:Internal>:_INTERNAL>

    # Profile
    $<$<CONFIG:Profile>:NDEBUG>
    $<$<CONFIG:Profile>:WWDEBUG>
    $<$<CONFIG:Profile>:WINVER=0x400>
    $<$<CONFIG:Profile>:_WINDOWS>
    $<$<CONFIG:Profile>:WIN32_LEAN_AND_MEAN>
    $<$<CONFIG:Profile>:WIN32>

    # Release
    $<$<CONFIG:Release>:WINVER=0x400>
    $<$<CONFIG:Release>:_WINDOWS>
    $<$<CONFIG:Release>:WIN32_LEAN_AND_MEAN>
    $<$<CONFIG:Release>:NDEBUG>
    $<$<CONFIG:Release>:WIN32>
    $<$<CONFIG:Release>:IG_DEBUG_STACKTRACE>
)

#
# (Optional) If you want to mimic MSVC-specific compiler switches like:
#   /O2 (MaxSpeed), /Ob2 (inline any suitable), /GF (string pooling), /Gy (function-level linking),
#   and treat warnings as errors (/WX), you can do something like:
#
# if(MSVC)
#   target_compile_options(WWDebug PRIVATE
#       "$<$<CONFIG:Debug>:/O2 /Ob2 /GF /Gy /WX>"
#       "$<$<CONFIG:Internal>:/O2 /Ob1 /GF /Gy /WX>"
#       "$<$<CONFIG:Profile>:/O2 /Ob2 /GF /Gy /WX>"
#       "$<$<CONFIG:Release>:/O2 /Ob2 /GF /Gy /WX>"
#   )
# endif()
#

# Match the vcxproj library names for each configuration and place them in the same folder:
#   ..\..\..\Lib\WWDebug.lib, WWDebugDebug_x64.lib, etc.
#
# We use ARCHIVE_OUTPUT_NAME_* because we are building a STATIC library, and
# ARCHIVE_OUTPUT_DIRECTORY_* so that all libraries end up under the same ../Lib folder.

set_target_properties(WWDebug PROPERTIES
    ARCHIVE_OUTPUT_NAME_DEBUG          "WWDebugDebug_x64"
    ARCHIVE_OUTPUT_NAME_INTERNAL       "WWDebugInternal_x64"
    ARCHIVE_OUTPUT_NAME_PROFILE        "WWDebugProfile"
    ARCHIVE_OUTPUT_NAME_RELEASE        "WWDebug"
)

#
# That’s all.  Running CMake (with a multi-config generator like Visual Studio) will now create:
#
#   Debug   ->  ...\Lib\WWDebugDebug_x64.lib
#   Internal->  ...\Lib\WWDebugInternal_x64.lib
#   Profile ->  ...\Lib\WWDebugProfile.lib
#   Release ->  ...\Lib\WWDebug.lib
#
# and it will define the same set of preprocessor symbols in each configuration as in the original project.
#
