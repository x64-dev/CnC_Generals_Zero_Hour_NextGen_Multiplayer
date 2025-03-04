#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "speex" for configuration "Debug"
set_property(TARGET speex APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(speex PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/speex.lib"
  )

list(APPEND _cmake_import_check_targets speex )
list(APPEND _cmake_import_check_files_for_speex "${_IMPORT_PREFIX}/lib/speex.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
