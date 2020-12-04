add_definitions(-DPLATFORM_x86_bitvisor)

set(PLATFORM_OBJECTS
  os.cpp
  serial1.cpp
  start.asm
  platform.cpp
  kernel_start.cpp
  sanity_checks.cpp
  bitvisor_manager.cpp
  ../x86_pc/init_libc.cpp
  ../x86_pc/rand.cpp
  )

add_library(x86_64_bitvisor STATIC ${PLATFORM_OBJECTS})
set_target_properties(x86_64_bitvisor PROPERTIES LINKER_LANGUAGE CXX)

set_target_properties(x86_64_bitvisor
  PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/platform
  ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/platform
  ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/platform
)

#
# Installation
#
set(CMAKE_INSTALL_MESSAGE LAZY) # to avoid spam

install(TARGETS x86_64_bitvisor DESTINATION platform)
