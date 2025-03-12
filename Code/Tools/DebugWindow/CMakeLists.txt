cmake_minimum_required(VERSION 3.20)

#------------------------------------------------------------------------------
# The project name. We'll call it "DebugWindow" to match the .vcxproj
#------------------------------------------------------------------------------
project(DebugWindow LANGUAGES C CXX)

# For Visual Studio (or other multi-config generators), we specify these configurations:
set(CMAKE_CONFIGURATION_TYPES Debug Release CACHE STRING "Configs" FORCE)

#------------------------------------------------------------------------------
# Create a SHARED library target (because ConfigurationType=DynamicLibrary means .dll).
#
# We'll list the .cpp files from <ItemGroup><ClCompile>. The .rc file is included for resources.
# The .def file is also included so the linker sees exported symbols.
#------------------------------------------------------------------------------
add_library(DebugWindow SHARED
    DebugWindow.cpp
    DebugWindowDialog.cpp
    DebugWindow.rc       # Resource file
    DebugWindow.def      # Exports symbol table
    StdAfx.cpp           # Creates the PCH
)

#------------------------------------------------------------------------------
# 1) MFC usage in a static library => we set MFC_FLAG=1
#   (In modern CMake, that is how you replicate "UseOfMfc = Static".)
#------------------------------------------------------------------------------
set_property(TARGET DebugWindow PROPERTY MFC_FLAG 1)

#------------------------------------------------------------------------------
# 2) Precompiled headers
#   The .vcxproj uses "StdAfx.cpp" to Create, and other .cpp files "Use". 
#   The simplest approach in CMake is the built-in target_precompile_headers() command,
#   referencing "stdafx.h" as the PCH. CMake automatically sets up one PCH .obj used by all.
#
#   If you want the exact old-style "StdAfx.cpp -> Create" approach, you can do advanced source
#   property settings. But typically this simpler approach is enough:
#------------------------------------------------------------------------------
target_precompile_headers(DebugWindow PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:stdafx.h>
)

#------------------------------------------------------------------------------
# 3) We are using MFC statically, but the runtime library is also static
#    (/MTd for Debug, /MT for Release). We also see "MultiByte" => define _MBCS.
#------------------------------------------------------------------------------
target_compile_definitions(DebugWindow PRIVATE
  # For both Debug and Release:
  _MBCS

  $<$<CONFIG:Debug>:
    WIN32
    _DEBUG
    _WINDOWS
    _USRDLL
  >
  $<$<CONFIG:Release>:
    WIN32
    NDEBUG
    _WINDOWS
    _USRDLL
  >
)

#------------------------------------------------------------------------------
# 4) Per-configuration compile options
#   - Debug => /Od (Disabled optimization), /MTd (MultiThreadedDebug), plus typical /Zi
#   - Release => /O2 (MaxSpeed), /Ob1 or /Ob2, /MT (MultiThreaded)
#   - This project sets "InlineFunctionExpansion=OnlyExplicitInline" => /Ob1 in Release
#------------------------------------------------------------------------------
target_compile_options(DebugWindow PRIVATE
  $<$<C_COMPILER_ID:MSVC>:
    # For Debug
    $<$<CONFIG:Debug>:
      /Od           # Disabled optimization
      /MTd          # Static runtime, debug
      /Zi           # Debug info
    >
    # For Release
    $<$<CONFIG:Release>:
      /O2           # MaxSpeed optimization
      /Ob1          # "OnlyExplicitInline"
      /MT           # Static runtime
      /GF /Gy       # (Optional) string pooling & function-level linking
    >
  >
)

#------------------------------------------------------------------------------
# 5) Resource compile definitions. Typically _DEBUG or NDEBUG in each config.
#------------------------------------------------------------------------------
if(MSVC)
  target_compile_definitions(DebugWindow PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
  )
endif()

#------------------------------------------------------------------------------
# 6) Output naming for the .dll and the import library:
#   - Debug => DLL: "../../../Run/DebugWindowD_x64.dll", import lib: ".\Debug/DebugWindowD.lib"
#   - Release => DLL: "../../../Run/DebugWindow_x64.dll", import lib: ".\Release/DebugWindow.lib"
#
# We'll replicate that using RUNTIME_OUTPUT_DIRECTORY_<CONFIG> for the DLL 
# and ARCHIVE_OUTPUT_DIRECTORY_<CONFIG> for the .lib (on Windows, a SHARED library
# produces both .dll and .lib).
#------------------------------------------------------------------------------

set_target_properties(DebugWindow PROPERTIES

  # Where the .dll goes:
  RUNTIME_OUTPUT_DIRECTORY_Debug    "${CMAKE_CURRENT_SOURCE_DIR}/../../../Run"
  RUNTIME_OUTPUT_DIRECTORY_Release  "${CMAKE_CURRENT_SOURCE_DIR}/../../../Run"

  OUTPUT_NAME_Debug    "DebugWindowD_x64"
  OUTPUT_NAME_Release  "DebugWindow_x64"

  # Where the .lib (import library) goes:
  ARCHIVE_OUTPUT_DIRECTORY_Debug    "${CMAKE_CURRENT_SOURCE_DIR}/Debug"
  ARCHIVE_OUTPUT_DIRECTORY_Release  "${CMAKE_CURRENT_SOURCE_DIR}/Release"

  OUTPUT_NAME_DEBUG    "DebugWindowD"   # => "DebugWindowD.lib"
  OUTPUT_NAME_RELEASE  "DebugWindow"    # => "DebugWindow.lib"
)

#------------------------------------------------------------------------------
# 7) Link flags. The .vcxproj sets:
#   <SubSystem>Windows</SubSystem> => /SUBSYSTEM:WINDOWS
#   <RandomizedBaseAddress>false</RandomizedBaseAddress> => /DYNAMICBASE:NO
#   <GenerateMapFile>true</GenerateMapFile> => /MAP
#   <ProgramDatabaseFile> => we'll handle with typical /Zi usage in Debug or /DEBUG in Release
#   We'll replicate with link options:
#------------------------------------------------------------------------------
target_link_options(DebugWindow PRIVATE
  $<$<CONFIG:Debug>:
    /SUBSYSTEM:WINDOWS
    /DYNAMICBASE:NO
    /MAP
  >
  $<$<CONFIG:Release>:
    /SUBSYSTEM:WINDOWS
    /DYNAMICBASE:NO
    /MAP
  >
)

message(STATUS "DebugWindow CMake conversion loaded. Configs = ${CMAKE_CONFIGURATION_TYPES}")
