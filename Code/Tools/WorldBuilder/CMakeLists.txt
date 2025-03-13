cmake_minimum_required(VERSION 3.20)

#------------------------------------------------------------------------------
# Project name "WorldBuilder" – an MFC application
#------------------------------------------------------------------------------
project(WorldBuilder LANGUAGES CXX)

#------------------------------------------------------------------------------
# We have three configurations from the vcxproj: Debug, Internal, Release.
#------------------------------------------------------------------------------
set(CMAKE_CONFIGURATION_TYPES Debug Internal Release CACHE STRING "Configs" FORCE)

#------------------------------------------------------------------------------
# 1) Create an executable target named "WorldBuilder"
#    We'll replicate the .cpp sources from the <ItemGroup><ClCompile>.
#------------------------------------------------------------------------------
add_executable(WorldBuilder WIN32
    # Resource:
    res/WorldBuilder.rc

    # From "src\..." blocks
    src/addplayerdialog.cpp
    src/AutoEdgeOutTool.cpp
    src/BaseBuildProps.cpp
    src/BlendEdgeTool.cpp
    src/BlendMaterial.cpp
    src/BorderTool.cpp
    src/brushoptions.cpp
    src/BrushTool.cpp
    src/BuildList.cpp
    src/BuildListTool.cpp
    src/CameraOptions.cpp
    src/CButtonShowColor.cpp
    src/CellWidth.cpp
    src/CFixTeamOwnerDialog.cpp
    src/ContourOptions.cpp
    src/CUndoable.cpp
    src/DrawObject.cpp
    src/EditAction.cpp
    src/EditCondition.cpp
    src/EditCoordParameter.cpp
    src/EditGroup.cpp
    src/EditObjectParameter.cpp
    src/EditParameter.cpp
    src/EulaDialog.cpp
    src/ExportScriptsOptions.cpp
    src/EyedropperTool.cpp
    src/FeatherOptions.cpp
    src/FeatherTool.cpp
    src/FenceOptions.cpp
    src/FenceTool.cpp
    src/FloodFillTool.cpp
    src/GlobalLightOptions.cpp
    src/GroveOptions.cpp
    src/GroveTool.cpp
    src/HandScrollTool.cpp
    src/ImpassableOptions.cpp
    src/LayersList.cpp
    src/LightOptions.cpp
    src/MainFrm.cpp
    src/mapobjectprops.cpp
    src/MapPreview.cpp
    src/MapSettings.cpp
    src/MeshMoldOptions.cpp
    src/MeshMoldTool.cpp
    src/MoundOptions.cpp
    src/MoundTool.cpp
    src/MyToolbar.cpp
    src/NewHeightMap.cpp
    src/ObjectOptions.cpp
    src/ObjectPreview.cpp
    src/ObjectTool.cpp
    src/OpenMap.cpp
    src/OptionsPanel.cpp
    src/PickUnitDialog.cpp
    src/playerlistdlg.cpp
    src/PointerTool.cpp
    src/PolygonTool.cpp
    src/propedit.cpp
    src/RampOptions.cpp
    src/RampTool.cpp
    src/RoadOptions.cpp
    src/RoadTool.cpp
    src/SaveMap.cpp
    src/ScorchOptions.cpp
    src/ScorchTool.cpp
    src/ScriptActionsFalse.cpp
    src/ScriptActionsTrue.cpp
    src/ScriptConditions.cpp
    src/ScriptDialog.cpp
    src/ScriptProperties.cpp
    src/SelectMacrotexture.cpp
    src/ShadowOptions.cpp
    src/SplashScreen.cpp
    src/StdAfx.cpp
    src/TeamBehavior.cpp
    src/TeamGeneric.cpp
    src/TeamIdentity.cpp
    src/TeamReinforcement.cpp
    src/teamsdialog.cpp
    src/TerrainMaterial.cpp
    src/TerrainModal.cpp
    src/TerrainSwatches.cpp
    src/TileTool.cpp
    src/Tool.cpp
    src/WaterOptions.cpp
    src/WaterTool.cpp
    src/WaypointOptions.cpp
    src/WaypointTool.cpp
    src/WBFrameWnd.cpp
    src/WBHeightMap.cpp
    src/WBPopupSlider.cpp
    src/wbview.cpp
    src/wbview3d.cpp
    src/WHeightMapEdit.cpp
    src/WorldBuilder.cpp
    src/WorldBuilderDoc.cpp
    src/WorldBuilderView.cpp
)

#------------------------------------------------------------------------------
# 2) Set MFC usage to "Use MFC in a Shared DLL" (i.e., dynamic MFC).
#    Also set MultiByte: typically implies '_MBCS'.
#------------------------------------------------------------------------------
set_property(TARGET WorldBuilder PROPERTY
    # MFC_FLAG = 2 => Use MFC in a Shared DLL
    MFC_FLAG 2
)
# Also define '_AFXDLL' (for dynamic MFC) and '_MBCS' (since CharacterSet=MultiByte).
target_compile_definitions(WorldBuilder PRIVATE
    $<$<CONFIG:Debug>:_AFXDLL _MBCS>
    $<$<CONFIG:Internal>:_AFXDLL _MBCS>
    $<$<CONFIG:Release>:_AFXDLL _MBCS>
)

#------------------------------------------------------------------------------
# 3) Configuration-specific include paths:
#------------------------------------------------------------------------------
target_include_directories(WorldBuilder PRIVATE
  # Debug
  $<$<CONFIG:Debug>:
    ../../GameRenderer/
    ../../Libraries/OpenAL/include
    ../../Libraries/Source/WWVegas/Miles6/Include
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
    ../../Libraries/Source/Compression
    # Many .cpp specifically add "..\res", but it's the same for Debug/Internal/Release,
    # so we unify it here
    ../res
  >

  # Internal
  $<$<CONFIG:Internal>:
    ../../GameRenderer/
    ../../Libraries/OpenAL/include
    ../../Libraries/Source/WWVegas/Miles6/Include
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
    ../../Libraries/Source/Compression
    ../res
  >

  # Release
  $<$<CONFIG:Release>:
    ../../GameRenderer/
    ../../Libraries/OpenAL/include
    ../../Libraries/Source/WWVegas/Miles6/Include
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
    ../../Libraries/Source/Compression
    ../res
  >
)

#------------------------------------------------------------------------------
# 4) Configuration-specific preprocessor defs and compile options:
#    The .vcxproj sets different sets for each config. We'll replicate:
#------------------------------------------------------------------------------
target_compile_definitions(WorldBuilder PRIVATE

  # Debug
  $<$<CONFIG:Debug>:
    __PLACEMENT_VEC_NEW_INLINE    
    _DEBUG
    EDITOR
    WIN32
    _WINDOWS
    REGEX_MALLOC
    STDC_HEADERS
    WIN32_LEAN_AND_MEAN
  >

  # Internal
  $<$<CONFIG:Internal>:
    __PLACEMENT_VEC_NEW_INLINE
    _DISABLE_STRING_ANNOTATION
    _DISABLE_VECTOR_ANNOTATION
    NDEBUG
    _INTERNAL
    EDITOR
    WIN32
    _WINDOWS
    REGEX_MALLOC
    STDC_HEADERS
    WIN32_LEAN_AND_MEAN
  >

  # Release
  $<$<CONFIG:Release>:
    __PLACEMENT_VEC_NEW_INLINE
    _DISABLE_STRING_ANNOTATION
    _DISABLE_VECTOR_ANNOTATION
    IG_DEBUG_STACKTRACE
    NDEBUG
    _RELEASE
    EDITOR
    WIN32
    _WINDOWS
    REGEX_MALLOC
    STDC_HEADERS
    WIN32_LEAN_AND_MEAN
  >
)

# Typical MSVC flags for each config:
target_compile_options(WorldBuilder PRIVATE
  $<$<C_COMPILER_ID:MSVC>:

    # Debug => /Od (or /O2 if we exactly replicate <Optimization>Disabled vs "MaxSpeed"), 
    #   but the .vcxproj says <Optimization>Disabled => /Od
    #   plus /MDd for "MultiThreadedDebugDLL"
    $<$<CONFIG:Debug>:/Od /MDd /Zi /RTC1>

    # Internal => <Optimization>MaxSpeed => /O2, <InlineFunctionExpansion>OnlyExplicitInline => /Ob1
    #   plus /MD
    $<$<CONFIG:Internal>:/O2 /Ob1 /MD /Zi /GF /Gy>

    # Release => <Optimization>MaxSpeed => /O2, <InlineFunctionExpansion>AnySuitable => /Ob2
    #   plus /MD
    $<$<CONFIG:Release>:/O2 /Ob2 /MD /GF /Gy>
  >
)

#------------------------------------------------------------------------------
# 5) Resource compilation:
#   The .vcxproj sets <PreprocessorDefinitions>_DEBUG or NDEBUG for each config,
#   plus it includes "res" folder in each config's ResourceCompile steps.
#   We rely on `target_include_directories(...)` for that "res" path.
#   If you need to specifically define Resource preprocessor macros, you can do so similarly:
#------------------------------------------------------------------------------
if(MSVC)
  # Just an example to define resource macros; adapt as needed.
  target_compile_definitions(WorldBuilder PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Internal>:NDEBUG>
    $<$<CONFIG:Release>:NDEBUG>
  )
endif()

#------------------------------------------------------------------------------
# 6) Linking logic: The .vcxproj has big AdditionalDependencies, AdditionalLibraryDirectories, 
#    plus different output exes and ignoring default libs. We'll replicate them with generator expressions.
#------------------------------------------------------------------------------
# a) Link libraries
target_link_libraries(WorldBuilder PRIVATE
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

  $<$<CONFIG:Debug>: 
    d3d12
    openal32
    dxguid
    dinput8
    ddraw
    d3dx9
    d3d9
    dxerr
    winmm
    vfw32
    dsound
    wsock32
    imm32
    wininet
    # GameSpy & sub-libs in "brigades\Debug\usbrigades", etc. 
    # Typically you'd do absolute or relative paths; here we list them as placeholders:
    brigades/Debug/usbrigades
    Chat/Debug/uschat
    common/Debug/uscommon
    Direct2Game/Debug/usd2g
    gcdkey/Debug/uscdkey
    ghttp/Debug/ushttp
    GP/Debug/usgp
    gstats/Debug/usstats
    gt2/Debug/usgt2
    natneg/Debug/usnatneg
    Peer/Debug/uspeer
    pinger/Debug/uspinger
    pt/Debug/uspt
    qr2/Debug/usqr2
    sake/Debug/ussake
    sc/Debug/ussc
    serverbrowsing/Debug/usserverbrowsing
    sharedDll/Debug/UniSpySDK
    Voice2/Debug/usvoice2
    Voice2/libgsm/Debug/gsm
    Voice2/libspeex/Debug/speex
    webservices/Debug/uswebservice
  >

  $<$<CONFIG:Internal>:
    d3d12
    openal32
    dxguid
    dinput8
    ddraw
    d3dx9
    d3d9
    dxerr
    winmm
    vfw32
    dsound
    wsock32
    imm32
    wininet
    # Similarly for Internal config:
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
  >

  $<$<CONFIG:Release>:
    d3d12
    openal32
    dxguid
    dinput8
    ddraw
    d3dx9
    d3d9
    dxerr
    winmm
    vfw32
    dsound
    wsock32
    imm32
    wininet
    # Similarly for Release config:
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
  >
)

# If we want to ignore default libraries for certain configs:
target_link_options(WorldBuilder PRIVATE
  $<$<CONFIG:Debug>:/NODEFAULTLIB:libcd /DYNAMICBASE:NO /FORCE>
  $<$<CONFIG:Internal>:/NODEFAULTLIB:libc /DYNAMICBASE:NO /FORCE>
  $<$<CONFIG:Release>:/NODEFAULTLIB:libc /DYNAMICBASE:NO /FORCE>
)

# b) Additional library directories:
target_link_directories(WorldBuilder PRIVATE
  $<$<CONFIG:Debug>:
    ../../Libraries/Source/GameSpy/bin/x64
    ../../Libraries/OpenAL/bin/x64/RelWithDebInfo
    ../../Libraries/Source/WWVegas/Miles6/Libraries/Win64/DebugRelease
    ../../Libraries/DX90SDK/Lib/x64
    ../../Libraries/Lib
    ../../GameEngine/Lib
    ../../GameEngineDevice/Lib
    ../../GameEngine
  >
  $<$<CONFIG:Internal>:
    ../../Libraries/Source/GameSpy/bin/x64
    ../../Libraries/OpenAL/bin/x64/RelWithDebInfo
    ../../Libraries/Source/WWVegas/Miles6/Libraries/Win64/DebugRelease
    ../../Libraries/DX90SDK/Lib/x64
    ../../Libraries/Lib
    ../../GameEngine/Lib
    ../../GameEngineDevice/Lib
    ../../GameEngine
  >
  $<$<CONFIG:Release>:
    ../../Libraries/Source/GameSpy/bin/x64
    ../../Libraries/OpenAL/bin/x64/RelWithDebInfo
    ../../Libraries/Source/WWVegas/Miles6/Libraries/Win64/DebugRelease
    ../../Libraries/DX90SDK/Lib/x64
    ../../Libraries/Lib
    ../../GameEngine/Lib
    ../../GameEngineDevice/Lib
    ../../GameEngine
  >
)

set_target_properties(WorldBuilder PROPERTIES
    # Put all .exe files in a folder called "Run"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../run"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../../../run"
    RUNTIME_OUTPUT_DIRECTORY_INTERNAL "${CMAKE_CURRENT_SOURCE_DIR}/../../../run"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../../../run"

    # Each config has its own output name:
    OUTPUT_NAME_DEBUG "WorldBuilder_debug"      # Debug
    OUTPUT_NAME_INTERNAL "WorldBuilder_internal"   # Internal
    OUTPUT_NAME_RELEASE "WorldBuilder"     # Release
)


#------------------------------------------------------------------------------
# 8) Project references:
#   The vcxproj references other .vcxproj (GameEngine, GameEngineDevice, etc.).
#   In CMake, you typically do:
#       add_subdirectory(...) # to define those library targets
#       target_link_libraries(WorldBuilder PRIVATE GameEngine GameEngineDevice etc.)
#   If no direct CMakeLists exist for them, you can link them as external  files.
#------------------------------------------------------------------------------
# Example (assuming you have separate CMakeLists in those paths):
# add_subdirectory("../../GameEngine" GameEngine)
# add_subdirectory("../../GameEngineDevice" GameEngineDevice)
# target_link_libraries(WorldBuilder PRIVATE GameEngine GameEngineDevice GameRenderer Compression wwsaveload ...)

message(STATUS "WorldBuilder MFC CMake conversion loaded. Configs = ${CMAKE_CONFIGURATION_TYPES}")
