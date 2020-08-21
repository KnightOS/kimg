cmake_minimum_required(VERSION 2.8.5)

project(kimg C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic")

if (NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(ImageMagick REQUIRED COMPONENTS MagickWand)

string(SUBSTRING "${ImageMagick_VERSION_STRING}" 0 1 ImageMagickMajor)

add_definitions(
  -DMAGICKCORE_HDRI_ENABLE=1
  -DMAGICKCORE_QUANTUM_DEPTH=16
  -DMAGICK_VERSION=${ImageMagickMajor}
)
add_executable(kimg kimg.c)
target_include_directories(kimg PRIVATE ${ImageMagick_INCLUDE_DIRS})
target_link_libraries(kimg ${ImageMagick_LIBRARIES})

# Docs
ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_BINARY_DIR}/kimg.1
  COMMAND a2x --no-xmllint --doctype manpage --format manpage ${CMAKE_CURRENT_SOURCE_DIR}/kimg.1.txt -D ${CMAKE_BINARY_DIR}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/kimg.1.txt
)

ADD_CUSTOM_TARGET(man ALL DEPENDS ${CMAKE_BINARY_DIR}/kimg.1)

INSTALL(
  FILES ${CMAKE_BINARY_DIR}/kimg.1
  DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man/man1
)
install(TARGETS kimg DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

