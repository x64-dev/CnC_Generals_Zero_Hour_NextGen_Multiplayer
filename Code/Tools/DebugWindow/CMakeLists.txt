add_library(DebugWindow SHARED
    DebugWindow.cpp
    DebugWindowDialog.cpp
    DebugWindow.rc       # Resource file
    DebugWindow.def      # Exports symbol table
    StdAfx.cpp           # Creates the PCH
)

set_property(TARGET DebugWindow PROPERTY MFC_FLAG 1)

target_precompile_headers(DebugWindow PRIVATE
    stdafx.h
)

target_compile_definitions(DebugWindow PRIVATE
    ${GNG_COMPILE_DEFINITIONS}
)

target_compile_options(DebugWindow PRIVATE
    ${GNG_COMPILE_OPTIONS}
)

target_link_options(DebugWindow PRIVATE
    ${GNG_LINK_OPTIONS}
)
