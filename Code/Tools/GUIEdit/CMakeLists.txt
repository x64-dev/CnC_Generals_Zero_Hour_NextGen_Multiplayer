#------------------------------------------------------------------------------
# 1) Create an executable target for "GUIEdit"
#   We'll list the .cpp sources from <ItemGroup><ClCompile>, plus the .rc file for resources.
#------------------------------------------------------------------------------
add_executable(GUIEdit WIN32
    # Resource
    Resource/GUIEdit.rc

    # Main .cpp files from "Source\Dialog Procedures" etc.
    "Source/Dialog Procedures/CallbackEditor.cpp"
    "Source/Dialog Procedures/CheckBoxProperties.cpp"
    "Source/Dialog Procedures/ColorDialog.cpp"
    "Source/Dialog Procedures/ComboBoxProperties.cpp"
    "Source/Dialog Procedures/GenericProperties.cpp"
    "Source/Dialog Procedures/GridSettings.cpp"
    "Source/Dialog Procedures/ListboxProperties.cpp"
    "Source/Dialog Procedures/NewLayoutDialog.cpp"
    "Source/Dialog Procedures/ProgressBarProperties.cpp"
    "Source/Dialog Procedures/PushButtonProperties.cpp"
    "Source/Dialog Procedures/RadioButtonProperties.cpp"
    "Source/Dialog Procedures/SliderProperties.cpp"
    "Source/Dialog Procedures/StaticTextProperties.cpp"
    "Source/Dialog Procedures/TabControlProperties.cpp"
    "Source/Dialog Procedures/TextEntryProperties.cpp"
    Source/EditWindow.cpp
    Source/GUIEdit.cpp
    Source/GUIEditDisplay.cpp
    Source/GUIEditWindowManager.cpp
    Source/HierarchyView.cpp
    Source/LayoutScheme.cpp
    Source/Properties.cpp
    Source/Save.cpp
    Source/WinMain.cpp
)

#------------------------------------------------------------------------------
# 2) Include directories that differ by config. The .vcxproj sets:
#    ".\Include;.\Resource;..\..\Libraries\Include;..\..\GameEngine\Include;..\..\GameEngineDevice\Include;..\..\Libraries\Source\WWVegas;..\..\Libraries\Source\WWVegas\WWLib;..\..\Libraries\Source\WWVegas\WWMath;..\..\Libraries\Source\WWVegas\WWDebug;..\..\Libraries\Source\WWVegas\WWSaveLoad;..\..\Libraries\Source\WWVegas\WW3D2"
#------------------------------------------------------------------------------
target_include_directories(GUIEdit PRIVATE
  $<$<CONFIG:Debug>:
    ./Include
    ./Resource
    ../../Libraries/Include
    ../../GameEngine/Include
    ../../GameEngineDevice/Include
    ../../Libraries/Source/WWVegas
    ../../Libraries/Source/WWVegas/WWLib
    ../../Libraries/Source/WWVegas/WWMath
    ../../Libraries/Source/WWVegas/WWDebug
    ../../Libraries/Source/WWVegas/WWSaveLoad
    ../../Libraries/Source/WWVegas/WW3D2
  >
  $<$<CONFIG:Internal>:
    ./Include
    ./Resource
    ../../Libraries/Include
    ../../GameEngine/Include
    ../../GameEngineDevice/Include
    ../../Libraries/Source/WWVegas
    ../../Libraries/Source/WWVegas/WWLib
    ../../Libraries/Source/WWVegas/WWMath
    ../../Libraries/Source/WWVegas/WWDebug
    ../../Libraries/Source/WWVegas/WWSaveLoad
    ../../Libraries/Source/WWVegas/WW3D2
  >
  $<$<CONFIG:Release>:
    ./Include
    ./Resource
    ../../Libraries/Include
    ../../GameEngine/Include
    ../../GameEngineDevice/Include
    ../../Libraries/Source/WWVegas
    ../../Libraries/Source/WWVegas/WWLib
    ../../Libraries/Source/WWVegas/WWMath
    ../../Libraries/Source/WWVegas/WWDebug
    ../../Libraries/Source/WWVegas/WWSaveLoad
    ../../Libraries/Source/WWVegas/WW3D2
  >
)

set_target_properties(GUIEdit PROPERTIES
    # Put all .exe files in a folder called "Run"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../run"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../../../run"
    RUNTIME_OUTPUT_DIRECTORY_INTERNAL "${CMAKE_CURRENT_SOURCE_DIR}/../../../run"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../../../run"

    # Each config has its own output name:
    OUTPUT_NAME_DEBUG "GUIEdit_debug"
    OUTPUT_NAME_INTERNAL "GUIEdit_internal"
    OUTPUT_NAME_RELEASE "GUIEdit"
)


#------------------------------------------------------------------------------
# 3) Preprocessor definitions
#   Debug => WIN32, _DEBUG, _WINDOWS
#   Internal => WIN32, NDEBUG, _WINDOWS, _INTERNAL
#   Release => _WINDOWS, WIN32, NDEBUG, _RELEASE
#------------------------------------------------------------------------------
target_compile_definitions(GUIEdit PRIVATE
  $<$<CONFIG:Debug>:WIN32 _DEBUG _WINDOWS>
  $<$<CONFIG:Internal>:WIN32 NDEBUG _WINDOWS _INTERNAL>
  $<$<CONFIG:Release>:_WINDOWS WIN32 NDEBUG _RELEASE>
)

#------------------------------------------------------------------------------
# 5) Resource compile definitions 
#   - Debug => _DEBUG
#   - Internal => NDEBUG
#   - Release => NDEBUG
#------------------------------------------------------------------------------
if(MSVC)
  target_compile_definitions(GUIEdit PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Internal>:NDEBUG>
    $<$<CONFIG:Release>:NDEBUG>
  )
endif()

#------------------------------------------------------------------------------
# 6) Link libraries differ by config
#   The .vcxproj sets:
#   Debug => AdditionalDependencies: BenchmarkD.lib;WW3D2Debug.lib;WWDebugDebug.lib;... + standard libs
#            <IgnoreSpecificDefaultLibraries>libcd
#            SubSystem=Windows => /SUBSYSTEM:WINDOWS
#   Release => AdditionalDependencies: Benchmark.lib;WW3D2.lib;WWDebug.lib;WWLib.lib... 
#             <IgnoreSpecificDefaultLibraries>libc.lib
#   Internal => AdditionalDependencies: Benchmark.lib;WW3D2Internal.lib;WWDebugInternal.lib;...
#             <IgnoreSpecificDefaultLibraries>libc.lib
#
#   Also AdditionalLibraryDirectories => ../..GameEngine\Lib, etc. 
#------------------------------------------------------------------------------
target_link_directories(GUIEdit PRIVATE
  $<$<CONFIG:Debug>:
    ../../GameEngine/Lib
    ../../GameEngineDevice/Lib
    ../../Libraries/Lib
    ../../GameEngine
  >
  $<$<CONFIG:Internal>:
    ../../GameEngine/Lib
    ../../GameEngineDevice/Lib
    ../../Libraries/Lib
    ../../GameEngine
  >
  $<$<CONFIG:Release>:
    ../../GameEngine/Lib
    ../../GameEngineDevice/Lib
    ../../Libraries/Lib
    ../../GameEngine
  >
)

# For the GameSpy Debug libs, the .vcxproj lists them in the final <ItemGroup>, 
# apparently for debugging only. We'll assume they belong to Debug config.
set(GAMESPY_DEBUG_LIBS
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Libraries/Source/GameSpy/bin/brigades/Debug/usbrigades.lib
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Libraries/Source/GameSpy/bin/Chat/Debug/uschat.lib
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Libraries/Source/GameSpy/bin/common/Debug/uscommon.lib
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Libraries/Source/GameSpy/bin/Direct2Game/Debug/usd2g.lib
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Libraries/Source/GameSpy/bin/gcdkey/Debug/uscdkey.lib
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Libraries/Source/GameSpy/bin/ghttp/Debug/ushttp.lib
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Libraries/Source/GameSpy/bin/GP/Debug/usgp.lib
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Libraries/Source/GameSpy/bin/gstats/Debug/usstats.lib
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Libraries/Source/GameSpy/bin/gt2/Debug/usgt2.lib
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Libraries/Source/GameSpy/bin/natneg/Debug/usnatneg.lib
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Libraries/Source/GameSpy/bin/Peer/Debug/uspeer.lib
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Libraries/Source/GameSpy/bin/pinger/Debug/uspinger.lib
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Libraries/Source/GameSpy/bin/pt/Debug/uspt.lib
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Libraries/Source/GameSpy/bin/qr2/Debug/usqr2.lib
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Libraries/Source/GameSpy/bin/sake/Debug/ussake.lib
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Libraries/Source/GameSpy/bin/sc/Debug/ussc.lib
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Libraries/Source/GameSpy/bin/serverbrowsing/Debug/usserverbrowsing.lib
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Libraries/Source/GameSpy/bin/sharedDll/Debug/UniSpySDK.lib
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Libraries/Source/GameSpy/bin/Voice2/Debug/usvoice2.lib
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Libraries/Source/GameSpy/bin/Voice2/libgsm/Debug/gsm.lib
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Libraries/Source/GameSpy/bin/Voice2/libspeex/Debug/speex.lib
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Libraries/Source/GameSpy/bin/webservices/Debug/uswebservice.lib
)

target_link_libraries(GUIEdit PRIVATE
    GameEngine
    GameEngineDevice
    wsock32.lib
    dxguid.lib
    dinput8.lib
    d3dx9.lib
    d3d9.lib
    odbc32.lib
    odbccp32.lib
    vfw32.lib
    winmm.lib
    dsound.lib
    comctl32.lib
    imm32.lib
    ${GAMESPY_DEBUG_LIBS}
)

target_link_directories(GUIEdit PRIVATE
    ../../Libraries/DX90SDK/Lib/x64
)


#------------------------------------------------------------------------------
# 7) Linker settings
#   The .vcxproj sets <IgnoreSpecificDefaultLibraries>libcd in Debug, 
#   and <IgnoreSpecificDe
