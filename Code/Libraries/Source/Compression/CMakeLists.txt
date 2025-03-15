add_library(Compression STATIC
    # EAC
    EAC/btreeabout.cpp
    EAC/btreedecode.cpp
    EAC/btreeencode.cpp
    EAC/huffabout.cpp
    EAC/huffdecode.cpp
    EAC/huffencode.cpp
    EAC/refabout.cpp
    EAC/refdecode.cpp
    EAC/refencode.cpp

    # LZHCompress
    LZHCompress/CompLibSource/Huff.cpp
    LZHCompress/CompLibSource/Lz.cpp
    LZHCompress/CompLibSource/Lzhl.cpp
    LZHCompress/CompLibSource/Lzhl_tcp.cpp
    LZHCompress/NoxCompress.cpp

    # ZLib
    ZLib/adler32.c
    ZLib/compress.c
    ZLib/crc32.c
    ZLib/deflate.c
    ZLib/gzio.c
    ZLib/infblock.c
    ZLib/infcodes.c
    ZLib/inffast.c
    ZLib/inflate.c
    ZLib/inftrees.c
    ZLib/infutil.c
    ZLib/maketree.c
    ZLib/trees.c
    ZLib/uncompr.c
    ZLib/zutil.c

    # Top-level
    CompressionManager.cpp
)

target_include_directories(Compression PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

target_compile_definitions(Compression PRIVATE
    ${GNG_COMPILE_DEFINITIONS}
)

target_compile_options(Compression PRIVATE
    ${GNG_COMPILE_OPTIONS}
)

target_link_options(Compression PRIVATE
    ${GNG_LINK_OPTIONS}
)
