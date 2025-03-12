cmake_minimum_required(VERSION 3.15)

project(WWLib LANGUAGES CXX)

# We want 4 configurations in Visual Studio (or other multi-config generator):
set(CMAKE_CONFIGURATION_TYPES "Debug;Internal;Profile;Release" CACHE STRING "Configs" FORCE)

# 1) Create a STATIC library target:
add_library(wwlib STATIC
    # Source files from <ClCompile>:
    b64pipe.cpp
    b64straw.cpp
    base64.cpp
    blit.cpp
    blowfish.cpp
    buff.cpp
    bufffile.cpp
    chunkio.cpp
    convert.cpp
    cpudetect.cpp
    crc.cpp
    cstraw.cpp
    data.cpp
    ddraw.cpp
    dsurface.cpp
    ffactory.cpp
    gcd_lcm.cpp
    hash.cpp
    hsv.cpp
    ini.cpp
    int.cpp
    jshell.cpp
    keyboard.cpp
    LaunchWeb.cpp
    lcw.cpp
    load.cpp
    mono.cpp
    mpmath.cpp
    mpu.cpp
    msgloop.cpp
    multilist.cpp
    mutex.cpp
    nstrdup.cpp
    palette.cpp
    pcx.cpp
    pipe.cpp
    pk.cpp
    ramfile.cpp
    random.cpp
    rawfile.cpp
    rc4.cpp
    rcfile.cpp
    readline.cpp
    realcrc.cpp
    refcount.cpp
    registry.cpp
    rgb.cpp
    rle.cpp
    rndstrng.cpp
    sampler.cpp
    sha.cpp
    slnode.cpp
    srandom.cpp
    stimer.cpp
    straw.cpp
    strtok_r.cpp
    systimer.cpp
    tagblock.cpp
    TARGA.cpp
    textfile.cpp
    thread.cpp
    trim.cpp
    vector.cpp
    widestring.cpp
    win.cpp
    WWCOMUtil.cpp
    wwfile.cpp
    wwfont.cpp
    wwstring.cpp
    xpipe.cpp
    xstraw.cpp
    xsurface.cpp
    _convert.cpp
    _mono.cpp

    # (Optional) If you want headers visible in IDEs:
    # always.h
    # argv.h
    # b64pipe.h
    # ...
    # xsurface.h
    # _mono.h
)

# 2) Match the distinct .lib output filenames for each config:
set_target_properties(wwlib PROPERTIES
    # Put all libs in a "Lib" folder under your build directory:
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Lib"

    # Library name for each config:
    OUTPUT_NAME_DEBUG    "WWLibdebug_x64"
    OUTPUT_NAME_INTERNAL "WWLibInternal_x64"
    OUTPUT_NAME_PROFILE  "WWLibProfile"
    OUTPUT_NAME_RELEASE  "WWLib"
)

# 3) Common include dirs (the .vcxproj had "..\wwdebug" + anything else you might need):
target_include_directories(wwlib PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../wwdebug"
)

# 4) Preprocessor definitions for each config, from <PreprocessorDefinitions> in .vcxproj:
target_compile_definitions(wwlib PRIVATE

    # Debug:
    $<$<CONFIG:Debug>:__PLACEMENT_VEC_NEW_INLINE>
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Debug>:WINVER=0x400>
    $<$<CONFIG:Debug>:_WINDOWS>
    $<$<CONFIG:Debug>:WIN32_LEAN_AND_MEAN>
    $<$<CONFIG:Debug>:REGEX_MALLOC>
    $<$<CONFIG:Debug>:STDC_HEADERS>

    # Release:
    $<$<CONFIG:Release>:_DISABLE_STRING_ANNOTATION>
    $<$<CONFIG:Release>:_DISABLE_VECTOR_ANNOTATION>
    $<$<CONFIG:Release>:__PLACEMENT_VEC_NEW_INLINE>
    $<$<CONFIG:Release>:WINVER=0x400>
    $<$<CONFIG:Release>:_WINDOWS>
    $<$<CONFIG:Release>:WIN32_LEAN_AND_MEAN>
    $<$<CONFIG:Release>:REGEX_MALLOC>
    $<$<CONFIG:Release>:STDC_HEADERS>
    $<$<CONFIG:Release>:NDEBUG>
    $<$<CONFIG:Release>:IG_DEBUG_STACKTRACE>

    # Internal:
    $<$<CONFIG:Internal>:_DISABLE_STRING_ANNOTATION>
    $<$<CONFIG:Internal>:_DISABLE_VECTOR_ANNOTATION>
    $<$<CONFIG:Internal>:__PLACEMENT_VEC_NEW_INLINE>
    $<$<CONFIG:Internal>:NDEBUG>
    $<$<CONFIG:Internal>:WINVER=0x400>
    $<$<CONFIG:Internal>:_WINDOWS>
    $<$<CONFIG:Internal>:WIN32_LEAN_AND_MEAN>
    $<$<CONFIG:Internal>:REGEX_MALLOC>
    $<$<CONFIG:Internal>:STDC_HEADERS>
    $<$<CONFIG:Internal>:_INTERNAL>

    # Profile:
    $<$<CONFIG:Profile>:__PLACEMENT_VEC_NEW_INLINE>
    $<$<CONFIG:Profile>:NDEBUG>
    $<$<CONFIG:Profile>:WWDEBUG>
    $<$<CONFIG:Profile>:WINVER=0x400>
    $<$<CONFIG:Profile>:_WINDOWS>
    $<$<CONFIG:Profile>:WIN32_LEAN_AND_MEAN>
    $<$<CONFIG:Profile>:WIN32>
    $<$<CONFIG:Profile>:REGEX_MALLOC>
    $<$<CONFIG:Profile>:STDC_HEADERS>
)

