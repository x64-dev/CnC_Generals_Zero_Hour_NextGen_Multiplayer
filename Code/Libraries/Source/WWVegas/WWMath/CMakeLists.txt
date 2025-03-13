cmake_minimum_required(VERSION 3.15)

project(WWMath LANGUAGES CXX)

# We want these 4 configurations in Visual Studio or other multi-config generators:
set(CMAKE_CONFIGURATION_TYPES "Debug;Internal;Profile;Release" CACHE STRING "Configs" FORCE)

# 1) Create a STATIC library target:
add_library(wwmath STATIC
    # All the .cpp files from the <ItemGroup> in the .vcxproj:
    aabox.cpp
    aabtreecull.cpp
    cardinalspline.cpp
    catmullromspline.cpp
    colmath.cpp
    colmathaabox.cpp
    colmathaabtri.cpp
    colmathfrustum.cpp
    colmathline.cpp
    colmathobbobb.cpp
    colmathobbox.cpp
    colmathobbtri.cpp
    colmathplane.cpp
    colmathsphere.cpp
    cullsys.cpp
    curve.cpp
    euler.cpp
    frustum.cpp
    gridcull.cpp
    hermitespline.cpp
    lineseg.cpp
    lookuptable.cpp
    matrix3.cpp
    matrix3d.cpp
    matrix4.cpp
    obbox.cpp
    ode.cpp
    pot.cpp
    quat.cpp
    tcbspline.cpp
    tri.cpp
    v3_rnd.cpp
    vehiclecurve.cpp
    vp.cpp
    wwmath.cpp

    # If you want the headers visible in IDEs, you can list them too:
    # aabox.h
    # aabtreecull.h
    # ...
    # wwmath.h
    # wwmathids.h
)

# 2) Match the distinct .lib output filenames for each configuration:
set_target_properties(wwmath PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Lib"

    # And specify the actual .lib names:
    OUTPUT_NAME_DEBUG    "WWMathDebug_x64"
    OUTPUT_NAME_INTERNAL "WWMathInternal_x64"
    OUTPUT_NAME_PROFILE  "WWMathProfile"
    OUTPUT_NAME_RELEASE  "WWMath"
)

# 3) Include directories (common to all configs):
target_include_directories(wwmath PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../DX90SDK/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../wwlib"
    "${CMAKE_CURRENT_SOURCE_DIR}/../wwdebug"
    "${CMAKE_CURRENT_SOURCE_DIR}/../wwsaveload"
)

# 4) Preprocessor definitions for each config, from the <PreprocessorDefinitions> in .vcxproj:
target_compile_definitions(wwmath PRIVATE

    # Debug:
    $<$<CONFIG:Debug>:G_CODE_BASE>
    $<$<CONFIG:Debug>:DIRECTX>
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Debug>:WINVER=0x400>
    $<$<CONFIG:Debug>:_WINDOWS>
    $<$<CONFIG:Debug>:WIN32_LEAN_AND_MEAN>

    # Internal:
    $<$<CONFIG:Internal>:NDEBUG>
    $<$<CONFIG:Internal>:WINVER=0x400>
    $<$<CONFIG:Internal>:_WINDOWS>
    $<$<CONFIG:Internal>:WIN32_LEAN_AND_MEAN>
    $<$<CONFIG:Internal>:_INTERNAL>
    $<$<CONFIG:Internal>:_DISABLE_STRING_ANNOTATION>
    $<$<CONFIG:Internal>:_DISABLE_VECTOR_ANNOTATION>

    # Profile:
    $<$<CONFIG:Profile>:NDEBUG>
    $<$<CONFIG:Profile>:WWDEBUG>
    $<$<CONFIG:Profile>:WINVER=0x400>
    $<$<CONFIG:Profile>:_WINDOWS>
    $<$<CONFIG:Profile>:WIN32_LEAN_AND_MEAN>
    $<$<CONFIG:Profile>:WIN32>

    # Release:
    $<$<CONFIG:Release>:WINVER=0x400>
    $<$<CONFIG:Release>:_WINDOWS>
    $<$<CONFIG:Release>:WIN32_LEAN_AND_MEAN>
    $<$<CONFIG:Release>:NDEBUG>
    $<$<CONFIG:Release>:WIN32>
    $<$<CONFIG:Release>:IG_DEBUG_STACKTRACE>
)

