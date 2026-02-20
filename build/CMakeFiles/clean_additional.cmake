# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "Acadence_autogen"
  "CMakeFiles/Acadence_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/Acadence_autogen.dir/ParseCache.txt"
  )
endif()
