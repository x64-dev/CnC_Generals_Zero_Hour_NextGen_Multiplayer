cmake_minimum_required(VERSION 3.20)

#------------------------------------------------------------------------------
# Project name. We'll call it "ParticleEditor" to match the .vcxproj
#------------------------------------------------------------------------------
project(ParticleEditor LANGUAGES CXX)

# Since the .vcxproj has only two configs (Debug|x64, Release|x64), we set them:
set(CMAKE_CONFIGURATION_TYPES Debug Release CACHE STRING "Configs" FORCE)

#------------------------------------------------------------------------------
# 1) Create a SHARED library target (DynamicLibrary => .dll).
#
# We'll list the .cpp files from <ItemGroup><ClCompile> plus the resource file and .def file.
#------------------------------------------------------------------------------
add_library(ParticleEditor SHARED
    # Resource
    ParticleEditor.rc
    # .def file for exports
    ParticleEditor.def
    # .cpp files
    CButtonShowColor.cpp
    CColorAlphaDialog.cpp
    CSwitchesDialog.cpp
    EmissionTypePanels.cpp
    MoreParmsDialog.cpp
    ParticleEditor.cpp
    ParticleEditorDialog.cpp
    ParticleTypePanels.cpp
    ShaderTypePanels.cpp
    StdAfx.cpp  # Creates the PCH
    VelocityTypePanels.cpp
)

#------------------------------------------------------------------------------
# 2) MFC usage = Static => set MFC_FLAG=1
#   Also define _MBCS for MultiByte char set usage.
#------------------------------------------------------------------------------
set_property(TARGET ParticleEditor PROPERTY MFC_FLAG 1)

target_compile_definitions(ParticleEditor PRIVATE
    _MBCS
)

#------------------------------------------------------------------------------
# 3) Precompiled header
#   The .vcxproj uses "StdAfx.cpp -> Create", and other .cpp -> Use. 
#   In CMake, a simpler approach is "target_precompile_headers":
#------------------------------------------------------------------------------
target_precompile_headers(ParticleEditor PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:stdafx.h>
)

#------------------------------------------------------------------------------
# 4) Per-configuration compile definitions and flags:
#
# Debug => <Optimization>Disabled => /Od, <RuntimeLibrary>MultiThreadedDebug => /MTd
#          plus #defines: WIN32, _DEBUG, _WINDOWS, _USRDLL
#
# Release => <Optimization>MaxSpeed => /O2, "AnySuitable" => /Ob2, <RuntimeLibrary>MultiThreaded => /MT
#            plus #defines: WIN32, NDEBUG, _WINDOWS, _USRDLL
#------------------------------------------------------------------------------
target_compile_definitions(ParticleEditor PRIVATE
  $<$<CONFIG:Debug>:WIN32 _DEBUG _WINDOWS _USRDLL>
  $<$<CONFIG:Release>:WIN32 NDEBUG _WINDOWS _USRDLL>
)

target_compile_options(ParticleEditor PRIVATE
  $<$<C_COMPILER_ID:MSVC>:
    # Debug => /Od /MTd 
    $<$<CONFIG:Debug>:/Od /MTd /RTC1 /Zi>

    # Release => /O2 /Ob2 /MT
    $<$<CONFIG:Release>:/O2 /Ob2 /MT /GF /Gy>
  >
)

#------------------------------------------------------------------------------
# 5) Additional include directories (both Debug & Release share them):
#   "./include;./res;../../libraries/include;../../gameengine/include;../../gameenginedevice/include/;../../libraries/source/wwvegas;../../libraries/source/wwvegas/ww3d2;../../libraries/source/wwvegas/wwdebug;../../libraries/source/wwvegas/wwlib;../../libraries/source/wwvegas/wwmath;../../libraries/source/wwvegas/wwsaveload"
#------------------------------------------------------------------------------
target_include_directories(ParticleEditor PRIVATE
    ./include
    ./res
    ../../libraries/include
    ../../gameengine/include
    ../../gameenginedevice/include
    ../../libraries/source/wwvegas
    ../../libraries/source/wwvegas/ww3d2
    ../../libraries/source/wwvegas/wwdebug
    ../../libraries/source/wwvegas/wwlib
    ../../libraries/source/wwvegas/wwmath
    ../../libraries/source/wwvegas/wwsaveload
)

#------------------------------------------------------------------------------
# 6) Resource compile definitions
#   Debug => _DEBUG, Release => NDEBUG
#------------------------------------------------------------------------------
if(MSVC)
  target_compile_definitions(ParticleEditor PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
  )
endif()

#------------------------------------------------------------------------------
# 7) Output DLL naming:
#   Debug => "../../../Run/ParticleEditorD_x64.dll" with import lib => ".\Debug/ParticleEditorD.lib"
#   Release => "../../../Run/ParticleEditor_x64.dll" with import lib => ".\Release/ParticleEditor.lib"
#------------------------------------------------------------------------------
set_target_properties(ParticleEditor PROPERTIES

  # DLL location
  RUNTIME_OUTPUT_DIRECTORY_Debug    "${CMAKE_CURRENT_SOURCE_DIR}/../../../Run"
  RUNTIME_OUTPUT_DIRECTORY_Release  "${CMAKE_CURRENT_SOURCE_DIR}/../../../Run"

  OUTPUT_NAME_Debug    "ParticleEditorD_x64"
  OUTPUT_NAME_Release  "ParticleEditor_x64"

  # Import .lib location
  ARCHIVE_OUTPUT_DIRECTORY_Debug    "${CMAKE_CURRENT_SOURCE_DIR}/Debug"
  ARCHIVE_OUTPUT_DIRECTORY_Release  "${CMAKE_CURRENT_SOURCE_DIR}/Release"

  OUTPUT_NAME_DEBUG    "ParticleEditorD"
  OUTPUT_NAME_RELEASE  "ParticleEditor"
)

#------------------------------------------------------------------------------
# 8) Link flags:
#   The .vcxproj sets SubSystem=Windows => /SUBSYSTEM:WINDOWS
#   RandomizedBaseAddress=false => /DYNAMICBASE:NO
#   We replicate those with link options.
#------------------------------------------------------------------------------
target_link_options(ParticleEditor PRIVATE
  $<$<CONFIG:Debug>:/SUBSYSTEM:WINDOWS /DYNAMICBASE:NO /MAP>
  $<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS /DYNAMICBASE:NO /MAP>
)

message(STATUS "ParticleEditor MFC CMake conversion loaded. Configs = ${CMAKE_CONFIGURATION_TYPES}")
