cmake_minimum_required(VERSION 3.10)

#
# Project setup
#
project(GameRenderer LANGUAGES CXX)

#
# Source files (from <ClCompile>) and headers (from <ClInclude>)
#
set(GAMERENDERER_SOURCES
    dx12/tinydx.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_impl_dx9.cpp
    imgui/imgui_impl_win32.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    dx8/dx8caps.cpp
    dx8/dx8fvf.cpp
    dx8/dx8indexbuffer.cpp
    dx8/dx8polygonrenderer.cpp
    dx8/dx8renderer.cpp
    dx8/dx8texman.cpp
    dx8/dx8vertexbuffer.cpp
    dx8/dx8webbrowser.cpp
    dx8/dx8wrapper.cpp
)

set(GAMERENDERER_HEADERS
    dx12/tinydx.h
    dx8/dx8caps.h
    dx8/dx8fvf.h
    dx8/dx8indexbuffer.h
    dx8/dx8list.h
    dx8/dx8polygonrenderer.h
    dx8/dx8renderer.h
    dx8/dx8texman.h
    dx8/dx8vertexbuffer.h
    dx8/dx8webbrowser.h
    dx8/dx8wrapper.h
    GameRenderer.h
)

#
# Declare a static library target
#
add_library(GameRenderer STATIC
    ${GAMERENDERER_SOURCES}
    ${GAMERENDERER_HEADERS}
)

#
# Common include paths
#   (These appear to be the same for Debug, Release, and Internal, 
#    except for small differences—adjust if needed.)
#
target_include_directories(GameRenderer PRIVATE
    .
    # From the AdditionalIncludeDirectories in Release|x64, Internal|x64, and Debug|x64:
    ../Libraries/Source/WWVegas/WWSaveLoad
    ../Libraries/Source/WWVegas/WWDebug
    ../Libraries/Source/WWVegas/WWMath
    ../Libraries/Source/WWVegas/WW3D2
    ../Libraries/DX90SDK/Include
    ../Libraries/Source/WWVegas
    ../Libraries/Source/WWVegas/WWLib
    ../Libraries/Source/Compression
    Include/Precompiled
    ../Libraries/Source/
    Include
    ../Libraries/Include
    ../Libraries/Source/GameSpy
)

#
# Preprocessor definitions for each configuration
#   Using generator expressions so they only apply to the matching config.
#

# == Release ==
target_compile_definitions(GameRenderer PRIVATE
    $<$<CONFIG:Release>:__PLACEMENT_VEC_NEW_INLINE>
    $<$<CONFIG:Release>:_LIB>
    $<$<CONFIG:Release>:_WINDOWS>
    $<$<CONFIG:Release>:WIN32_LEAN_AND_MEAN>
    $<$<CONFIG:Release>:NDEBUG>
    $<$<CONFIG:Release>:IG_DEBUG_STACKTRACE>
)

# == Debug ==
target_compile_definitions(GameRenderer PRIVATE
    $<$<CONFIG:Debug>:__PLACEMENT_VEC_NEW_INLINE>
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Debug>:_LIB>
    $<$<CONFIG:Debug>:_WINDOWS>
    $<$<CONFIG:Debug>:WIN32_LEAN_AND_MEAN>
)

# == Internal ==
target_compile_definitions(GameRenderer PRIVATE
    $<$<CONFIG:Internal>:__PLACEMENT_VEC_NEW_INLINE>
    $<$<CONFIG:Internal>:NDEBUG>
    $<$<CONFIG:Internal>:_LIB>
    $<$<CONFIG:Internal>:_WINDOWS>
    $<$<CONFIG:Internal>:WIN32_LEAN_AND_MEAN>
    $<$<CONFIG:Internal>:_INTERNAL>
    $<$<CONFIG:Internal>:_DISABLE_STRING_ANNOTATION>
    $<$<CONFIG:Internal>:_DISABLE_VECTOR_ANNOTATION>
)

#
# If you want to match the /MD, /MDd, etc. runtime library (and other flags):
#   (Note: This is one way. Alternatively, you can set properties like 
#         MSVC_RUNTIME_LIBRARY in CMake 3.15+ if you prefer.)
#
if(MSVC)
    target_compile_options(GameRenderer PRIVATE
        # Common flags (e.g. /W3 /nologo)
        /W3
        /nologo

        # Debug: /MDd + /Zi
        $<$<CONFIG:Debug>:/MDd>
        $<$<CONFIG:Debug>:/Zi>
        $<$<CONFIG:Debug>:/Ob1>        # "Any Suitable" or "Only Explicit" inline expansions, etc.

        # Release: /MD + /Zi or /O2, etc.
        $<$<CONFIG:Release>:/MD>
        $<$<CONFIG:Release>:/Zi>
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:Release>:/Ob2>      # "Any Suitable"

        # Internal: /MD + /O2 + /Zi, etc.
        $<$<CONFIG:Internal>:/MD>
        $<$<CONFIG:Internal>:/Zi>
        $<$<CONFIG:Internal>:/O2>
        $<$<CONFIG:Internal>:/Ob1>     # "Only Explicit Inline," for example
    )
endif()

#
# You can optionally configure where to output the .lib if you wish:
#   (CMake by default puts static libraries in <build-dir>/<config>/)
#
#set_target_properties(GameRenderer PROPERTIES
#    ARCHIVE_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/Debug"
#    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release"
#    ARCHIVE_OUTPUT_DIRECTORY_INTERNAL "${CMAKE_BINARY_DIR}/Internal"
#)

