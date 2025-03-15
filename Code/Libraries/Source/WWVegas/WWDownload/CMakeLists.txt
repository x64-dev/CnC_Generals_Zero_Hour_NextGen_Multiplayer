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

target_include_directories(WWDownload PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
)

target_compile_definitions(WWDownload PRIVATE
    ${GNG_COMPILE_DEFINITIONS}
)

target_compile_options(WWDownload PRIVATE
    ${GNG_COMPILE_OPTIONS}
)

target_link_options(WWDownload PRIVATE
    ${GNG_LINK_OPTIONS}
)
