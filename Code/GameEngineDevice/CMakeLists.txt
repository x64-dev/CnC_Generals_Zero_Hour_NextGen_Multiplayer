cmake_minimum_required(VERSION 3.20)

#------------------------------------------------------------------------------
# The project name "GameEngineDevice" is chosen to match typical usage.
#------------------------------------------------------------------------------
project(GameEngineDevice LANGUAGES CXX)

# If using a generator that supports multiple configs (like Visual Studio),
# you can set them like so:
set(CMAKE_CONFIGURATION_TYPES Debug Internal Release CACHE STRING "Configs" FORCE)

#------------------------------------------------------------------------------
# Create a STATIC library target (matches <ConfigurationType>StaticLibrary).
#------------------------------------------------------------------------------

add_library(GameEngineDevice STATIC
    # --------------------------------------------------------------------------
    # List of all .cpp files from the <ItemGroup><ClCompile> (none are excluded).
    # --------------------------------------------------------------------------
    Source/MilesAudioDevice/MilesAudioCache.cpp
    Source/MilesAudioDevice/MilesAudioManager.cpp
    Source/OpenALAudioDevice/OpenALAudioLoader.cpp
    Source/OpenALAudioDevice/OpenALAudioManager.cpp
    Source/OpenALAudioDevice/OpenALDriver.cpp
    Source/VideoDevice/Bink/BinkVideoPlayer.cpp
    Source/W3DDevice/Common/System/W3DFunctionLexicon.cpp
    Source/W3DDevice/Common/System/W3DRadar.cpp
    Source/W3DDevice/Common/Thing/W3DModuleFactory.cpp
    Source/W3DDevice/Common/Thing/W3DThingFactory.cpp
    Source/W3DDevice/Common/W3DConvert.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DDebrisDraw.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DDefaultDraw.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DDependencyModelDraw.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DLaserDraw.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DModelDraw.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DOverlordTankDraw.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DPoliceCarDraw.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DProjectileStreamDraw.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DRopeDraw.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DScienceModelDraw.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DSupplyDraw.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DTankDraw.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DTankTruckDraw.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DTracerDraw.cpp
    Source/W3DDevice/GameClient/Drawable/Draw/W3DTruckDraw.cpp
    Source/W3DDevice/GameClient/GUI/Gadget/W3DCheckBox.cpp
    Source/W3DDevice/GameClient/GUI/Gadget/W3DComboBox.cpp
    Source/W3DDevice/GameClient/GUI/Gadget/W3DHorizontalSlider.cpp
    Source/W3DDevice/GameClient/GUI/Gadget/W3DListBox.cpp
    Source/W3DDevice/GameClient/GUI/Gadget/W3DProgressBar.cpp
    Source/W3DDevice/GameClient/GUI/Gadget/W3DPushButton.cpp
    Source/W3DDevice/GameClient/GUI/Gadget/W3DRadioButton.cpp
    Source/W3DDevice/GameClient/GUI/Gadget/W3DStaticText.cpp
    Source/W3DDevice/GameClient/GUI/Gadget/W3DTabControl.cpp
    Source/W3DDevice/GameClient/GUI/Gadget/W3DTextEntry.cpp
    Source/W3DDevice/GameClient/GUI/Gadget/W3DVerticalSlider.cpp
    Source/W3DDevice/GameClient/GUI/GUICallbacks/W3DControlBar.cpp
    Source/W3DDevice/GameClient/GUI/GUICallbacks/W3DMainMenu.cpp
    Source/W3DDevice/GameClient/GUI/GUICallbacks/W3DMOTD.cpp
    Source/W3DDevice/GameClient/GUI/W3DGameFont.cpp
    Source/W3DDevice/GameClient/GUI/W3DGameWindow.cpp
    Source/W3DDevice/GameClient/GUI/W3DGameWindowManager.cpp
    Source/W3DDevice/GameClient/HeightMap.cpp
    Source/W3DDevice/GameClient/Shadow/W3DBufferManager.cpp
    Source/W3DDevice/GameClient/Shadow/W3DProjectedShadow.cpp
    Source/W3DDevice/GameClient/Shadow/W3DShadow.cpp
    Source/W3DDevice/GameClient/Shadow/W3DVolumetricShadow.cpp
    Source/W3DDevice/GameClient/TerrainTex.cpp
    Source/W3DDevice/GameClient/TileData.cpp
    Source/W3DDevice/GameClient/W3DAssetManager.cpp
    Source/W3DDevice/GameClient/W3DAssetManagerExposed.cpp
    Source/W3DDevice/GameClient/W3DBibBuffer.cpp
    Source/W3DDevice/GameClient/W3DBridgeBuffer.cpp
    Source/W3DDevice/GameClient/W3DCustomEdging.cpp
    Source/W3DDevice/GameClient/W3DDebugDisplay.cpp
    Source/W3DDevice/GameClient/W3DDebugIcons.cpp
    Source/W3DDevice/GameClient/W3DDisplay.cpp
    Source/W3DDevice/GameClient/W3DDisplayString.cpp
    Source/W3DDevice/GameClient/W3DDisplayStringManager.cpp
    Source/W3DDevice/GameClient/W3DDynamicLight.cpp
    Source/W3DDevice/GameClient/W3DFileSystem.cpp
    Source/W3DDevice/GameClient/W3DGameClient.cpp
    Source/W3DDevice/GameClient/W3DGranny.cpp
    Source/W3DDevice/GameClient/W3DInGameUI.cpp
    Source/W3DDevice/GameClient/W3DMouse.cpp
    Source/W3DDevice/GameClient/W3DParticleSys.cpp
    Source/W3DDevice/GameClient/W3DPoly.cpp
    Source/W3DDevice/GameClient/W3DRoadBuffer.cpp
    Source/W3DDevice/GameClient/W3DScene.cpp
    Source/W3DDevice/GameClient/W3DShaderManager.cpp
    Source/W3DDevice/GameClient/W3DShroud.cpp
    Source/W3DDevice/GameClient/W3DStatusCircle.cpp
    Source/W3DDevice/GameClient/W3DTerrainTracks.cpp
    Source/W3DDevice/GameClient/W3DTerrainVisual.cpp
    Source/W3DDevice/GameClient/W3DTreeBuffer.cpp
    Source/W3DDevice/GameClient/W3DVideoBuffer.cpp
    Source/W3DDevice/GameClient/W3DView.cpp
    Source/W3DDevice/GameClient/W3dWaypointBuffer.cpp
    Source/W3DDevice/GameClient/W3DWebBrowser.cpp
    Source/W3DDevice/GameClient/Water/W3DWater.cpp
    Source/W3DDevice/GameClient/Water/W3DWaterTracks.cpp
    Source/W3DDevice/GameClient/WorldHeightMap.cpp
    Source/W3DDevice/GameLogic/W3DGameLogic.cpp
    Source/W3DDevice/GameLogic/W3DGhostObject.cpp
    Source/W3DDevice/GameLogic/W3DTerrainLogic.cpp
    Source/Win32Device/Common/Win32BIGFile.cpp
    Source/Win32Device/Common/Win32BIGFileSystem.cpp
    Source/Win32Device/Common/Win32CDManager.cpp
    Source/Win32Device/Common/Win32GameEngine.cpp
    Source/Win32Device/Common/Win32LocalFile.cpp
    Source/Win32Device/Common/Win32LocalFileSystem.cpp
    Source/Win32Device/Common/Win32OSDisplay.cpp
    Source/Win32Device/GameClient/Win32DIKeyboard.cpp
    Source/Win32Device/GameClient/Win32Mouse.cpp
)

#------------------------------------------------------------------------------
# Include paths (identical for Debug, Internal, Release):
#------------------------------------------------------------------------------
target_include_directories(GameEngineDevice PRIVATE
  $<$<CONFIG:Debug>:
    ../GameRenderer
    ../Libraries/OpenAL/include
    ../Libraries/Bink/include
    ../Libraries/DX90SDK/Include
    ../Libraries/Source/WWVegas/Miles6/include
    ../Main
    Include
    ../GameEngine/Include
    ../Libraries/Include
    ../Libraries/Source/WWVegas
    ../Libraries/Source/WWVegas/WW3D2
    ../Libraries/Source/WWVegas/WWLib
    ../Libraries/Source/WWVegas/WWDebug
    ../Libraries/Source/WWVegas/WWMath
    ../Libraries/Source/WWVegas/WWSaveLoad
    ../Libraries/Include/Granny
  >
  $<$<CONFIG:Internal>:
    ../GameRenderer
    ../Libraries/OpenAL/include
    ../Libraries/Bink/include
    ../Libraries/DX90SDK/Include
    ../Libraries/Source/WWVegas/Miles6/include
    ../Main
    Include
    ../GameEngine/Include
    ../Libraries/Include
    ../Libraries/Source/WWVegas
    ../Libraries/Source/WWVegas/WW3D2
    ../Libraries/Source/WWVegas/WWLib
    ../Libraries/Source/WWVegas/WWDebug
    ../Libraries/Source/WWVegas/WWMath
    ../Libraries/Source/WWVegas/WWSaveLoad
    ../Libraries/Include/Granny
  >
  $<$<CONFIG:Release>:
    ../GameRenderer
    ../Libraries/OpenAL/include
    ../Libraries/Bink/include
    ../Libraries/DX90SDK/Include
    ../Libraries/Source/WWVegas/Miles6/include
    ../Main
    Include
    ../GameEngine/Include
    ../Libraries/Include
    ../Libraries/Source/WWVegas
    ../Libraries/Source/WWVegas/WW3D2
    ../Libraries/Source/WWVegas/WWLib
    ../Libraries/Source/WWVegas/WWDebug
    ../Libraries/Source/WWVegas/WWMath
    ../Libraries/Source/WWVegas/WWSaveLoad
    ../Libraries/Include/Granny
  >
)

#------------------------------------------------------------------------------
# Preprocessor definitions by configuration:
#------------------------------------------------------------------------------

target_compile_definitions(GameEngineDevice PRIVATE
  $<$<CONFIG:Debug>: 
    __PLACEMENT_VEC_NEW_INLINE
    _DEBUG
    _LIB
    _WINDOWS
  >

  $<$<CONFIG:Internal>:
    __PLACEMENT_VEC_NEW_INLINE
    NDEBUG
    _LIB
    _WINDOWS
    _INTERNAL
    _DISABLE_STRING_ANNOTATION
    _DISABLE_VECTOR_ANNOTATION
  >

  $<$<CONFIG:Release>:
    __PLACEMENT_VEC_NEW_INLINE
    _LIB
    _WINDOWS
    IG_DEBUG_STACKTRACE
    WIN32
    NDEBUG
    _RELEASE
  >
)

#------------------------------------------------------------------------------
# Compiler and runtime library flags (typical MSVC settings):
#------------------------------------------------------------------------------
target_compile_options(GameEngineDevice PRIVATE
  $<$<C_COMPILER_ID:MSVC>:
    # Debug => /Od /MDd /Zi (No optimization, debug runtime, PDB)
    $<$<CONFIG:Debug>:/Od /MDd /Zi /RTC1>

    # Internal => /O2 /Ob1 /MD /Zi + /GF + /Gy
    $<$<CONFIG:Internal>:/O2 /Ob1 /MD /Zi /GF /Gy>

    # Release => /O2 /Ob2 /MD + /GF + /Gy (the .vcxproj had ProgramDatabase for Release, 
    # you could optionally add /Zi if you want .pdb in Release as well)
    $<$<CONFIG:Release>:/O2 /Ob2 /MD /GF /Gy>
  >
)

#------------------------------------------------------------------------------
# Output library naming: sets the .lib names exactly as in the vcxproj:
# - Debug => "Lib/GameEngineDeviceDebug_x64.lib"
# - Internal => "Lib/GameEngineDeviceInternal_x64.lib"
# - Release => "Lib/GameEngineDevice.lib"
#------------------------------------------------------------------------------
set_target_properties(GameEngineDevice PROPERTIES
  OUTPUT_NAME_DEBUG      "GameEngineDeviceDebug_x64"
  OUTPUT_NAME_Internal   "GameEngineDeviceInternal_x64"
  OUTPUT_NAME_Release    "GameEngineDevice"
)

#------------------------------------------------------------------------------
# Precompiled headers (OPTIONAL). The .vcxproj here does not specify a .h to create/use,
# but if you ever want to do it with CMake:
#
# target_precompile_headers(GameEngineDevice PRIVATE
#     $<$<COMPILE_LANGUAGE:CXX>:SomePchHeader.h>
# )
#
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Project references:
# If you have EABrowserDispatch as a separate CMake target, you'd do:
#
#   add_subdirectory(../Libraries/Source/EABrowserDispatch EABrowserDispatch)
#   target_link_libraries(GameEngineDevice PRIVATE EABrowserDispatch)
#
# Or, if it is another top-level project you open separately, you can just omit 
# or do an external library approach. 
#------------------------------------------------------------------------------
message(STATUS "GameEngineDevice CMake conversion loaded. Configs = ${CMAKE_CONFIGURATION_TYPES}")
