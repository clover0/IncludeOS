set(PLATFORM_OBJECTS
  binding.cpp
  )

add_library(x86_64_bv_binding STATIC ${PLATFORM_OBJECTS})
set_target_properties(x86_64_bv_binding PROPERTIES LINKER_LANGUAGE CXX)

set_target_properties(x86_64_bv_binding
  PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bitvisor
  ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bitvisor
  ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bitvisor
)

#
# Installation
#
set(CMAKE_INSTALL_MESSAGE LAZY) # to avoid spam

install(TARGETS x86_64_bv_binding DESTINATION bitvisor)
