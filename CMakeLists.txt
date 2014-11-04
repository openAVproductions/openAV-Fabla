
cmake_minimum_required (VERSION 2.6)

project (fabla)

set(FABLA_VERSION_MAJOR "1")
set(FABLA_VERSION_MINOR "3")
set(FABLA_VERSION_PATCH "1")

set(FABLA_VERSION "${FABLA_VERSION_MAJOR}.${FABLA_VERSION_MINOR}.${FABLA_VERSION_PATCH}")

option(RELEASE_BUILD  "Build for production usage" ON )

find_package(PkgConfig)

#SET(CMAKE_INSTALL_PREFIX "/usr" )

pkg_check_modules(LV2 lv2 REQUIRED)
include_directories( ${LV2_INCLUDE_DIRS}  )

pkg_check_modules(NTK ntk REQUIRED)
include_directories( ${NTK_INCLUDE_DIRS}  )
link_directories   ( ${NTK_LIBRARY_DIRS}  )

pkg_check_modules(SNDFILE sndfile REQUIRED)
include_directories( ${SNDFILE_INCLUDE_DIRS}  )
link_directories   ( ${SNDFILE_LIBRARY_DIRS}  )

SET(CMAKE_SHARED_LINKER_FLAGS "-fPIC -shared -Wl,-z,nodelete -Wl,--no-undefined")

IF(RELEASE_BUILD)
  SET(CMAKE_CXX_FLAGS "-g -Wall -Wno-unused-variable -msse2 -mfpmath=sse -ffast-math")
  SET(CMAKE_C_FLAGS "-g -Wall -W -Wno-unused-variable -msse2 -mfpmath=sse -ffast-math")
ENDIF()

# print config
string( ASCII 27 _esc)

function(COLOR_MESSAGE TEXT)
    if(CMAKE_COLOR_MAKEFILE)
        MESSAGE(${TEXT})
    else()
        string(REGEX REPLACE "${_esc}.[0123456789;]*m" "" __TEXT ${TEXT} )
        MESSAGE(${__TEXT})
    endif()
endfunction()

SET(green     "${_esc}[1;32m")
SET(blue      "${_esc}[1;34m")
SET(reset     "${_esc}[1;0m")

COLOR_MESSAGE( "
  ${blue}Fabla Configuration : ${FABLA_VERSION}${reset}
-----------------------
* ${green}Install Directory${reset}               : ${CMAKE_INSTALL_PREFIX}\n" )
# /print config



FILE(GLOB sources dsp/fabla.cxx +
  gui/*.cxx +
  gui/*.c )

ADD_LIBRARY(fabla SHARED ${sources})

target_link_libraries( fabla ${NTK_LIBRARIES}     )
target_link_libraries( fabla ${SNDFILE_LIBRARIES} )

# Remove "lib" part before name (fabla.so, not libfabla.so)
set_target_properties(fabla PROPERTIES PREFIX "")

# add the install targets
install (TARGETS fabla DESTINATION lib/lv2/fabla.lv2/)

# install .ttl files
install(FILES "dsp/fabla.ttl"     DESTINATION lib/lv2/fabla.lv2/)
install(FILES "dsp/manifest.ttl"  DESTINATION lib/lv2/fabla.lv2/)
