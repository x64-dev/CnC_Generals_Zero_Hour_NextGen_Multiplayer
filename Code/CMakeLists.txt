cmake_minimum_required(VERSION 3.15)

project(RTS LANGUAGES C CXX)

# Force multi-config for Visual Studio (Debug, Internal, Release)
set(CMAKE_CONFIGURATION_TYPES "Debug;Internal;Release" CACHE STRING "Configs" FORCE)

set(CMAKE_EXE_LINKER_FLAGS_DEBUGE "")
set(CMAKE_EXE_LINKER_FLAGS_INTERNAL "")
set(CMAKE_EXE_LINKER_FLAGS_INTERNALE "")
set(CMAKE_EXE_LINKER_FLAGS_PROFILE "")
set(CMAKE_EXE_LINKER_FLAGS_PROFILEE "")

add_subdirectory(GameEngine)
add_subdirectory(GameEngineDevice)
add_subdirectory(GameRenderer)
add_subdirectory(Tools/WorldBuilder)
add_subdirectory(Libraries/Source/Compression)
add_subdirectory(Libraries/Source/WWVegas/ww3d2)
add_subdirectory(Libraries/Source/WWVegas/WWDebug)
add_subdirectory(Libraries/Source/WWVegas/WWDownload)
add_subdirectory(Libraries/Source/WWVegas/WWLib)
add_subdirectory(Libraries/Source/WWVegas/WWMath)
add_subdirectory(Libraries/Source/WWVegas/WWSaveLoad)
 
# Add the main executable; WIN32 for the Windows subsystem (no console).
add_executable(RTS WIN32
    # Source files
    Libraries/Bink/include/bink.c
    Libraries/Bink/include/cleanup.c
    Libraries/Bink/include/miles.c
    Main/WinMain.cpp
    Main/RTS.rc

    # (Optional) If you want to see these headers in the project tree:
    # Main/resource.h
    # Main/WinMain.h
    # etc.
)

#-----------------------------------------------------------------------------------
# 1) Output name & directory per configuration
#-----------------------------------------------------------------------------------
set_target_properties(RTS PROPERTIES
    # Put all .exe files in a folder called "Run"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../run"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../run"
    RUNTIME_OUTPUT_DIRECTORY_INTERNAL "${CMAKE_CURRENT_SOURCE_DIR}/../run"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../run"

    # Each config has its own output name:
    OUTPUT_NAME_DEBUG "Generals_debug"      # Debug
    OUTPUT_NAME_INTERNAL "Generals_internal"   # Internal
    OUTPUT_NAME_RELEASE "Generals"     # Release
)

#-----------------------------------------------------------------------------------
# 2) Include paths
#    You had the *same* AdditionalIncludeDirectories across all configs, so we
#    can just list them once. If you do want them different per config, you can
#    wrap them in generator expressions ($<$<CONFIG:Debug>:...>).
#-----------------------------------------------------------------------------------
target_include_directories(RTS PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/GameRenderer"
    "${CMAKE_CURRENT_SOURCE_DIR}/Libraries/Bink/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/Libraries/DX90SDK/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/Libraries/Source/WWVegas/Miles6/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/Libraries/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/GameEngine/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/GameEngineDevice/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/Libraries/Source/WWVegas"
    "${CMAKE_CURRENT_SOURCE_DIR}/Libraries/Source/WWVegas/WWLib"
    "${CMAKE_CURRENT_SOURCE_DIR}/Libraries/Source/WWVegas/WWMath"
    "${CMAKE_CURRENT_SOURCE_DIR}/Libraries/Source/WWVegas/WWDebug"
    "${CMAKE_CURRENT_SOURCE_DIR}/Libraries/Source/WWVegas/WWSaveLoad"
    "${CMAKE_CURRENT_SOURCE_DIR}/Libraries/Source/WWVegas/WW3D2"
    "${CMAKE_CURRENT_SOURCE_DIR}/Libraries/Include/Granny"
)

#-----------------------------------------------------------------------------------
# 3) Preprocessor definitions per configuration
#
#    From the .vcxproj:
#    - Debug:
#      __PLACEMENT_VEC_NEW_INLINE;_DEBUG;_WINDOWS;BROWSER_DEBUG
#    - Release:
#      _DISABLE_STRING_ANNOTATION;_DISABLE_VECTOR_ANNOTATION;
#      __PLACEMENT_VEC_NEW_INLINE;IG_DEBUG_STACKTRACE;
#      NDEBUG;_RELEASE;WIN32;_WINDOWS
#    - Internal:
#      _DISABLE_STRING_ANNOTATION;_DISABLE_VECTOR_ANNOTATION;
#      __PLACEMENT_VEC_NEW_INLINE;NDEBUG;_INTERNAL;_WINDOWS
#-----------------------------------------------------------------------------------
target_compile_definitions(RTS PRIVATE
    # For Debug:
    $<$<CONFIG:Debug>:__PLACEMENT_VEC_NEW_INLINE>
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Debug>:_WINDOWS>
    $<$<CONFIG:Debug>:BROWSER_DEBUG>

    # For Release:
    $<$<CONFIG:Release>:_DISABLE_STRING_ANNOTATION>
    $<$<CONFIG:Release>:_DISABLE_VECTOR_ANNOTATION>
    $<$<CONFIG:Release>:__PLACEMENT_VEC_NEW_INLINE>
    $<$<CONFIG:Release>:IG_DEBUG_STACKTRACE>
    $<$<CONFIG:Release>:NDEBUG>
    $<$<CONFIG:Release>:_RELEASE>
    $<$<CONFIG:Release>:WIN32>
    $<$<CONFIG:Release>:_WINDOWS>

    # For Internal:
    $<$<CONFIG:Internal>:_DISABLE_STRING_ANNOTATION>
    $<$<CONFIG:Internal>:_DISABLE_VECTOR_ANNOTATION>
    $<$<CONFIG:Internal>:__PLACEMENT_VEC_NEW_INLINE>
    $<$<CONFIG:Internal>:NDEBUG>
    $<$<CONFIG:Internal>:_INTERNAL>
    $<$<CONFIG:Internal>:_WINDOWS>
)

#-----------------------------------------------------------------------------------
# 4) Example compiler flags for MSVC
#-----------------------------------------------------------------------------------
if(MSVC)
    target_compile_options(RTS PRIVATE
        # Debug config: no optimization, debug runtime
        $<$<CONFIG:Debug>:/Od>
        $<$<CONFIG:Debug>:/MDd>

        # Release config: optimize, release runtime
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:Release>:/MD>

        # Internal config: often “RelWithDebInfo”-style
        $<$<CONFIG:Internal>:/O2>
        $<$<CONFIG:Internal>:/MD>

        # Common warnings level
        /W3
    )
endif()

#-----------------------------------------------------------------------------------
# 5) Link libraries
#    Use generator expressions again for per-config .lib differences.
#    (Below is just an example pattern for some of the original libs.)
#-----------------------------------------------------------------------------------
target_link_libraries(RTS PRIVATE
    # Common Windows libs
    user32
    gdi32
    shell32
    # (Add more system libs if needed)

    # Common DirectX / OpenAL libs
    d3d9
    d3d9on12
    d3d12
    Shcore
    openal32
    dxguid
    dinput8
    odbc32
    odbccp32
    d3dx9
    dxerr
    winmm
    vfw32
    dsound
    wsock32
    imm32
    wininet

    #Gamespy Stuff
    brigades/RelWithDebInfo/usbrigades
    Chat/RelWithDebInfo/uschat
    common/RelWithDebInfo/uscommon
    Direct2Game/RelWithDebInfo/usd2g
    gcdkey/RelWithDebInfo/uscdkey
    ghttp/RelWithDebInfo/ushttp
    GP/RelWithDebInfo/usgp
    gstats/RelWithDebInfo/usstats
    gt2/RelWithDebInfo/usgt2
    natneg/RelWithDebInfo/usnatneg
    Peer/RelWithDebInfo/uspeer
    pinger/RelWithDebInfo/uspinger
    pt/RelWithDebInfo/uspt
    qr2/RelWithDebInfo/usqr2
    sake/RelWithDebInfo/ussake
    sc/RelWithDebInfo/ussc
    serverbrowsing/RelWithDebInfo/usserverbrowsing
    sharedDll/RelWithDebInfo/UniSpySDK
    Voice2/RelWithDebInfo/usvoice2
    Voice2/libgsm/RelWithDebInfo/gsm
    Voice2/libspeex/RelWithDebInfo/speex
    webservices/RelWithDebInfo/uswebservice

    GameEngine
    GameEngineDevice
    GameRenderer
    Compression
    ww3d2
    WWDebug
    WWDownload
    wwlib
    wwmath
    wwsaveload
)

#-----------------------------------------------------------------------------------
# 6) If you need to replicate the custom build steps (e.g. “versionUpdate.exe”)
#    you can use a PRE_BUILD custom command:
#-----------------------------------------------------------------------------------
# add_custom_command(
#    TARGET RTS
#    PRE_BUILD
#    COMMAND "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/versionUpdate.exe"
#            "${CMAKE_CURRENT_SOURCE_DIR}/Main/generatedVersion.h"
#    COMMAND "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/buildVersionUpdate.exe"
#            "${CMAKE_CURRENT_SOURCE_DIR}/Main/buildVersion.h"
#    COMMENT "Incrementing version numbers..."
# )

#-----------------------------------------------------------------------------------
# 7) If you need library directories beyond the full paths, you can do:
#-----------------------------------------------------------------------------------
target_link_directories(RTS PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Libraries/DX90SDK/Lib/x64
    $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/Libraries/d3d9on12/bin/src/Release>
    $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/Libraries/Source/GameSpy/bin/x64>
    $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/Libraries/OpenAL/bin/x64/RelWithDebInfo>
    # etc.
    $<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/Libraries/d3d9on12/bin/src/Release>
    # ...
    $<$<CONFIG:Internal>:${CMAKE_CURRENT_SOURCE_DIR}/Libraries/d3d9on12/bin/src/Release>
    # ...
)

# Done!
