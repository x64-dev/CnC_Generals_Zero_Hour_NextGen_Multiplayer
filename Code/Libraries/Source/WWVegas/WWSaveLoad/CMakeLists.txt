cmake_minimum_required(VERSION 3.15)

# Your library name can be "WWSaveLoad" or whatever you like:
project(WWSaveLoad LANGUAGES CXX)

# We want 6 configs in Visual Studio: Debug, DebugE, Internal, Profile, ProfileE, Release
set(CMAKE_CONFIGURATION_TYPES "Debug;DebugE;Internal;Profile;ProfileE;Release" CACHE STRING "Configs" FORCE)

# Create a STATIC library target (like the MSBuild "StaticLibrary" config).
add_library(wwsaveload STATIC
    definition.cpp
    definitionfactory.cpp
    definitionfactorymgr.cpp
    definitionmgr.cpp
    parameter.cpp
    persistfactory.cpp
    pointerremap.cpp
    saveload.cpp
    saveloadstatus.cpp
    saveloadsubsystem.cpp
    twiddler.cpp
    wwsaveload.cpp

    # If you want to list headers too, just for IDE visibility:
    # definition.h
    # definitionclassids.h
    # definitionfactory.h
    # ...
)

#
# --- Output library file names per configuration.
#     The original .vcxproj uses different .lib filenames.
#     We'll place them all in the same directory, but you can
#     adjust the directory if you want to match exactly (..\..\..\Lib).
#
set_target_properties(wwsaveload PROPERTIES

    # Place all .lib outputs in, say, a "Lib" folder under the build directory:
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Lib"

    # Match the .vcxproj's specific filenames:
    OUTPUT_NAME_DEBUG    "WWSaveLoadDebug_x64"
    OUTPUT_NAME_DEBUGE   "WWSaveLoadDebugE"
    OUTPUT_NAME_INTERNAL "WWSaveLoadInternal_x64"
    OUTPUT_NAME_PROFILE  "WWSaveLoadProfile"
    OUTPUT_NAME_PROFILEE "WWSaveLoadProfileE"
    OUTPUT_NAME_RELEASE  "WWSaveLoad"
)

#
# --- Common include directories (used by all configs).
#     Matches '..\wwlib;..\wwdebug;..\wwmath' from the .vcxproj.
#
target_include_directories(wwsaveload PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../wwlib"
    "${CMAKE_CURRENT_SOURCE_DIR}/../wwdebug"
    "${CMAKE_CURRENT_SOURCE_DIR}/../wwmath"
)

#
# --- Preprocessor definitions per configuration, mirroring the .vcxproj.
#     We use generator expressions: $<$<CONFIG:DebugE>: ... >
#
target_compile_definitions(wwsaveload PRIVATE

    # Debug => _DEBUG; WINVER=0x400; _WINDOWS
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Debug>:WINVER=0x400>
    $<$<CONFIG:Debug>:_WINDOWS>

    # DebugE => _DEBUG; WWDEBUG; WINVER=0x400; _WINDOWS; WIN32; PARAM_EDITING_ON
    $<$<CONFIG:DebugE>:_DEBUG>
    $<$<CONFIG:DebugE>:WWDEBUG>
    $<$<CONFIG:DebugE>:WINVER=0x400>
    $<$<CONFIG:DebugE>:_WINDOWS>
    $<$<CONFIG:DebugE>:WIN32>
    $<$<CONFIG:DebugE>:PARAM_EDITING_ON>

    # Internal => NDEBUG; WINVER=0x400; _WINDOWS; _INTERNAL
    $<$<CONFIG:Internal>:NDEBUG>
    $<$<CONFIG:Internal>:WINVER=0x400>
    $<$<CONFIG:Internal>:_WINDOWS>
    $<$<CONFIG:Internal>:_INTERNAL>

    # Profile => NDEBUG; WWDEBUG; WINVER=0x400; _WINDOWS; WIN32
    $<$<CONFIG:Profile>:NDEBUG>
    $<$<CONFIG:Profile>:WWDEBUG>
    $<$<CONFIG:Profile>:WINVER=0x400>
    $<$<CONFIG:Profile>:_WINDOWS>
    $<$<CONFIG:Profile>:WIN32>

    # ProfileE => NDEBUG; WWDEBUG; WINVER=0x400; _WINDOWS; WIN32; PARAM_EDITING_ON
    $<$<CONFIG:ProfileE>:NDEBUG>
    $<$<CONFIG:ProfileE>:WWDEBUG>
    $<$<CONFIG:ProfileE>:WINVER=0x400>
    $<$<CONFIG:ProfileE>:_WINDOWS>
    $<$<CONFIG:ProfileE>:WIN32>
    $<$<CONFIG:ProfileE>:PARAM_EDITING_ON>

    # Release => WINVER=0x400; _WINDOWS; NDEBUG; WIN32; IG_DEBUG_STACKTRACE
    $<$<CONFIG:Release>:WINVER=0x400>
    $<$<CONFIG:Release>:_WINDOWS>
    $<$<CONFIG:Release>:NDEBUG>
    $<$<CONFIG:Release>:WIN32>
    $<$<CONFIG:Release>:IG_DEBUG_STACKTRACE>
)

#
# --- Example: Match the different MSVC runtime libraries (/MDd vs /MD),
#     optimization settings (/Od vs /O2), etc.
#
#     NOTE: In your .vcxproj:
#       - Debug, DebugE => MultiThreadedDebugDLL => /MDd
#       - Internal, Profile, ProfileE, Release => MultiThreadedDLL => /MD
#       - DebugE, ProfileE => Optimization = Disabled => /Od
#       - Debug => "MaxSpeed" => typically /O2, "InlineFunctionExpansion=AnySuitable" => /Ob2
#       - Internal => "MaxSpeed", "OnlyExplicitInline" => /O2 /Ob1
#       - Profile => "MaxSpeed", "AnySuitable" => /O2 /Ob2
#       - Release => "MaxSpeed", "AnySuitable" => /O2 /Ob2
#


#
# That’s the core. You can keep adding custom commands or other flags
# (e.g., /GF for string pooling) if you want to replicate every detail.
#
