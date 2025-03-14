add_library(DebugWindow SHARED
    DebugWindow.cpp
    DebugWindowDialog.cpp
    DebugWindow.rc       # Resource file
    DebugWindow.def      # Exports symbol table
    StdAfx.cpp           # Creates the PCH
)

#------------------------------------------------------------------------------
# 1) MFC usage in a static library => we set MFC_FLAG=1
#------------------------------------------------------------------------------
set_property(TARGET DebugWindow PROPERTY MFC_FLAG 1)

target_precompile_headers(DebugWindow PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:stdafx.h>
)

#------------------------------------------------------------------------------
# 3) We are using MFC statically, but the runtime library is also static
#    (/MTd for Debug, /MT for Release). We also see "MultiByte" => define _MBCS.
#------------------------------------------------------------------------------
target_compile_definitions(DebugWindow PRIVATE
  _MBCS
  WIN32
  _WINDOWS
  _USRDLL

  $<$<CONFIG:Debug>:_DEBUG>

  $<$<CONFIG:Release>:NDEBUG>
)

target_compile_options(DebugWindow PRIVATE
  $<$<C_COMPILER_ID:MSVC>:
    # Debug
    $<$<CONFIG:Debug>:
      /Od           # Disabled optimization
      /MTd          # Static runtime, debug
      /Zi           # Debug info
    >
    # Release
    $<$<CONFIG:Release>:
      /O2           # MaxSpeed optimization
      /Ob1          # "OnlyExplicitInline"
      /MT           # Static runtime
      /GF /Gy       # (Optional) string pooling & function-level linking
    >
  >
)

target_link_options(DebugWindow PRIVATE
  /SUBSYSTEM:WINDOWS
  /DYNAMICBASE:NO
  /MAP
)
