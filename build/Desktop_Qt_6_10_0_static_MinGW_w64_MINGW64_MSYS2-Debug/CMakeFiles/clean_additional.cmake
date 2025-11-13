# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "BarcodeScanner_autogen"
  "CMakeFiles\\BarcodeScanner_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\BarcodeScanner_autogen.dir\\ParseCache.txt"
  )
endif()
