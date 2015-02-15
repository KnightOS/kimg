cmake_minimum_required(VERSION 2.8.5)
project(mkpkg C)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin/")
add_definitions("-Wall")
find_package(ImageMagick REQUIRED)

add_executable(kimg
    src/main.c
)

if(WIN32 OR CYGWIN)
  install(PROGRAMS bin/kimg.exe
      DESTINATION bin)
else()
  install(PROGRAMS bin/kimg
      DESTINATION bin)
endif()

set(CMAKE_BUILD_TYPE Release)

ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/kimg.1
  COMMAND a2x --no-xmllint --doctype manpage --format manpage ${CMAKE_CURRENT_SOURCE_DIR}/kimg.1.txt 
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/kimg.1.txt
)

ADD_CUSTOM_TARGET(man ALL
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/kimg.1)

INSTALL(
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/kimg.1
  DESTINATION ${CMAKE_INSTALL_PREFIX}/man/man1
)
