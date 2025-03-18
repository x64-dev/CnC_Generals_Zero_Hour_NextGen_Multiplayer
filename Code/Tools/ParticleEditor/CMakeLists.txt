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

set_target_properties(ParticleEditor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../../../run/"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../../../run/"
)

target_compile_definitions(ParticleEditor PRIVATE
  WIN32
  _WINDOWS
  _USRDLL
  $<$<CONFIG:Debug>:_DEBUG>
  $<$<CONFIG:Release>:NDEBUG>
)

target_compile_options(ParticleEditor PRIVATE
  $<$<C_COMPILER_ID:MSVC>:
    # Debug => /Od /MTd 
    $<$<CONFIG:Debug>:/Od /MTd /RTC1 /Zi>

    # Release => /O2 /Ob2 /MT
    $<$<CONFIG:Release>:/O2 /Ob2 /MT /GF /Gy>
  >
)

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

target_link_options(ParticleEditor PRIVATE
  /SUBSYSTEM:WINDOWS
  /DYNAMICBASE:NO
  /MAP
)
