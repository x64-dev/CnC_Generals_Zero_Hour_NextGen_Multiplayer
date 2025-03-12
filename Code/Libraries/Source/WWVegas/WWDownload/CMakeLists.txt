cmake_minimum_required(VERSION 3.15)

project(WWDownload LANGUAGES CXX)

# 1) We have three configs: Debug, Internal, Release
set(CMAKE_CONFIGURATION_TYPES "Debug;Internal;Release" CACHE STRING "Configs" FORCE)

# 2) Create a static library target
add_library(WWDownload STATIC
    Download.cpp
    Ftp.cpp
    registry.cpp
    urlBuilder.cpp

    # If you want headers listed for IDE visibility:
    # Download.h
    # DownloadDebug.h
    # ...
)

# 3) Set output library names and paths for each configuration
set_target_properties(WWDownload PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"

    # The .vcxproj sets:
    # Debug => WWDownloadDebug_x64.lib
    # Internal => WWDownloadInternal_x64.lib
    # Release => WWDownload.lib
    OUTPUT_NAME_DEBUG    "WWDownloadDebug_x64"
    OUTPUT_NAME_INTERNAL "WWDownloadInternal_x64"
    OUTPUT_NAME_RELEASE  "WWDownload"
)

# 4) Include directories
#    The .vcxproj uses ".." as AdditionalIncludeDirectories for all configs
target_include_directories(WWDownload PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
)

# 5) Per-configuration preprocessor definitions
target_compile_definitions(WWDownload PRIVATE

    # Debug => _DEBUG; _LIB
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Debug>:_LIB>

    # Internal => DEBUG; _LIB; _DISABLE_STRING_ANNOTATION; _DISABLE_VECTOR_ANNOTATION
    $<$<CONFIG:Internal>:DEBUG>
    $<$<CONFIG:Internal>:_LIB>
    $<$<CONFIG:Internal>:_DISABLE_STRING_ANNOTATION>
    $<$<CONFIG:Internal>:_DISABLE_VECTOR_ANNOTATION>

    # Release => WIN32; NDEBUG; _LIB
    $<$<CONFIG:Release>:WIN32>
    $<$<CONFIG:Release>:NDEBUG>
    $<$<CONFIG:Release>:_LIB>
)

# 6) Typical MSVC compile flags for each config
if(MSVC)
    target_compile_options(WWDownload PRIVATE

        # Debug => /Od + /MDd + basic runtime checks (/RTC1 or /RTCs)
        $<$<CONFIG:Debug>:/Od>
        $<$<CONFIG:Debug>:/MDd>
        $<$<CONFIG:Debug>:/RTC1>

        # Internal => /O2 + /Ob1 + /MD
        #  (MaxSpeed => /O2, OnlyExplicitInline => /Ob1, MultiThreadedDLL => /MD)
        $<$<CONFIG:Internal>:/O2>
        $<$<CONFIG:Internal>:/Ob1>
        $<$<CONFIG:Internal>:/MD>

        # Release => /O2 + /Ob2 + /MD
        #  (MaxSpeed => /O2, AnySuitable => /Ob2, MultiThreadedDLL => /MD)
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:Release>:/Ob2>
        $<$<CONFIG:Release>:/MD>

        # Common flags (String Pooling => /GF if you want, WarningLevel => /W3, etc.)
        /W3
        /Gy    # function-level linking
    )
endif()

#
# Done! This should provide a static library with three configurations and
# distinct .lib names, matching the principal settings in your .vcxproj.
#
