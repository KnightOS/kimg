cmake_minimum_required(VERSION 2.8.5)
# Project Options
project(kimg C)

# Cmake Options
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin")

# Compiler Options
set(CMAKE_C_FLAGS "-std=c99 -Wall")

# Dependencies
find_package(ImageMagick REQUIRED COMPONENTS MagickWand)
if(ImageMagick_FOUND)
  include_directories(SYSTEM ${ImageMagick_INCLUDE_DIRS})
  link_directories(${ImageMagick_LIBRARY_DIRS})
endif()

# Executable

add_definitions(
  -DMAGICKCORE_HDRI_ENABLE=1
  -DMAGICKCORE_QUANTUM_DEPTH=16
)
add_executable(kimg
  src/main.c
)
target_link_libraries(kimg
  ${ImageMagick_LIBRARIES}
)

if(WIN32 OR CYGWIN)
  install(TARGETS kimg
      DESTINATION bin)
else()
  install(TARGETS kimg
      DESTINATION bin)
endif()

# Docs
ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_BINARY_DIR}/kimg.1
  COMMAND a2x --no-xmllint --doctype manpage --format manpage ${CMAKE_CURRENT_SOURCE_DIR}/kimg.1.txt -D ${CMAKE_BINARY_DIR}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/kimg.1.txt
)

ADD_CUSTOM_TARGET(man ALL
  DEPENDS ${CMAKE_BINARY_DIR}/kimg.1)

INSTALL(
  FILES ${CMAKE_BINARY_DIR}/kimg.1
  DESTINATION man/man1
)
